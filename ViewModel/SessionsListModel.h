#ifndef SESSIONSLISTMODEL_H
#define SESSIONSLISTMODEL_H

#include <QAbstractItemModel>
#include <QObject>
#include <QDateTime>
#include <QDate>
#include <QList>
#include <QSet>
#include <QTimer>
#include <QVector>
#include "Session.h"
#include "RecordedSessionsFactory.h"
#include "LiveSessionFactory.h"
#include "./../Model/Parameters/BoardMessagesSqliteReader.h"
#include "RecordedSession.h"
#include "SessionPlayer.h"
#include "DriverDataPlayer.h"
#include "LiveSession.h"

class SessionsListModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	enum SessionRoles
	{
		SessionIdRole = Qt::UserRole + 1,
		SessionNameRole,
		CreatedAtRole,
		CreatedAtFormattedRole,
		MessageCountRole,
		ParameterCountRole,
		DescriptionRole,
		RecordingRole,
		RecordedSessionRole,
		SessionTypeRole,
		IsLiveSessionRole,
		ParametersModelRole,
		PlayerModelRole,
		ChartsModelRole,
		TestColorRole,
		IconRole,
		SaveProgressRole,
		LoadProgressRole,
		IsDayFolderRole,
		FlatSessionIndexRole,
	};

	explicit SessionsListModel(QObject *parent = nullptr);
	~SessionsListModel();

	Q_INVOKABLE bool isRecordingState() const { return m_recording;}
	Q_INVOKABLE void startRecording ();
	Q_INVOKABLE void stopRecording();
	Q_INVOKABLE void setRecordingState(bool enable);

	int sessionCount() const { return m_sessions.size(); }

	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& index) const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QHash<int, QByteArray> roleNames() const override;

	QModelIndex indexForFlatSession(int flatIndex) const;

	Q_INVOKABLE QVariantMap get(int index) const;

	void setReader(BoardMessagesSqliteReader *reader);
	Q_INVOKABLE void refreshSessions();
	Q_INVOKABLE void addRecordedSession(const BoardMessagesSqliteReader::SessionInfo &sessionInfo);
	void setSaveProgress(int pct);
	Q_INVOKABLE void updateSessionMessageCount(int sessionId, int messageCount);
	Q_INVOKABLE void removeSession(int index);

	Session* getSession(int index) const;
	Session* getSessionById(int sessionId) const;
	LiveSession* liveSession() const;
	BoardMessagesSqliteReader::SessionInfo getSessionInfo(int index) const;
	int getSessionId(int index) const;
	int findSessionIndex(int sessionId) const;

	Q_INVOKABLE void selectSession(int sessionIndex);

	bool createLiveSession();
	void updateLiveSessionCounters();
	void resetLiveSessionCounters();

	Q_INVOKABLE void switchToSession(int sessionIndex);
	Q_INVOKABLE void switchToLiveSession();

	Q_INVOKABLE void resetLiveSession();

	Session* getCurrentActiveSession() const { return m_currentActiveSession; }

	Session* operator [] (int index) const
	{
		return session(index);
	}

	Session* session(int index) const
	{
		if (index >= 0 && index < m_sessions.size())
		{
			return m_sessions[index];
		}

		return nullptr;
	}

signals:
	void sessionsRefreshed();
	void errorOccurred(const QString &error);
	void sessionSwitched(Session* session);
	void sessionInsertedAt(int flatIndex, Session* session);
	void sessionRemovedAt(int flatIndex);
	void sessionAboutToBeRemoved(int flatIndex);

private slots:
	void onRecordedSessionsCreated(const QList<Session*>& sessions);
	void onLiveSessionCreated(Session* session);
	void onSessionChanged();
	void onMessageCountChanged(int count);
	void onParameterCountChanged(int count);
	void flushLiveSessionListUpdate();

private:
	struct SessionTreeItem;

	void sortSessionList();
	void rebuildTree();
	void clearTree();
	SessionTreeItem* findSessionNode(Session* session) const;
	SessionTreeItem* findDayFolder(const QDate& day) const;
	int dayFolderInsertRow(const QDate& day) const;
	bool insertSessionIntoTree(Session* session);
	void removeSessionFromTree(Session* session);
	void updateFlatIndices();
	void notifySubtreeVisualRefresh(const QModelIndex& parentIndex);
	SessionTreeItem* itemAt(const QModelIndex& index) const;
	QModelIndex indexForItem(const SessionTreeItem* item) const;
	void connectSessionSignals(Session* session);
	void addSessionToList(Session* session);
	void removeSessionFromList(Session* session);
	void updateSessionInList(Session* session, const QVector<int>& roles = QVector<int>());

private:
	BoardMessagesSqliteReader *m_reader;
	bool m_recording = false;
	QList<Session*> m_sessions;

	RecordedSessionsFactory *m_recordedSessionsFactory;
	LiveSessionFactory *m_liveSessionFactory;

	Session* m_liveSession;
	Session* m_currentActiveSession;

	int m_liveSessionSaveProgress = -1;

	SessionTreeItem* m_root = nullptr;
	QSet<SessionTreeItem*> m_liveTreeNodes;
	QTimer* m_liveListUpdateTimer = nullptr;
	bool m_liveListUpdatePending = false;
};

#endif // SESSIONSLISTMODEL_H
