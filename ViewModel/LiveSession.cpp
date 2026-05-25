#include "LiveSession.h"
#include "./../Model/Parameters/Tree/ParameterTreeStorage.h"
#include <QDebug>
#include <QDateTime>

#include "DriverDataPlayer.h"

LiveSession::LiveSession(QObject *parent)
	: Session(parent)
	, m_startTime(QDateTime::currentDateTime())
	, m_messageCount(0)
	, m_parameterCount(0)
	, m_isRecording(false)
{
	m_player = new DriverDataPlayer(this);
	m_player->setStorage(m_treeStorage);

	m_chartsModel->setPlayer(m_player);

	connect(m_player, &DataPlayer::played, this, &LiveSession::onPlayed);

	// Увеличиваем счетчик параметров при добавлении нового значения в хранилище
	connect(m_treeStorage, &ParameterTreeStorage::valueAdded, this, [this](ParameterTreeHistoryItem*)
	{
		incrementParameterCount();
	});
}

void LiveSession::onPlayed(ParameterTreeStorage* subStorage, bool isBackPlaying)
{
	m_parametersModel->setSnapshot(subStorage, isBackPlaying);

	if (m_startTime.isNull())
	{
		m_startTime = QDateTime::currentDateTime();
	}
}

void LiveSession::open()
{
	Session::open();
}

void LiveSession::startSession()
{
	if (m_startTime.isNull())
	{
		m_startTime = QDateTime::currentDateTime();
		emit sessionStarted();
		qDebug() << "LiveSession: Session started at" << m_startTime.toString();
	}
}

void LiveSession::stopSession()
{
	if (!m_startTime.isNull())
	{
		qDebug() << "LiveSession: Session stopped at" << QDateTime::currentDateTime().toString();
		emit sessionStopped();
	}
}

void LiveSession::setRecording(bool recording)
{
	if (m_isRecording != recording)
	{
		m_isRecording = recording;
		emit recordingStateChanged(recording);
		emit sessionChanged();
		qDebug() << "LiveSession: Recording state changed to" << recording;
	}
}

void LiveSession::incrementMessageCount()
{
	m_messageCount++;
	emit messageCountChanged(m_messageCount);
	emit sessionChanged();
}

void LiveSession::incrementParameterCount()
{
	m_parameterCount++;
	emit parameterCountChanged(m_parameterCount);
	emit sessionChanged();
}

void LiveSession::resetCounters()
{
	m_messageCount = 0;
	m_parameterCount = 0;
	emit messageCountChanged(m_messageCount);
	emit parameterCountChanged(m_parameterCount);
	emit sessionChanged();
	qDebug() << "LiveSession: Counters reset";
}

void LiveSession::clearStorage()
{
	if (m_treeStorage)
	{
		m_treeStorage->clear();
	}
	resetCounters();
}
