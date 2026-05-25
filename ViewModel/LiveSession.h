#ifndef LIVESESSION_H
#define LIVESESSION_H

#include "Session.h"
#include "./../Model/Parameters/Tree/ParameterTreeHistoryItem.h"

/**
 * @brief Класс для представления живой сессии (поток данных от драйвера)
 */
class LiveSession : public Session
{
	Q_OBJECT

public:
	explicit LiveSession(QObject *parent = nullptr);
	~LiveSession() = default;

	// Реализация виртуальных методов
	int getId() const override { return -1; } // У живой сессии нет ID в БД
	QString getName() const override { return tr("Live Data"); }
	QDateTime getCreatedAt() const override { return m_startTime; }
	QString getDescription() const override { return "Поток данных в реальном времени"; }
	int getMessageCount() const override { return m_messageCount; }
	int getParameterCount() const override { return m_parameterCount; }
	SessionType getType() const override { return Session::LiveSession; }
	bool isRecording() const override { return m_isRecording; }

	// Реализация методов для работы с хранилищем
	void clearStorage() override;

	// Методы для управления живой сессией
	void startSession();
	void stopSession();
	void setRecording(bool recording);
	void incrementMessageCount();
	void incrementParameterCount();
	void resetCounters();

	// Получение времени начала сессии
	QDateTime getStartTime() const { return m_startTime; }

	void onPlayed(ParameterTreeStorage* subStorage, bool isBackPlaying);

	void open() override;

signals:
	void sessionStarted();
	void sessionStopped();
	void recordingStateChanged(bool recording);

private:
	QDateTime m_startTime;
	int m_messageCount;
	int m_parameterCount;
	bool m_isRecording;

};

#endif // LIVESESSION_H
