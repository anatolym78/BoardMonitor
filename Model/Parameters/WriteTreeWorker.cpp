#include "WriteTreeWorker.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>
#include <QDebug>

WriteTreeWorker::WriteTreeWorker(const QString& dbPath,
                                  int sessionId,
                                  const QList<ParameterRowData>& rows,
                                  QObject* parent)
	: QObject(parent)
	, m_dbPath(dbPath)
	, m_sessionId(sessionId)
	, m_rows(rows)
	, m_connectionName("WriteTreeWorker_" + QUuid::createUuid().toString(QUuid::WithoutBraces))
{}

void WriteTreeWorker::run()
{
	int total = 0;
	for (const auto& row : m_rows)
		total += qMin(row.values.size(), row.timestamps.size());

	if (total == 0)
	{
		emit progress(100);
		emit finished(true);
		return;
	}

	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
	db.setDatabaseName(m_dbPath);

	if (!db.open())
	{
		qWarning() << "WriteTreeWorker: не удалось открыть БД:" << db.lastError().text();
		QSqlDatabase::removeDatabase(m_connectionName);
		emit finished(false);
		return;
	}

	if (!db.transaction())
	{
		qWarning() << "WriteTreeWorker: не удалось начать транзакцию";
		db.close();
		QSqlDatabase::removeDatabase(m_connectionName);
		emit finished(false);
		return;
	}

	int done = 0;
	int lastPct = -1;
	bool success = true;

	for (const auto& row : m_rows)
	{
		int paramId = getOrCreateParameterId(row.label, db);
		if (paramId <= 0)
		{
			success = false;
			break;
		}

		const int n = qMin(row.values.size(), row.timestamps.size());
		for (int i = 0; i < n; ++i)
		{
			QSqlQuery query(db);
			query.prepare("INSERT INTO parameter_values (session_id, parameter_id, value, timestamp) VALUES (?, ?, ?, ?)");
			query.addBindValue(m_sessionId);
			query.addBindValue(paramId);
			query.addBindValue(row.values.at(i).toString());
			query.addBindValue(row.timestamps.at(i));

			if (!query.exec())
			{
				qWarning() << "WriteTreeWorker: ошибка INSERT:" << query.lastError().text();
				success = false;
				break;
			}

			++done;

			// Испускаем прогресс только при изменении целого процента
			int pct = done * 100 / total;
			if (pct != lastPct)
			{
				emit progress(pct);
				lastPct = pct;
			}
		}
		if (!success) break;
	}

	if (success)
	{
		if (!db.commit())
		{
			qWarning() << "WriteTreeWorker: ошибка commit";
			success = false;
		}
	}
	else
	{
		db.rollback();
	}

	db.close();
	QSqlDatabase::removeDatabase(m_connectionName);

	emit finished(success);
}

int WriteTreeWorker::getOrCreateParameterId(const QString& label, QSqlDatabase& db)
{
	QSqlQuery query(db);
	query.prepare("SELECT id FROM parameters WHERE label = ?");
	query.addBindValue(label);
	if (query.exec() && query.next())
		return query.value(0).toInt();

	query.prepare("INSERT INTO parameters (label, unit) VALUES (?, ?)");
	query.addBindValue(label);
	query.addBindValue(QString());
	if (query.exec())
		return query.lastInsertId().toInt();

	qWarning() << "WriteTreeWorker: не удалось получить/создать параметр:" << label;
	return -1;
}
