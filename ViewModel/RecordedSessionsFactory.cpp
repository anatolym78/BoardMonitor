#include "RecordedSessionsFactory.h"
#include <QDebug>

RecordedSessionsFactory::RecordedSessionsFactory(QObject *parent)
    : QObject(parent)
    , m_reader(nullptr)
{
}

void RecordedSessionsFactory::setReader(BoardMessagesSqliteReader *reader)
{
    m_reader = reader;
}

QList<Session*> RecordedSessionsFactory::createSessions()
{
    QList<Session*> sessions;
    
    if (!m_reader)
    {
        qWarning() << "RecordedSessionsFactory: Reader is not set";
        emit errorOccurred("Reader не установлен");
        return sessions;
    }
    
    try
    {
        QList<BoardMessagesSqliteReader::SessionInfo> sessionInfos = m_reader->getAvailableSessions();
        
        for (const auto& sessionInfo : sessionInfos)
        {
            Session* session = createSession(sessionInfo);
            if (session)
            {
                sessions.append(session);
            }
        }
        
        qDebug() << "RecordedSessionsFactory: Created" << sessions.size() << "recorded sessions";
        emit sessionsCreated(sessions);
    }
    catch (const std::exception &e)
    {
        qWarning() << "RecordedSessionsFactory: Error creating sessions:" << e.what();
        emit errorOccurred(QString("Ошибка создания сессий: %1").arg(e.what()));
    }
    
    return sessions;
}

Session* RecordedSessionsFactory::createSession(const BoardMessagesSqliteReader::SessionInfo& sessionInfo)
{
    RecordedSession* session = new RecordedSession(sessionInfo, this);
    qDebug() << "RecordedSessionsFactory: Created session" << sessionInfo.id << "with name" << sessionInfo.name;
    return session;
}

void RecordedSessionsFactory::updateSession(Session* session, const BoardMessagesSqliteReader::SessionInfo& sessionInfo)
{
    if (!session)
    {
        qWarning() << "RecordedSessionsFactory: Cannot update null session";
        return;
    }
    
    RecordedSession* recordedSession = qobject_cast<RecordedSession*>(session);
    if (recordedSession)
    {
        recordedSession->updateSessionInfo(sessionInfo);
        qDebug() << "RecordedSessionsFactory: Updated session" << sessionInfo.id;
    }
    else
    {
        qWarning() << "RecordedSessionsFactory: Session is not a RecordedSession";
    }
}
