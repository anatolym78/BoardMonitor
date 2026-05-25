#ifndef BOARDMESSAGESSQLITEREADER_H
#define BOARDMESSAGESSQLITEREADER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include <QList>
#include "./../Parameters/Tree/ParameterTreeStorage.h"

class BoardMessagesSqliteReader : public QObject
{
    Q_OBJECT

public:
    explicit BoardMessagesSqliteReader(const QString &databasePath = "BoardStationData.db", 
                                      QObject *parent = nullptr);
    ~BoardMessagesSqliteReader();
    
    // Получение списка доступных сессий
    struct SessionInfo 
    {
        int id;
        QString name;
        QDateTime createdAt;
        QString description;
        int messageCount;
        int parameterCount;
    };
    
    QList<SessionInfo> getAvailableSessions();
    
    // Получение статистики сессии
    SessionInfo getSessionInfo(int sessionId);
    
    // Удаление сессии
    bool removeSession(int sessionId);

	// Загрузка данных сессии в древовидную модель параметров
	bool loadSessionToTree(int sessionId, ParameterTreeStorage* storage);

	QString getDatabasePath() const { return m_databasePath; }

signals:
    void readError(const QString &error);

private:
    // Инициализация базы данных
    bool initializeDatabase();
    
	// Вспомогательное: гарантировать существование узла-истории по пути метки и вернуть его
	ParameterTreeHistoryItem* ensureHistoryPath(ParameterTreeStorage* storage, const QStringList& labelParts) const;

private:
    QString m_databasePath;
    QSqlDatabase m_database;
};

#endif // BOARDMESSAGESSQLITEREADER_H
