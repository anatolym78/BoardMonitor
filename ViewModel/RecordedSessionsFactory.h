#ifndef RECORDEDSESSIONSFACTORY_H
#define RECORDEDSESSIONSFACTORY_H

#include <QObject>
#include <QList>
#include "Session.h"
#include "RecordedSession.h"
#include "./../Model/Parameters/BoardMessagesSqliteReader.h"

/**
 * @brief Фабрика для создания записанных сессий из базы данных
 */
class RecordedSessionsFactory : public QObject
{
    Q_OBJECT

public:
    explicit RecordedSessionsFactory(QObject *parent = nullptr);
    ~RecordedSessionsFactory() = default;

    // Методы для работы с фабрикой
    void setReader(BoardMessagesSqliteReader *reader);
    QList<Session*> createSessions();
    Session* createSession(const BoardMessagesSqliteReader::SessionInfo& sessionInfo);
    
    // Методы для обновления существующих сессий
    void updateSession(Session* session, const BoardMessagesSqliteReader::SessionInfo& sessionInfo);

signals:
    void sessionsCreated(const QList<Session*>& sessions);
    void errorOccurred(const QString &error);

private:
    BoardMessagesSqliteReader *m_reader;
};

#endif // RECORDEDSESSIONSFACTORY_H
