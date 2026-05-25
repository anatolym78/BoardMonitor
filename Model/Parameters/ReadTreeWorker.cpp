#include "ReadTreeWorker.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>
#include <QDebug>

ReadTreeWorker::ReadTreeWorker(const QString& dbPath, int sessionId, QObject* parent)
	: QObject(parent)
	, m_dbPath(dbPath)
	, m_sessionId(sessionId)
	, m_connectionName("ReadTreeWorker_" + QUuid::createUuid().toString(QUuid::WithoutBraces))
{}

void ReadTreeWorker::run()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
	db.setDatabaseName(m_dbPath);
	db.setConnectOptions("QSQLITE_OPEN_READONLY");

	if (!db.open())
	{
		qWarning() << "ReadTreeWorker: не удалось открыть БД:" << db.lastError().text();
		QSqlDatabase::removeDatabase(m_connectionName);
		emit finished(false);
		return;
	}

	// Узнаём общее количество строк для вычисления прогресса
	int total = 0;
	{
		QSqlQuery countQuery(db);
		countQuery.prepare("SELECT COUNT(*) FROM parameter_values WHERE session_id = ?");
		countQuery.addBindValue(m_sessionId);
		if (countQuery.exec() && countQuery.next())
			total = countQuery.value(0).toInt();
	}

	if (total == 0)
	{
		db.close();
		QSqlDatabase::removeDatabase(m_connectionName);
		emit progress(100);
		emit finished(true);
		return;
	}

	m_result.reserve(total);

	QSqlQuery query(db);
	query.prepare(R"(
		SELECT p.label, pv.value, pv.timestamp
		FROM parameter_values pv
		JOIN parameters p ON pv.parameter_id = p.id
		WHERE pv.session_id = ?
		ORDER BY pv.timestamp ASC, p.label ASC
	)");
	query.addBindValue(m_sessionId);

	if (!query.exec())
	{
		qWarning() << "ReadTreeWorker: ошибка SELECT:" << query.lastError().text();
		db.close();
		QSqlDatabase::removeDatabase(m_connectionName);
		emit finished(false);
		return;
	}

	int done = 0;
	int lastPct = -1;

	while (query.next())
	{
		ReadRowData row;
		row.label = query.value(0).toString();

		const QString valueStr = query.value(1).toString();
		bool ok = false;
		const double dbl = valueStr.toDouble(&ok);
		if (ok)
		{
			row.value = dbl;
		}
		else
		{
			const QString lower = valueStr.toLower();
			if (lower == "true" || lower == "false")
				row.value = (lower == "true");
			else
				row.value = valueStr;
		}

		row.timestamp = query.value(2).toDateTime();
		m_result.append(row);

		++done;
		int pct = done * 100 / total;
		if (pct != lastPct)
		{
			emit progress(pct);
			lastPct = pct;
		}
	}

	db.close();
	QSqlDatabase::removeDatabase(m_connectionName);
	emit finished(true);
}
