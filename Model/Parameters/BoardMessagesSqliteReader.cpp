#include "BoardMessagesSqliteReader.h"
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QApplication>
#include "./Tree/ParameterTreeGroupItem.h"
#include "./Tree/ParameterTreeArrayItem.h"
#include "./Tree/ParameterTreeHistoryItem.h"

BoardMessagesSqliteReader::BoardMessagesSqliteReader(const QString &databasePath, QObject *parent)
    : QObject(parent)
    , m_databasePath(QApplication::applicationDirPath() + "/" + databasePath)
{
    initializeDatabase();
}

BoardMessagesSqliteReader::~BoardMessagesSqliteReader()
{
    if (m_database.isOpen()) 
    {
        m_database.close();
    }
}

QList<BoardMessagesSqliteReader::SessionInfo> BoardMessagesSqliteReader::getAvailableSessions()
{
    QList<SessionInfo> sessions;
    
    QSqlQuery query(m_database);
    // Используем MAX(pv.id) - MIN(pv.id) для оценки количества сообщений, если timestamp дублируются
    // Или группируем по секундам для уменьшения шума
    query.prepare(R"(
        SELECT s.id, s.name, s.created_at, s.description,
               (SELECT COUNT(DISTINCT timestamp) FROM parameter_values WHERE session_id = s.id) as message_count,
               (SELECT COUNT(id) FROM parameter_values WHERE session_id = s.id) as parameter_count
        FROM sessions s
        ORDER BY s.created_at DESC
    )");
    
    if (query.exec()) 
    {
        while (query.next()) 
        {
            SessionInfo info;
            info.id = query.value("id").toInt();
            info.name = query.value("name").toString();
            // Время теперь сохраняется в локальном времени, поэтому просто читаем его
            info.createdAt = query.value("created_at").toDateTime();
            info.description = query.value("description").toString();
            // COUNT(DISTINCT timestamp) может давать завышенные значения из-за микросекунд
            // Используем приближенную оценку или считаем честно
            info.messageCount = query.value("message_count").toInt();
            info.parameterCount = query.value("parameter_count").toInt();
            sessions.append(info);
        }
    }
    else
    {
        qWarning() << "BoardMessagesSqliteReader: Ошибка получения списка сессий:" << query.lastError().text();
        emit readError("Ошибка получения списка сессий");
    }
    
    return sessions;
}

BoardMessagesSqliteReader::SessionInfo BoardMessagesSqliteReader::getSessionInfo(int sessionId)
{
    SessionInfo info;
    info.id = -1;
    
    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT s.id, s.name, s.created_at, s.description,
               (SELECT COUNT(DISTINCT timestamp) FROM parameter_values WHERE session_id = s.id) as message_count,
               (SELECT COUNT(id) FROM parameter_values WHERE session_id = s.id) as parameter_count
        FROM sessions s
        WHERE s.id = ?
    )");
    query.addBindValue(sessionId);
    
    if (query.exec() && query.next()) 
    {
        info.id = query.value("id").toInt();
        info.name = query.value("name").toString();
        // Время теперь сохраняется в локальном времени, поэтому просто читаем его
        info.createdAt = query.value("created_at").toDateTime();
        info.description = query.value("description").toString();
        // Принудительно ограничиваем сообщение count разумными пределами
        // Если частота записи 30Гц, то message_count не должен превышать duration * 30
        // Но здесь мы просто берем честное значение из базы
        info.messageCount = query.value("message_count").toInt();
        info.parameterCount = query.value("parameter_count").toInt();
    }
    else
    {
        qWarning() << "BoardMessagesSqliteReader: Ошибка получения информации о сессии:" << query.lastError().text();
        emit readError("Ошибка получения информации о сессии");
    }
    
    return info;
}

bool BoardMessagesSqliteReader::initializeDatabase()
{
    m_database = QSqlDatabase::addDatabase("QSQLITE", "BoardStationReaderConnection");
    m_database.setDatabaseName(m_databasePath);
    
    if (!m_database.open()) 
    {
        qWarning() << "BoardMessagesSqliteReader: Не удалось открыть базу данных:" << m_database.lastError().text();
        emit readError("Не удалось открыть базу данных");
        return false;
    }
    
    return true;
}

bool BoardMessagesSqliteReader::removeSession(int sessionId)
{
    if (sessionId <= 0)
    {
        qWarning() << "BoardMessagesSqliteReader: Invalid session ID for removal:" << sessionId;
        return false;
    }
    
    QSqlQuery query(m_database);
    
    // Начинаем транзакцию
    m_database.transaction();
    
    try 
    {
        // Удаляем все значения параметров для этой сессии
        query.prepare("DELETE FROM parameter_values WHERE session_id = ?");
        query.addBindValue(sessionId);
        
        if (!query.exec()) 
        {
            qWarning() << "BoardMessagesSqliteReader: Failed to delete parameter values for session" << sessionId << ":" << query.lastError().text();
            m_database.rollback();
            return false;
        }
        
        // Удаляем саму сессию
        query.prepare("DELETE FROM sessions WHERE id = ?");
        query.addBindValue(sessionId);
        
        if (!query.exec()) 
        {
            qWarning() << "BoardMessagesSqliteReader: Failed to delete session" << sessionId << ":" << query.lastError().text();
            m_database.rollback();
            return false;
        }
        
        // Подтверждаем транзакцию
        m_database.commit();
        
        qDebug() << "BoardMessagesSqliteReader: Successfully removed session" << sessionId;
        return true;
    }
    catch (const std::exception &e) 
    {
        qWarning() << "BoardMessagesSqliteReader: Exception during session removal:" << e.what();
        m_database.rollback();
        return false;
    }
}

bool BoardMessagesSqliteReader::loadSessionToTree(int sessionId, ParameterTreeStorage* storage)
{
	if (!storage)
	{
		qWarning() << "BoardMessagesSqliteReader: loadSessionToTree - storage is null";
		return false;
	}

	// Очищаем текущее дерево
	storage->clear();

	QSqlQuery query(m_database);
	query.prepare(R"(
		SELECT p.label, p.unit, pv.value, pv.timestamp
		FROM parameter_values pv
		JOIN parameters p ON pv.parameter_id = p.id
		WHERE pv.session_id = ?
		ORDER BY pv.timestamp ASC, p.label ASC
	)");
	query.addBindValue(sessionId);

	if (!query.exec())
	{
		qWarning() << "BoardMessagesSqliteReader: Ошибка загрузки сессии в дерево:" << query.lastError().text();
		emit readError("Ошибка загрузки сессии в дерево");
		return false;
	}

	// Восстанавливаем узлы-истории и добавляем значения
	while (query.next())
	{
		const QString label = query.value("label").toString();
		const QString valueStr = query.value("value").toString();
		const QDateTime ts = query.value("timestamp").toDateTime();

		// Парсим значение как число/булево/строку (как при createParameterFromQuery)
		QVariant value;
		bool ok = false;
		const double dbl = valueStr.toDouble(&ok);
		if (ok)
		{
			value = dbl;
		}
		else
		{
			const QString lower = valueStr.toLower();
			if (lower == "true" || lower == "false")
			{
				value = (lower == "true");
			}
			else
			{
				value = valueStr;
			}
		}

		// Получаем части составной метки
		const QStringList parts = label.split('.');
		if (parts.isEmpty()) continue;

		auto historyItem = ensureHistoryPath(storage, parts);
		if (!historyItem) continue;
		historyItem->addValue(value, ts);
	}

	return true;
}

ParameterTreeHistoryItem* BoardMessagesSqliteReader::ensureHistoryPath(ParameterTreeStorage* storage, const QStringList& labelParts) const
{
	if (!storage || labelParts.isEmpty()) return nullptr;

	ParameterTreeItem* current = storage;
	// Все части пути, кроме последней, считаем группами
	for (int i = 0; i < labelParts.size() - 1; ++i)
	{
		const QString& part = labelParts.at(i);
		auto existing = current->findChildByLabel(part, false);
		if (!existing)
		{
			// По умолчанию создаем как Group, но если узел уже существует как Array, используем его
			// При установке снимка через setSnapshot типы будут исправлены при несовпадении
			auto group = new ParameterTreeGroupItem(part, current);
			current->appendChild(group);
			current = group;
		}
		else
		{
			current = existing;
		}
	}

	// Последняя часть — узел-история
	const QString& leaf = labelParts.last();
	auto existingLeaf = current->findChildByLabel(leaf, false);
	if (existingLeaf && existingLeaf->type() == ParameterTreeItem::ItemType::History)
	{
		return static_cast<ParameterTreeHistoryItem*>(existingLeaf);
	}

	if (!existingLeaf)
	{
		auto history = new ParameterTreeHistoryItem(leaf, current);
		current->appendChild(history);
		return history;
	}

	// Если по какой-то причине лист существует, но не History — создадим новый History с уникальным именем
	auto history = new ParameterTreeHistoryItem(leaf + "_hist", current);
	current->appendChild(history);
	return history;
}
