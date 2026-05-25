#ifndef DATAPLAYER_H
#define DATAPLAYER_H

#include <QObject>
#include <QDateTime>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QMutex>
#include "./../Model/Parameters/Tree/ParameterTreeStorage.h"

class DataPlayer : public QObject
{
	Q_OBJECT
	
	Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
	Q_PROPERTY(QDateTime currentPosition READ currentPosition NOTIFY currentPositionChanged)
	Q_PROPERTY(QString currentSessionName READ currentSessionName NOTIFY currentSessionNameChanged)
	Q_PROPERTY(QDateTime sessionStartTime READ sessionStartTime NOTIFY sessionStartTimeChanged)
	Q_PROPERTY(QDateTime sessionEndTime READ sessionEndTime NOTIFY sessionEndTimeChanged)
	Q_PROPERTY(double sessionDuration READ sessionDuration NOTIFY sessionDurationChanged)
	Q_PROPERTY(double elapsedTime READ elapsedTime NOTIFY elapsedTimeChanged)

public:
	explicit DataPlayer(QObject *parent = nullptr);
	virtual ~DataPlayer();

	// Свойства
	bool isPlaying() const { return m_isPlaying; }
	QDateTime currentPosition() const;
	QString currentSessionName() const { return m_currentSessionName; }
	QDateTime sessionStartTime() const { return m_sessionStartTime; }
	QDateTime sessionEndTime() const { return m_sessionEndTime; }
	double sessionDuration() const;
	double elapsedTime() const;

	// Методы управления
	Q_INVOKABLE virtual void play();
	Q_INVOKABLE virtual void stop();
	Q_INVOKABLE virtual void pause();
	Q_INVOKABLE virtual void setPosition(QDateTime position);
	Q_INVOKABLE virtual void setElapsedTime(double position);
	Q_INVOKABLE virtual void moveToBegin();
	Q_INVOKABLE virtual void onMoved(double value);
	Q_INVOKABLE virtual void reset() {}

	Q_INVOKABLE bool isPlayable() { return m_isPlayable; }
	
	// Методы для работы с хранилищем
	virtual void setStorage(ParameterTreeStorage* storage);

	virtual void resetState() = 0;
	virtual void initialPlay() = 0;

	ParameterTreeStorage* storage() const { return m_storage; }

signals:
	void isPlayingChanged();
	void currentPositionChanged();
	void currentSessionNameChanged();
	void sessionStartTimeChanged();
	void sessionEndTimeChanged();
	void sessionDurationChanged();
	void elapsedTimeChanged();
	void playbackFinished();
	void played(ParameterTreeStorage* subStorage, bool isBackPlaying);
	void stopped();

protected:
	virtual void startPlayback();
	virtual void stopPlayback();
	virtual void updatePlaybackPosition();
	virtual void clearStorage();
	virtual void playbackLoop();
	void playParametersInTimeRange(const QDateTime& startTime, const QDateTime& endTime);

protected:
	ParameterTreeStorage* m_storage;
	QFuture<void> m_playbackFuture;
	QAtomicInt m_shouldStop;
	
	bool m_isPlaying;
	
	QString m_currentSessionName;
	QDateTime m_currentPosition; // Текущая позиция как время
	QDateTime m_sessionStartTime;
	QDateTime m_sessionEndTime;

	mutable QMutex m_positionMutex;

	bool m_isPlayable = true;
};

#endif // DATAPLAYER_H
