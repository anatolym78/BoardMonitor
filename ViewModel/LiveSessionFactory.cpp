#include "LiveSessionFactory.h"
#include <QDebug>

LiveSessionFactory::LiveSessionFactory(QObject *parent)
    : QObject(parent)
    , m_liveSession(nullptr)
{
}

Session* LiveSessionFactory::createLiveSession()
{
    if (!m_liveSession)
    {
        m_liveSession = new LiveSession(this);
        
        // Подключаем сигналы живой сессии к фабрике
        connect(m_liveSession, &LiveSession::sessionStarted,
                this, &LiveSessionFactory::liveSessionStarted);
        connect(m_liveSession, &LiveSession::sessionStopped,
                this, &LiveSessionFactory::liveSessionStopped);
        connect(m_liveSession, &LiveSession::recordingStateChanged,
                this, &LiveSessionFactory::recordingStateChanged);
        
        qDebug() << "LiveSessionFactory: Created live session";
        emit liveSessionCreated(m_liveSession);
    }
    
    return m_liveSession;
}

void LiveSessionFactory::startLiveSession()
{
    if (m_liveSession)
    {
        m_liveSession->startSession();
    }
    else
    {
        qWarning() << "LiveSessionFactory: Live session not created yet";
    }
}

void LiveSessionFactory::stopLiveSession()
{
    if (m_liveSession)
    {
        m_liveSession->stopSession();
    }
    else
    {
        qWarning() << "LiveSessionFactory: Live session not created yet";
    }
}

void LiveSessionFactory::setRecording(bool recording)
{
    if (m_liveSession)
    {
        m_liveSession->setRecording(recording);
    }
    else
    {
        qWarning() << "LiveSessionFactory: Live session not created yet";
    }
}

void LiveSessionFactory::incrementMessageCount()
{
    if (m_liveSession)
    {
        m_liveSession->incrementMessageCount();
    }
    else
    {
        qWarning() << "LiveSessionFactory: Live session not created yet";
    }
}

void LiveSessionFactory::incrementParameterCount()
{
    if (m_liveSession)
    {
        m_liveSession->incrementParameterCount();
    }
    else
    {
        qWarning() << "LiveSessionFactory: Live session not created yet";
    }
}

void LiveSessionFactory::resetCounters()
{
    if (m_liveSession)
    {
        m_liveSession->resetCounters();
    }
    else
    {
        qWarning() << "LiveSessionFactory: Live session not created yet";
    }
}
