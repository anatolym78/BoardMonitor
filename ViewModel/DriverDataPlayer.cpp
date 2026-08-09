#include "DriverDataPlayer.h"
#include <QDebug>

#include "./../Model/Parameters/Tree/ParameterTreeHistoryItem.h"

DriverDataPlayer::DriverDataPlayer(QObject *parent)
	: DataPlayer(parent)
{
	m_isPlayable = false;
	m_currentSessionName = tr("Live Data");
	emit currentSessionNameChanged();

	m_refreshTimer = new QTimer(this);
	m_refreshTimer->setSingleShot(true);
	m_refreshTimer->setInterval(REFRESH_INTERVAL_MS);
	connect(m_refreshTimer, &QTimer::timeout, this, &DriverDataPlayer::flushRefresh);
}

DriverDataPlayer::~DriverDataPlayer()
{
}

void DriverDataPlayer::setStorage(ParameterTreeStorage* storage)
{
	if (m_storage)
	{
		return;
	}

	DataPlayer::setStorage(storage);

	connect(m_storage, &ParameterTreeStorage::valueAdded,
		this, &DriverDataPlayer::onStorageValueAdded);
}

double DriverDataPlayer::dataHeadElapsed() const
{
	if (m_sessionStartTime.isNull() || m_dataHeadTime.isNull())
	{
		return 0.0;
	}
	return m_sessionStartTime.msecsTo(m_dataHeadTime) / 1000.0;
}

void DriverDataPlayer::updateDataHead(const QDateTime& timestamp)
{
	if (!timestamp.isValid())
	{
		return;
	}
	if (m_dataHeadTime.isValid() && timestamp <= m_dataHeadTime)
	{
		return;
	}
	m_dataHeadTime = timestamp;
	emit dataHeadChanged();
}

void DriverDataPlayer::onStorageValueAdded(ParameterTreeHistoryItem* historyItem)
{
	if (!historyItem)
	{
		return;
	}

	const QDateTime timestamp = historyItem->lastTimestamp();
	if (!timestamp.isValid())
	{
		return;
	}

	if (!m_isInitialized)
	{
		m_sessionStartTime = timestamp;
		{
			QMutexLocker locker(&m_positionMutex);
			m_currentPosition = timestamp;
		}
		m_sessionEndTime = timestamp.addSecs(static_cast<int>(TIME_RANGE));
		m_dataHeadTime = timestamp;
		m_isInitialized = true;
		emitTimeRangeSignals();
		emit dataHeadChanged();
		play();
	}

	extendTimeRangeTo(timestamp);
	updateDataHead(timestamp);

	// На паузе кружок/курсор графиков замирают; playhead растёт через dataHeadChanged
	if (!m_isPlaying)
	{
		return;
	}

	{
		QMutexLocker locker(&m_positionMutex);
		m_currentPosition = timestamp;
	}
	emit currentPositionChanged();
	emit elapsedTimeChanged();
	scheduleRefresh();
}

void DriverDataPlayer::play()
{
	if (m_isPlaying)
	{
		return;
	}

	m_isPlaying = true;
	m_shouldStop = 0;
	emit isPlayingChanged();

	if (!m_isInitialized)
	{
		return;
	}

	// После паузы — к актуальной голове данных (без обхода storage под чужим lock)
	QDateTime head = m_dataHeadTime;
	if (!head.isValid() && m_storage)
	{
		head = m_storage->latestTimestamp();
	}
	if (head.isValid())
	{
		{
			QMutexLocker locker(&m_positionMutex);
			m_currentPosition = head;
		}
		emit currentPositionChanged();
		emit elapsedTimeChanged();
	}

	scheduleRefresh();
}

void DriverDataPlayer::stop()
{
	if (m_refreshTimer)
	{
		m_refreshTimer->stop();
	}

	m_lastConsumedTimestamp = QDateTime();
	DataPlayer::stop();
	m_isInitialized = false;
}

void DriverDataPlayer::pause()
{
	if (!m_isPlaying)
	{
		return;
	}

	if (m_refreshTimer)
	{
		m_refreshTimer->stop();
	}

	m_isPlaying = false;
	m_shouldStop = 1;
	emit isPlayingChanged();
}

void DriverDataPlayer::setPosition(QDateTime position)
{
	if (position < m_sessionStartTime)
	{
		position = m_sessionStartTime;
	}
	else if (position > m_sessionEndTime)
	{
		position = m_sessionEndTime;
	}

	{
		QMutexLocker locker(&m_positionMutex);
		m_currentPosition = position;
	}
	emit currentPositionChanged();
	emit elapsedTimeChanged();
}

void DriverDataPlayer::resetState()
{
	if (m_refreshTimer)
	{
		m_refreshTimer->stop();
	}

	m_isPlaying = false;
	m_shouldStop = 1;
	m_isInitialized = false;
	m_lastConsumedTimestamp = QDateTime();
	m_dataHeadTime = QDateTime();

	m_sessionStartTime = QDateTime();
	m_sessionEndTime = QDateTime();
	{
		QMutexLocker locker(&m_positionMutex);
		m_currentPosition = QDateTime();
	}

	emit isPlayingChanged();
	emitTimeRangeSignals();
	emit dataHeadChanged();

	qDebug() << "DriverDataPlayer: State reset - ready for new live session";
}

void DriverDataPlayer::moveToBegin()
{
}

void DriverDataPlayer::reset()
{
	m_isInitialized = false;
	m_lastConsumedTimestamp = QDateTime();
	m_dataHeadTime = QDateTime();
}

void DriverDataPlayer::startPlayback()
{
	// Live-режим: без фонового playbackLoop по wall-clock.
}

void DriverDataPlayer::setRefreshPaused(bool paused)
{
	if (m_refreshPaused == paused)
	{
		return;
	}

	m_refreshPaused = paused;

	if (m_refreshPaused)
	{
		if (m_refreshTimer)
		{
			m_refreshTimer->stop();
		}
	}
	else if (m_isPlaying)
	{
		scheduleRefresh();
	}
}

void DriverDataPlayer::scheduleRefresh()
{
	if (m_refreshPaused || !m_refreshTimer || m_refreshTimer->isActive())
	{
		return;
	}

	m_refreshTimer->start();
}

void DriverDataPlayer::flushRefresh()
{
	if (m_refreshPaused || !m_isPlaying || !m_storage)
	{
		return;
	}

	const QDateTime latest = m_storage->latestTimestamp();
	if (!latest.isValid())
	{
		return;
	}

	if (m_lastConsumedTimestamp.isValid() && latest <= m_lastConsumedTimestamp)
	{
		return;
	}

	m_lastConsumedTimestamp = latest;
	updateDataHead(latest);

	{
		QMutexLocker locker(&m_positionMutex);
		m_currentPosition = latest;
	}

	emit played(nullptr, false);
	emit currentPositionChanged();
	emit elapsedTimeChanged();
}

void DriverDataPlayer::extendTimeRangeTo(const QDateTime& latestTimestamp)
{
	if (!latestTimestamp.isValid())
	{
		return;
	}

	const QDateTime threshold = m_sessionEndTime.addSecs(-10);
	if (latestTimestamp < threshold)
	{
		return;
	}

	const QDateTime newEnd = latestTimestamp.addSecs(static_cast<int>(TIME_RANGE));
	if (newEnd <= m_sessionEndTime)
	{
		return;
	}

	m_sessionEndTime = newEnd;
	emit sessionEndTimeChanged();
	emit sessionDurationChanged();
}

void DriverDataPlayer::emitTimeRangeSignals()
{
	emit sessionStartTimeChanged();
	emit sessionEndTimeChanged();
	emit sessionDurationChanged();
	emit currentPositionChanged();
	emit elapsedTimeChanged();
}
