#include "SessionPlayer.h"
#include <QDebug>
#include <QDateTime>
#include "./../Model/Parameters/Tree/ParameterTreeStorage.h"
#include "./../Model/Parameters/Tree/ParameterTreeItem.h"
#include "./../Model/Parameters/Tree/ParameterTreeHistoryItem.h"

SessionPlayer::SessionPlayer(QObject *parent)
	: DataPlayer(parent)
	, m_reader(nullptr)
	, m_currentSessionId(-1)
	, m_lastPlayedIndex(-1)
	, m_lastPlayedPosition(QDateTime())
{
	connect(this, &DataPlayer::stopped, this, [this]()
		{
			//initialPlay();
		});
}

SessionPlayer::~SessionPlayer()
{
	//stop();
}

void SessionPlayer::setStorage(ParameterTreeStorage* storage)
{
	DataPlayer::setStorage(storage);
}

void SessionPlayer::setReader(BoardMessagesSqliteReader* reader)
{
	m_reader = reader;
}

void SessionPlayer::initialPlay()
{
	playParametersInTimeRange(m_sessionStartTime, m_sessionStartTime.addMSecs(1));
}

void SessionPlayer::moveToBegin()
{
	DataPlayer::moveToBegin();
}

void SessionPlayer::initializeWithLoadedData()
{
	if (!m_storage)
	{
		qWarning() << "SessionPlayer: Storage not available for initialization";
		return;
	}

	// Обходим дерево и находим минимальную и максимальную метки времени
	QDateTime minTs;
	QDateTime maxTs;

	std::function<void(ParameterTreeItem*)> traverse = [&](ParameterTreeItem* node)
	{
		if (!node) return;
		if (node->type() == ParameterTreeItem::ItemType::History)
		{
			auto hist = static_cast<ParameterTreeHistoryItem*>(node);
			const auto& timestamps = hist->timestamps();
			if (!timestamps.isEmpty())
			{
				const QDateTime first = timestamps.first();
				const QDateTime last = timestamps.last();
				if (minTs.isNull() || first < minTs) minTs = first;
				if (maxTs.isNull() || last > maxTs) maxTs = last;
			}
		}
		for (auto child : node->children())
		{
			traverse(child);
		}
	};

	traverse(m_storage);

	if (minTs.isNull() || maxTs.isNull())
	{
		qWarning() << "SessionPlayer: No timestamps found in storage";
		return;
	}

	m_sessionStartTime = minTs;
	m_sessionEndTime = maxTs;
	{
		QMutexLocker locker(&m_positionMutex);
		m_currentPosition = m_sessionStartTime;
	}
	m_lastPlayedIndex = -1;
	m_lastPlayedPosition = m_sessionStartTime;

	emit sessionStartTimeChanged();
	emit sessionEndTimeChanged();
	emit sessionDurationChanged();
	emit currentPositionChanged();
	emit elapsedTimeChanged();

	qDebug() << "SessionPlayer: Initialized time range from"
			 << m_sessionStartTime.toString() << "to" << m_sessionEndTime.toString();
}

void SessionPlayer::updatePlaybackPosition()
{
	// Сохраняем предыдущую позицию для определения временного интервала
	QDateTime previousPosition;
	QDateTime currentPos;

	{
		QMutexLocker locker(&m_positionMutex);
		previousPosition = m_currentPosition;
	
		// Для SessionPlayer увеличиваем позицию на 100мс (0.1 секунды)
		m_currentPosition = m_currentPosition.addMSecs(100);
	 
		// Проверяем, не достигли ли конца сессии
		if (m_currentPosition >= m_sessionEndTime)
		{
			m_currentPosition = m_sessionEndTime;
			// Можно добавить логику завершения воспроизведения
		}
		currentPos = m_currentPosition;
	}
	
	// Проигрываем параметры, которые попадают в текущий временной интервал
	if (m_storage && m_isPlaying)
	{
		playParametersInTimeRange(previousPosition, currentPos);
	}
	
	emit currentPositionChanged();
	emit elapsedTimeChanged();
}

