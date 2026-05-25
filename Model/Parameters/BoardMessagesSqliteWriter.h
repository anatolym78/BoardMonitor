#ifndef BOARDMESSAGESSQLITEWRITER_H
#define BOARDMESSAGESSQLITEWRITER_H

#include <QObject>
#include <QMutex>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include "./Tree/ParameterTreeStorage.h"

class BoardMessagesSqliteWriter : public QObject
{
	Q_OBJECT

public:
	explicit BoardMessagesSqliteWriter(const QString &databasePath = "BoardStationData.db", 
									  QObject *parent = nullptr);
	~BoardMessagesSqliteWriter();
	
	// Управление сессиями
	void createNewSession(const QString &sessionName = QString());
	void switchToSession(int sessionId);
	void switchToSession(const QString &sessionName);
	QStringList getAvailableSessions() const;
	int getCurrentSessionId() const { return m_currentSessionId; }
	QString getCurrentSessionName() const;
	
	// Получение пути к файлу базы данных
	QString getDatabasePath() const { return m_databasePath; }
	
	// Очистка данных текущей сессии
	void clearCurrentSession();
	
	// Получение статистики
	int getMessageCount() const;
	int getParameterCount() const;

	// Сохранение древовидной модели параметров в текущую сессию
	// Проходит по всем узлам-историям и записывает их значения
	bool writeTree(ParameterTreeStorage* storage);

signals:
	void writeError(const QString &error);
	void writeSuccess(const QString &message);
	void sessionCreated(int sessionId, const QString &sessionName);
	void sessionSwitched(int sessionId, const QString &sessionName);
	void sessionCleared(int sessionId);

private:
	// Инициализация базы данных
	bool initializeDatabase();
	
	// Создание таблиц
	bool createTables();
	
	// Создание новой сессии
	int createSessionRecord(const QString &sessionName);
	
	// Получение ID параметра по метке (создание при необходимости)
	int getParameterId(const QString &label, const QString &unit);

private:
	QString m_databasePath;
	QSqlDatabase m_database;
	int m_currentSessionId;
	mutable QMutex m_databaseMutex;
};

#endif // BOARDMESSAGESSQLITEWRITER_H
