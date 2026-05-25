#ifndef LIVESESSIONFACTORY_H
#define LIVESESSIONFACTORY_H

#include <QObject>
#include "Session.h"
#include "LiveSession.h"

/**
 * @brief Фабрика для создания живой сессии
 */
class LiveSessionFactory : public QObject
{
    Q_OBJECT

public:
    explicit LiveSessionFactory(QObject *parent = nullptr);
    ~LiveSessionFactory() = default;

    // Методы для работы с фабрикой
    Session* createLiveSession();
    LiveSession* getLiveSession() const { return m_liveSession; }
    
    // Методы для управления живой сессией
    void startLiveSession();
    void stopLiveSession();
    void setRecording(bool recording);
    void incrementMessageCount();
    void incrementParameterCount();
    void resetCounters();

signals:
    void liveSessionCreated(Session* session);
    void liveSessionStarted();
    void liveSessionStopped();
    void recordingStateChanged(bool recording);

private:
    LiveSession* m_liveSession;
};

#endif // LIVESESSIONFACTORY_H
