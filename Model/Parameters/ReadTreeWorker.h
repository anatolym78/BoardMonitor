#ifndef READTREEWORKER_H
#define READTREEWORKER_H

#include <QObject>
#include <QVector>
#include <QVariant>
#include <QDateTime>

struct ReadRowData
{
	QString label;
	QVariant value;
	QDateTime timestamp;
};

/**
 * @brief Читает данные сессии из SQLite в отдельном потоке.
 *
 * Открывает собственное соединение с БД, сначала получает COUNT для вычисления
 * прогресса, затем читает строки и накапливает в m_result.
 * Доступ к result() безопасен из главного потока после получения finished().
 */
class ReadTreeWorker : public QObject
{
	Q_OBJECT
public:
	explicit ReadTreeWorker(const QString& dbPath, int sessionId, QObject* parent = nullptr);

	const QVector<ReadRowData>& result() const { return m_result; }

public slots:
	void run();

signals:
	void progress(int pct);      // 0-100
	void finished(bool success);

private:
	QString m_dbPath;
	int m_sessionId;
	QString m_connectionName;
	QVector<ReadRowData> m_result;
};

#endif // READTREEWORKER_H
