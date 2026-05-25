#ifndef RECORDEDSESSION_H
#define RECORDEDSESSION_H

#include "Session.h"
#include "./../Model/Parameters/BoardMessagesSqliteReader.h"
#include "./../Model/Parameters/ReadTreeWorker.h"

#include <QThread>

class BoardParameterHistoryStorage;

/**
 * @brief Класс для представления записанной сессии из базы данных
 */
class RecordedSession : public Session
{
	Q_OBJECT

public:
	explicit RecordedSession(const BoardMessagesSqliteReader::SessionInfo& sessionInfo, 
							QObject *parent = nullptr);
	~RecordedSession() = default;

	// Реализация виртуальных методов
	int getId() const override { return m_sessionInfo.id; }
	QString getName() const override { return m_sessionInfo.name; }
	QDateTime getCreatedAt() const override { return m_sessionInfo.createdAt; }
	QString getDescription() const override { return m_sessionInfo.description; }
	int getMessageCount() const override { return m_sessionInfo.messageCount; }
	int getParameterCount() const override { return m_sessionInfo.parameterCount; }
	SessionType getType() const override { return Session::RecordedSession; }
	bool isRecording() const override { return false; }

	// Реализация методов для работы с хранилищем
	void clearStorage() override;

	// Синхронная загрузка (для совместимости)
	void loadDataFromDatabase(BoardMessagesSqliteReader* reader);

	// Асинхронная загрузка с прогрессом
	void loadDataFromDatabaseAsync(const QString& dbPath);

	// Прогресс загрузки: -1 = не загружается, 0-100 = процент
	int loadProgress() const { return m_loadProgress; }

	// Методы для обновления данных сессии
	void updateSessionInfo(const BoardMessagesSqliteReader::SessionInfo& sessionInfo);
	void updateMessageCount(int count);
	void updateParameterCount(int count);

	// Получение полной информации о сессии
	const BoardMessagesSqliteReader::SessionInfo& getSessionInfo() const { return m_sessionInfo; }
	
	// Проверка, загружены ли данные в хранилище
	bool isDataLoaded() const;

	void open() override;

signals:
	// Испускается (отложенно, через QTimer::singleShot) после завершения async-загрузки
	void dataLoadCompleted();

private:
	void populateTreeFromRows(const QVector<ReadRowData>& rows);

	BoardMessagesSqliteReader::SessionInfo m_sessionInfo;
	bool m_dataLoaded = false;
	int m_loadProgress = -1;
	QThread* m_loadThread = nullptr;
};

#endif // RECORDEDSESSION_H
