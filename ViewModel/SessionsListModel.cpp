#include "SessionsListModel.h"
#include <QDebug>
#include <QIcon>
#include <QMap>
#include <QTimer>
#include <algorithm>

struct SessionsListModel::SessionTreeItem
{
	enum class Kind { DayFolder, Session };

	Kind kind = Kind::Session;
	Session* session = nullptr;
	QDate folderDate;
	SessionTreeItem* parent = nullptr;
	QList<SessionTreeItem*> children;
	int flatIndex = -1;
};

SessionsListModel::SessionsListModel(QObject *parent)
	: QAbstractItemModel(parent)
	, m_reader(nullptr)
	, m_recordedSessionsFactory(new RecordedSessionsFactory(this))
	, m_liveSessionFactory(new LiveSessionFactory(this))
	, m_liveSession(nullptr)
	, m_currentActiveSession(nullptr)
	, m_root(new SessionTreeItem())
{
	connect(m_recordedSessionsFactory, &RecordedSessionsFactory::sessionsCreated,
			this, &SessionsListModel::onRecordedSessionsCreated);
	connect(m_liveSessionFactory, &LiveSessionFactory::liveSessionCreated,
			this, &SessionsListModel::onLiveSessionCreated);

	m_liveListUpdateTimer = new QTimer(this);
	m_liveListUpdateTimer->setSingleShot(true);
	m_liveListUpdateTimer->setInterval(80);
	connect(m_liveListUpdateTimer, &QTimer::timeout,
			this, &SessionsListModel::flushLiveSessionListUpdate);
}

SessionsListModel::~SessionsListModel()
{
	clearTree();
	delete m_root;
	qDeleteAll(m_sessions);
	m_sessions.clear();
}

void SessionsListModel::clearTree()
{
	for (SessionTreeItem* child : m_root->children)
	{
		for (SessionTreeItem* grandChild : child->children)
		{
			delete grandChild;
		}
		delete child;
	}
	m_root->children.clear();
	m_liveTreeNodes.clear();
}

void SessionsListModel::sortSessionList()
{
	std::sort(m_sessions.begin(), m_sessions.end(),
			  [](Session* a, Session* b)
			  {
				  if (!a || !b) return false;

				  if (a->getType() == Session::LiveSession) return true;
				  if (b->getType() == Session::LiveSession) return false;

				  return *b < *a;
			  });
}

void SessionsListModel::rebuildTree()
{
	clearTree();

	sortSessionList();

	QMap<QDate, SessionTreeItem*> dayFolders;

	for (Session* session : m_sessions)
	{
		if (!session)
		{
			continue;
		}

		const int flatIndex = m_sessions.indexOf(session);

		if (session->getType() == Session::LiveSession)
		{
			auto* node = new SessionTreeItem();
			node->kind = SessionTreeItem::Kind::Session;
			node->session = session;
			node->flatIndex = flatIndex;
			node->parent = m_root;
			m_root->children.append(node);
			m_liveTreeNodes.insert(node);
			continue;
		}

		const QDate day = session->getCreatedAt().date();
		SessionTreeItem* folder = dayFolders.value(day);
		if (!folder)
		{
			folder = new SessionTreeItem();
			folder->kind = SessionTreeItem::Kind::DayFolder;
			folder->folderDate = day;
			folder->parent = m_root;
			dayFolders.insert(day, folder);
			m_liveTreeNodes.insert(folder);
		}

		auto* node = new SessionTreeItem();
		node->kind = SessionTreeItem::Kind::Session;
		node->session = session;
		node->flatIndex = flatIndex;
		node->parent = folder;
		folder->children.append(node);
		m_liveTreeNodes.insert(node);
	}

	QList<QDate> days = dayFolders.keys();
	std::sort(days.begin(), days.end(), std::greater<QDate>());
	for (const QDate& day : days)
	{
		m_root->children.append(dayFolders.value(day));
	}
}

SessionsListModel::SessionTreeItem* SessionsListModel::findSessionNode(Session* session) const
{
	if (!session)
	{
		return nullptr;
	}

	for (SessionTreeItem* top : m_root->children)
	{
		if (top->kind == SessionTreeItem::Kind::Session && top->session == session)
		{
			return top;
		}

		for (SessionTreeItem* child : top->children)
		{
			if (child->session == session)
			{
				return child;
			}
		}
	}

	return nullptr;
}

SessionsListModel::SessionTreeItem* SessionsListModel::findDayFolder(const QDate& day) const
{
	for (SessionTreeItem* child : m_root->children)
	{
		if (child->kind == SessionTreeItem::Kind::DayFolder && child->folderDate == day)
		{
			return child;
		}
	}

	return nullptr;
}

int SessionsListModel::dayFolderInsertRow(const QDate& day) const
{
	int row = 0;
	for (SessionTreeItem* child : m_root->children)
	{
		if (child->kind == SessionTreeItem::Kind::DayFolder && child->folderDate < day)
		{
			return row;
		}

		row++;
	}

	return row;
}

bool SessionsListModel::insertSessionIntoTree(Session* session)
{
	if (!session)
	{
		return false;
	}

	const int flatIndex = m_sessions.indexOf(session);

	if (session->getType() == Session::LiveSession)
	{
		auto* node = new SessionTreeItem();
		node->kind = SessionTreeItem::Kind::Session;
		node->session = session;
		node->flatIndex = flatIndex;
		node->parent = m_root;

		const int row = 0;
		beginInsertRows(QModelIndex(), row, row);
		m_root->children.insert(row, node);
		m_liveTreeNodes.insert(node);
		endInsertRows();
		return true;
	}

	const QDate day = session->getCreatedAt().date();
	SessionTreeItem* folder = findDayFolder(day);
	if (!folder)
	{
		folder = new SessionTreeItem();
		folder->kind = SessionTreeItem::Kind::DayFolder;
		folder->folderDate = day;
		folder->parent = m_root;

		const int folderRow = dayFolderInsertRow(day);
		beginInsertRows(QModelIndex(), folderRow, folderRow);
		m_root->children.insert(folderRow, folder);
		m_liveTreeNodes.insert(folder);
		endInsertRows();
	}

	const int folderRow = m_root->children.indexOf(folder);
	const QModelIndex folderIndex = index(folderRow, 0);

	auto* node = new SessionTreeItem();
	node->kind = SessionTreeItem::Kind::Session;
	node->session = session;
	node->flatIndex = flatIndex;
	node->parent = folder;

	const int sessionRow = 0;
	beginInsertRows(folderIndex, sessionRow, sessionRow);
	folder->children.insert(sessionRow, node);
	m_liveTreeNodes.insert(node);
	endInsertRows();

	notifySubtreeVisualRefresh(folderIndex);

	const int rootRows = rowCount();
	if (rootRows > 0)
	{
		emit dataChanged(index(0, 0), index(rootRows - 1, 0));
	}

	return true;
}

void SessionsListModel::removeSessionFromTree(Session* session)
{
	SessionTreeItem* node = findSessionNode(session);
	if (!node || !node->parent)
	{
		return;
	}

	SessionTreeItem* parentItem = node->parent;
	const int row = parentItem->children.indexOf(node);
	if (row < 0)
	{
		return;
	}

	const QModelIndex parentIndex = parentItem == m_root
		? QModelIndex()
		: indexForItem(parentItem);

	beginRemoveRows(parentIndex, row, row);
	parentItem->children.removeAt(row);
	m_liveTreeNodes.remove(node);
	endRemoveRows();
	delete node;

	if (parentItem != m_root && parentItem->children.isEmpty())
	{
		const int folderRow = m_root->children.indexOf(parentItem);
		if (folderRow >= 0)
		{
			beginRemoveRows(QModelIndex(), folderRow, folderRow);
			m_root->children.removeAt(folderRow);
			m_liveTreeNodes.remove(parentItem);
			endRemoveRows();
			delete parentItem;
		}
	}
	else if (parentIndex.isValid())
	{
		notifySubtreeVisualRefresh(parentIndex);
	}

	const int rootRows = rowCount();
	if (rootRows > 0)
	{
		emit dataChanged(index(0, 0), index(rootRows - 1, 0));
	}
}

void SessionsListModel::notifySubtreeVisualRefresh(const QModelIndex& parentIndex)
{
	const int rows = rowCount(parentIndex);
	if (rows <= 0)
	{
		return;
	}

	const QModelIndex topLeft = index(0, 0, parentIndex);
	const QModelIndex bottomRight = index(rows - 1, 0, parentIndex);
	emit dataChanged(topLeft, bottomRight);
}

SessionsListModel::SessionTreeItem* SessionsListModel::itemAt(const QModelIndex& index) const
{
	if (!index.isValid())
	{
		return nullptr;
	}

	auto* item = static_cast<SessionTreeItem*>(index.internalPointer());
	if (!item || !m_liveTreeNodes.contains(item))
	{
		return nullptr;
	}

	return item;
}

void SessionsListModel::updateFlatIndices()
{
	for (int i = 0; i < m_sessions.size(); ++i)
	{
		Session* session = m_sessions.at(i);
		SessionTreeItem* node = findSessionNode(session);
		if (node)
		{
			node->flatIndex = i;
		}
	}
}

QModelIndex SessionsListModel::indexForItem(const SessionTreeItem* item) const
{
	if (!item || item == m_root)
	{
		return QModelIndex();
	}

	SessionTreeItem* parentItem = item->parent;
	if (!parentItem)
	{
		return QModelIndex();
	}

	const int row = parentItem->children.indexOf(const_cast<SessionTreeItem*>(item));
	if (row < 0)
	{
		return QModelIndex();
	}

	return createIndex(row, 0, const_cast<SessionTreeItem*>(item));
}

QModelIndex SessionsListModel::indexForFlatSession(int flatIndex) const
{
	if (flatIndex < 0 || flatIndex >= m_sessions.size())
	{
		return QModelIndex();
	}

	Session* session = m_sessions.at(flatIndex);
	SessionTreeItem* node = findSessionNode(session);
	return indexForItem(node);
}

QModelIndex SessionsListModel::index(int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex(row, column, parent))
	{
		return QModelIndex();
	}

	SessionTreeItem* parentItem = parent.isValid() ? itemAt(parent) : m_root;
	if (!parentItem)
	{
		return QModelIndex();
	}

	if (row < 0 || row >= parentItem->children.size())
	{
		return QModelIndex();
	}

	return createIndex(row, column, parentItem->children.at(row));
}

QModelIndex SessionsListModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
	{
		return QModelIndex();
	}

	auto* item = static_cast<SessionTreeItem*>(index.internalPointer());
	if (!item || !m_liveTreeNodes.contains(item))
	{
		return QModelIndex();
	}

	if (!item->parent || item->parent == m_root)
	{
		return QModelIndex();
	}

	SessionTreeItem* parentItem = item->parent;
	if (!parentItem->parent)
	{
		return QModelIndex();
	}

	const int row = parentItem->parent->children.indexOf(parentItem);
	if (row < 0)
	{
		return QModelIndex();
	}

	return createIndex(row, 0, parentItem);
}

int SessionsListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.column() > 0)
	{
		return 0;
	}

	SessionTreeItem* parentItem = parent.isValid() ? itemAt(parent) : m_root;

	return parentItem ? parentItem->children.size() : 0;
}

int SessionsListModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 1;
}

Q_INVOKABLE void SessionsListModel::startRecording ()
{
	setRecordingState(true);
}

Q_INVOKABLE void SessionsListModel::stopRecording()
{
	setRecordingState(false);
}

Q_INVOKABLE void SessionsListModel::setRecordingState(bool enable)
{
	m_recording = enable;

	if (m_liveSession)
	{
		updateSessionInList(m_liveSession, {RecordingRole});
	}
}

QVariant SessionsListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
	{
		return QVariant();
	}

	auto* item = itemAt(index);
	if (!item)
	{
		return QVariant();
	}

	if (item->kind == SessionTreeItem::Kind::DayFolder)
	{
		switch (role)
		{
		case Qt::DecorationRole:
			return QIcon(":/Resources/icons8-folder-24.png");
		case SessionNameRole:
			return item->folderDate.toString("dd-MM-yyyy");
		case IsDayFolderRole:
			return true;
		case FlatSessionIndexRole:
			return -1;
		case Qt::DisplayRole:
			return item->folderDate.toString("dd-MM-yyyy");
		default:
			return QVariant();
		}
	}

	Session* session = item->session;
	if (!session)
	{
		return QVariant();
	}

	const bool isLive = session->getType() == Session::LiveSession;
	const bool isRecorded = session->getType() == Session::RecordedSession;

	switch (role)
	{
		case Qt::DecorationRole:
			if (isLive)
			{
				return QIcon(":/Resources/icons8-live-24.png");
			}
			return QIcon(":/Resources/icons8-list-24.png");
		case SessionIdRole:
			return session->getId();
		case SessionNameRole:
			return session->getName();
		case CreatedAtRole:
			return session->getCreatedAt();
		case CreatedAtFormattedRole:
			if (isLive)
			{
				return session->getCreatedAt().toString("dd.MM.yyyy hh:mm");
			}
			return session->getCreatedAt().toString("hh:mm");
		case MessageCountRole:
			return session->getMessageCount();
		case ParameterCountRole:
			return session->getParameterCount();
		case DescriptionRole:
			return session->getDescription();
		case Qt::DisplayRole:
			if (isRecorded)
			{
				return QString("%1 | %2 msgs")
					.arg(session->getCreatedAt().toString("hh:mm"))
					.arg(session->getMessageCount());
			}
			return QString("%1 (%2 сообщений)").arg(session->getName()).arg(session->getMessageCount());
		case RecordedSessionRole:
			return isRecorded;
		case RecordingRole:
			return session->isRecording();
		case SessionTypeRole:
			return static_cast<int>(session->getType());
		case IsLiveSessionRole:
			return isLive;
		case IsDayFolderRole:
			return false;
		case FlatSessionIndexRole:
			return item->flatIndex;
		case ParametersModelRole:
			return QVariant::fromValue(session->parametersModel());
		case PlayerModelRole:
			return QVariant::fromValue(session->player());
		case ChartsModelRole:
			return QVariant::fromValue(session->chartsModel());
		case TestColorRole:
			return item->flatIndex > 1
				? QVariant::fromValue(QColor::fromRgb(255, 0, 0))
				: QVariant::fromValue(QColor::fromRgb(255, 225, 0));
		case IconRole:
			if (isLive)
			{
				return ":/Resources/icons8-live-24.png";
			}
			return ":/Resources/icons8-list-24.png";
		case SaveProgressRole:
			if (isLive)
				return m_liveSessionSaveProgress;
			return -1;
		case LoadProgressRole:
			if (isRecorded)
			{
				auto* rs = qobject_cast<RecordedSession*>(session);
				return rs ? rs->loadProgress() : -1;
			}
			return -1;
		default:
			return QVariant();
	}
}

void SessionsListModel::setSaveProgress(int pct)
{
	m_liveSessionSaveProgress = pct;
	SessionTreeItem* liveNode = m_liveSession ? findSessionNode(m_liveSession) : nullptr;
	if (liveNode)
	{
		const QModelIndex idx = indexForItem(liveNode);
		emit dataChanged(idx, idx, {SaveProgressRole});
	}
}

QHash<int, QByteArray> SessionsListModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[SessionIdRole] = "sessionId";
	roles[SessionNameRole] = "sessionName";
	roles[CreatedAtRole] = "createdAt";
	roles[CreatedAtFormattedRole] = "createdAtFormatted";
	roles[MessageCountRole] = "messageCount";
	roles[ParameterCountRole] = "parameterCount";
	roles[DescriptionRole] = "description";
	roles[RecordingRole] = "recordingRole";
	roles[RecordedSessionRole] = "recordedSession";
	roles[SessionTypeRole] = "sessionType";
	roles[IsLiveSessionRole] = "isLiveSession";
	roles[ParametersModelRole] = "parametersModel";
	roles[PlayerModelRole] = "playerModel";
	roles[ChartsModelRole] = "chartModel";
	roles[TestColorRole] = "testColorRole";
	roles[SaveProgressRole] = "saveProgress";
	roles[LoadProgressRole] = "loadProgress";
	roles[IsDayFolderRole] = "isDayFolder";
	roles[FlatSessionIndexRole] = "flatSessionIndex";

	return roles;
}

QVariantMap SessionsListModel::get(int index) const
{
	QVariantMap result;
	if (index < 0 || index >= sessionCount())
		return result;

	QModelIndex modelIndex = indexForFlatSession(index);
	for (int role : roleNames().keys()) {
		result[roleNames().value(role)] = data(modelIndex, role);
	}
	return result;
}

void SessionsListModel::setReader(BoardMessagesSqliteReader *reader)
{
	if (m_reader != reader) 
	{
		m_reader = reader;
		m_recordedSessionsFactory->setReader(reader);
		refreshSessions();
	}
}

void SessionsListModel::refreshSessions()
{
	if (!m_reader) 
	{
		qWarning() << "SessionsListModel: Reader is not set";
		emit errorOccurred("Reader не установлен");
		return;
	}
	
	beginResetModel();
	
	// Очищаем существующие сессии
	m_liveSession = nullptr;
	m_liveSessionFactory->releaseLiveSession();
	qDeleteAll(m_sessions);
	m_sessions.clear();
	
	// Создаем живую сессию, если она еще не создана и добавляем в список
	createLiveSession();
	
	// Создаем записанные сессии через фабрику
	QList<Session*> recordedSessions = m_recordedSessionsFactory->createSessions();
	

	m_sessions.append(recordedSessions);
	for (Session* session : recordedSessions)
	{
		connectSessionSignals(session);
	}
	
	// Сортируем сессии и строим дерево
	sortSessionList();
	rebuildTree();
	
	qDebug() << "SessionsListModel: Загружено" << m_sessions.size() << "сессий";
	
	endResetModel();
	emit sessionsRefreshed();
}

Session* SessionsListModel::getSession(int index) const
{
	if (index >= 0 && index < m_sessions.size())
	{
		return m_sessions.at(index);
	}
	return nullptr;
}

Session* SessionsListModel::getSessionById(int sessionId) const
{
	if (sessionId == -1)
	{
		return liveSession();
	}

	for (auto session : m_sessions)
	{
		if (session->getId() == sessionId)
		{
			return session;
		}
	}

	return nullptr;
}

LiveSession* SessionsListModel::liveSession() const
{
	if (m_sessions.isEmpty()) return nullptr;

	return dynamic_cast<LiveSession*>(m_sessions[0]);
}

BoardMessagesSqliteReader::SessionInfo SessionsListModel::getSessionInfo(int index) const
{
	if (index >= 0 && index < m_sessions.size()) 
	{
		Session* session = m_sessions.at(index);
		if (session && session->getType() == Session::RecordedSession)
		{
			RecordedSession* recordedSession = qobject_cast<RecordedSession*>(session);
			if (recordedSession)
			{
				return recordedSession->getSessionInfo();
			}
		}
	}
	
	BoardMessagesSqliteReader::SessionInfo emptyInfo;
	emptyInfo.id = -1;
	return emptyInfo;
}

int SessionsListModel::getSessionId(int index) const
{
	if (index >= 0 && index < m_sessions.size()) 
	{
		Session* session = m_sessions.at(index);
		if (session)
		{
			return session->getId();
		}
	}
	return -1;
}

void SessionsListModel::addRecordedSession(const BoardMessagesSqliteReader::SessionInfo &sessionInfo)
{
	// Проверяем, не существует ли уже сессия с таким ID
	for (int i = 0; i < m_sessions.size(); ++i)
	{
		Session* session = m_sessions.at(i);
		if (session && session->getId() == sessionInfo.id)
		{
			qDebug() << "SessionsListModel: Session with ID" << sessionInfo.id << "already exists";
			return;
		}
	}
	
	// Создаем новую записанную сессию через фабрику
	Session* newSession = m_recordedSessionsFactory->createSession(sessionInfo);
	if (newSession)
	{
		addSessionToList(newSession);
		qDebug() << "SessionsListModel: Added new recorded session" << sessionInfo.id << "with name" << sessionInfo.name;
	}
}

void SessionsListModel::updateSessionMessageCount(int sessionId, int messageCount)
{
	int index = findSessionIndex(sessionId);
	if (index >= 0 && index < m_sessions.size())
	{
		Session* session = m_sessions.at(index);
		if (session && session->getType() == Session::RecordedSession)
		{
			RecordedSession* recordedSession = qobject_cast<RecordedSession*>(session);
			if (recordedSession)
			{
				recordedSession->updateMessageCount(messageCount);
				qDebug() << "SessionsListModel: Updated message count for session" << sessionId << "to" << messageCount;
			}
		}
	}
}

void SessionsListModel::removeSession(int index)
{
	if (index < 0 || index >= m_sessions.size())
	{
		qWarning() << "SessionsListModel: Invalid index for session removal:" << index;
		return;
	}
	
	Session* session = m_sessions.at(index);
	if (!session)
	{
		qWarning() << "SessionsListModel: Session at index" << index << "is null";
		return;
	}
	
	// Получаем ID сессии для удаления из базы данных (только для записанных сессий)
	int sessionId = session->getId();
	const int flatIndex = index;

	emit sessionAboutToBeRemoved(flatIndex);

	disconnect(session, nullptr, this, nullptr);

	// Удаляем сессию из списка
	removeSessionFromTree(session);
	m_sessions.removeAt(index);
	updateFlatIndices();

	// Сбрасываем указатель на активную сессию если она была удалена.
	// Явный выбор следующей сессии остаётся за вызывающим кодом (deleteRecord),
	// поэтому не вызываем switchToLiveSession/switchToSession — они сбросят
	// хранилища сессий, которые могут уже загружаться в фоне.
	if (m_currentActiveSession == session)
	{
		m_currentActiveSession = nullptr;
	}
	
	qDebug() << "SessionsListModel: Removed session at index" << index << "with ID" << sessionId;

	emit sessionRemovedAt(flatIndex);
	
	// Удаляем сессию из базы данных только для записанных сессий
	if (session->getType() == Session::RecordedSession && m_reader && sessionId > 0)
	{
		if (!m_reader->removeSession(sessionId))
		{
			qWarning() << "SessionsListModel: Failed to remove session" << sessionId << "from database";
			emit errorOccurred(QString("Не удалось удалить сессию %1 из базы данных").arg(sessionId));
		}
	}
	
	// Удаляем объект сессии
	session->deleteLater();
}

int SessionsListModel::findSessionIndex(int sessionId) const
{
	for (int i = 0; i < m_sessions.size(); ++i)
	{
		Session* session = m_sessions.at(i);
		if (session && session->getId() == sessionId)
		{
			return i;
		}
	}
	return -1;
}

bool SessionsListModel::createLiveSession()
{
	if (m_liveSession) return true;

	m_liveSession = m_liveSessionFactory->createLiveSession();
	if (m_liveSession)
	{
		connectSessionSignals(m_liveSession);

		m_currentActiveSession = m_liveSession;
			
		m_sessions.append(m_liveSession);
	}

	return m_liveSession;
}

void SessionsListModel::updateLiveSessionCounters()
{
	if (m_liveSession)
	{
		// Обновляем счетчик сообщений (каждое сообщение от драйвера)
		m_liveSessionFactory->incrementMessageCount();
		
		// Счетчик параметров будет обновляться автоматически через сигналы
		// когда параметры добавляются в хранилище
	}
}

void SessionsListModel::resetLiveSessionCounters()
{
	if (m_liveSession)
	{
		m_liveSessionFactory->resetCounters();
		
		// Обновляем отображение живой сессии в интерфейсе
		updateSessionInList(m_liveSession);
		
		qDebug() << "SessionsListModel: Reset live session counters";
	}
}

void SessionsListModel::onRecordedSessionsCreated(const QList<Session*>& sessions)
{
	// Этот метод вызывается фабрикой при создании записанных сессий
	// Сессии уже добавлены в список в методе refreshSessions
	qDebug() << "SessionsListModel: Received" << sessions.size() << "recorded sessions from factory";
}

void SessionsListModel::onLiveSessionCreated(Session*)
{
	// Этот метод вызывается фабрикой при создании живой сессии
	qDebug() << "SessionsListModel: Live session created by factory";
}

void SessionsListModel::onSessionChanged()
{
	Session* changedSession = qobject_cast<Session*>(sender());
	if (!changedSession)
	{
		return;
	}

	if (auto* recordedSession = qobject_cast<RecordedSession*>(changedSession))
	{
		if (recordedSession->loadProgress() >= 0)
		{
			updateSessionInList(changedSession, {LoadProgressRole});
			return;
		}
	}

	updateSessionInList(changedSession);
}

void SessionsListModel::onMessageCountChanged(int)
{
	Session* changedSession = qobject_cast<Session*>(sender());
	if (!changedSession)
	{
		return;
	}

	static const QVector<int> kMessageRoles = {
		MessageCountRole,
		Qt::DisplayRole,
		CreatedAtFormattedRole
	};

	if (changedSession->getType() == Session::LiveSession)
	{
		m_liveListUpdatePending = true;
		if (!m_liveListUpdateTimer->isActive())
		{
			m_liveListUpdateTimer->start();
		}
		return;
	}

	updateSessionInList(changedSession, kMessageRoles);
}

void SessionsListModel::onParameterCountChanged(int)
{
	Session* changedSession = qobject_cast<Session*>(sender());
	if (changedSession)
	{
		updateSessionInList(changedSession, {ParameterCountRole});
	}
}

void SessionsListModel::flushLiveSessionListUpdate()
{
	if (!m_liveListUpdatePending || !m_liveSession)
	{
		return;
	}

	m_liveListUpdatePending = false;
	static const QVector<int> kMessageRoles = {
		MessageCountRole,
		Qt::DisplayRole,
		CreatedAtFormattedRole
	};
	updateSessionInList(m_liveSession, kMessageRoles);
}

void SessionsListModel::connectSessionSignals(Session* session)
{
	if (!session)
	{
		return;
	}

	connect(session, &Session::sessionChanged,
			this, &SessionsListModel::onSessionChanged, Qt::UniqueConnection);
	connect(session, &Session::messageCountChanged,
			this, &SessionsListModel::onMessageCountChanged, Qt::UniqueConnection);
	connect(session, &Session::parameterCountChanged,
			this, &SessionsListModel::onParameterCountChanged, Qt::UniqueConnection);
}

void SessionsListModel::addSessionToList(Session* session)
{
	if (!session) return;

	connectSessionSignals(session);

	m_sessions.append(session);
	sortSessionList();

	const int flatIndex = m_sessions.indexOf(session);
	insertSessionIntoTree(session);
	updateFlatIndices();

	emit sessionInsertedAt(flatIndex, session);
}

void SessionsListModel::removeSessionFromList(Session* session)
{
	if (!session) return;

	const int flatIndex = m_sessions.indexOf(session);
	if (flatIndex < 0)
	{
		return;
	}

	removeSessionFromTree(session);
	m_sessions.removeAt(flatIndex);
	updateFlatIndices();
	emit sessionRemovedAt(flatIndex);
}

void SessionsListModel::updateSessionInList(Session* session, const QVector<int>& roles)
{
	if (!session) return;

	SessionTreeItem* node = findSessionNode(session);
	if (!node)
	{
		return;
	}

	const QModelIndex modelIndex = indexForItem(node);
	if (roles.isEmpty())
	{
		emit dataChanged(modelIndex, modelIndex);
	}
	else
	{
		emit dataChanged(modelIndex, modelIndex, roles);
	}
}

void SessionsListModel::switchToSession(int sessionIndex)
{
	if (sessionIndex < 0 || sessionIndex >= m_sessions.size())
	{
		qWarning() << "SessionsListModel: Invalid session index" << sessionIndex;
		return;
	}
	
	Session* session = m_sessions.at(sessionIndex);
	if (!session)
	{
		qWarning() << "SessionsListModel: Session at index" << sessionIndex << "is null";
		return;
	}
	
	// Очищаем хранилища всех других RecordedSession
	for (int i = 0; i < m_sessions.size(); ++i)
	{
		if (i != sessionIndex)
		{
			Session* otherSession = m_sessions.at(i);
			if (otherSession && otherSession->getType() == Session::RecordedSession)
			{
				otherSession->clearStorage();
			}
		}
	}
	
	// Устанавливаем текущую активную сессию
	m_currentActiveSession = session;
	
	qDebug() << "SessionsListModel: Switched to session" << session->getName() << "at index" << sessionIndex;
	
	// Эмитируем сигнал о смене сессии
	emit sessionSwitched(session);
}

void SessionsListModel::switchToLiveSession()
{
	if (!m_liveSession)
	{
		qWarning() << "SessionsListModel: Live session is not available";
		return;
	}
	
	// Очищаем хранилища всех RecordedSession
	for (Session* session : m_sessions)
	{
		if (session && session->getType() == Session::RecordedSession)
		{
			session->clearStorage();
		}
	}
	
	// Устанавливаем текущую активную сессию
	m_currentActiveSession = m_liveSession;
	
	qDebug() << "SessionsListModel: Switched to live session";
	
	// Эмитируем сигнал о смене сессии
	emit sessionSwitched(m_liveSession);
}

Q_INVOKABLE void SessionsListModel::resetLiveSession()
{
	if (liveSession() == nullptr) return;

	liveSession()->storage()->clear();

	liveSession()->player()->reset();
}

void SessionsListModel::selectSession(int sessionIndex)
{
	if (sessionIndex < 0 || sessionIndex >= m_sessions.size())
	{
		qWarning() << "SessionsListModel: Invalid session index" << sessionIndex;
		return;
	}

	Session* session = m_sessions.at(sessionIndex);
	if (!session)
	{
		qWarning() << "SessionsListModel: Session at index" << sessionIndex << "is null";
		return;
	}

	// Если это RecordedSession, запускаем асинхронную загрузку
	if (session->getType() == Session::RecordedSession)
	{
		RecordedSession* recordedSession = qobject_cast<RecordedSession*>(session);
		if (recordedSession && m_reader)
		{
			recordedSession->loadDataFromDatabaseAsync(m_reader->getDatabasePath());
		}
		return; // open() будет вызван после загрузки внутри loadDataFromDatabaseAsync
	}

	session->open();

	// qInfo() << "SessionsListModel: Selected session at index" << m_selectedIndex << ":" << session->getName();
}
