#ifndef WRITETREEWORKER_H
#define WRITETREEWORKER_H

#include <QObject>
#include <QList>
#include <QVariant>
#include <QDateTime>
#include <QSqlDatabase>

struct ParameterRowData
{
	QString label;
	QList<QVariant> values;
	QList<QDateTime> timestamps;
};

/**
 * @brief Записывает данные дерева параметров в SQLite в отдельном потоке.
 *
 * Открывает собственное соединение с БД (не зависит от соединения главного потока),
 * выполняет INSERT-ы в транзакции, испускает прогресс каждые ~1%.
 */
class WriteTreeWorker : public QObject
{
	Q_OBJECT
public:
	explicit WriteTreeWorker(const QString& dbPath,
	                          int sessionId,
	                          const QList<ParameterRowData>& rows,
	                          QObject* parent = nullptr);

public slots:
	void run();

signals:
	void progress(int pct);       // 0-100
	void finished(bool success);

private:
	int getOrCreateParameterId(const QString& label, QSqlDatabase& db);

	QString m_dbPath;
	int m_sessionId;
	QList<ParameterRowData> m_rows;
	QString m_connectionName;
};

#endif // WRITETREEWORKER_H
