#include "RecordedSession.h"
#include "./../Model/Parameters/Tree/ParameterTreeStorage.h"
#include "./../Model/Parameters/Tree/ParameterTreeGroupItem.h"
#include "./../Model/Parameters/Tree/ParameterTreeHistoryItem.h"
#include "./../Model/Parameters/ReadTreeWorker.h"
#include <QDebug>
#include <QTimer>
#include "SessionPlayer.h"

RecordedSession::RecordedSession(const BoardMessagesSqliteReader::SessionInfo& sessionInfo, QObject *parent)
	: Session(parent)
	, m_sessionInfo(sessionInfo)
{
	//m_storage = new BoardParameterHistoryStorage(this);

	m_player = new SessionPlayer(this);
	m_player->setStorage(m_treeStorage);

	connect(m_player, &SessionPlayer::played, m_parametersModel, &BoardParametersTreeModel::setSnapshot);

	m_chartsModel->setPlayer(m_player);
	//m_chartsModel->setStorage(m_storage);
}

void RecordedSession::open()
{
	if (!m_opened)
	{
		player()->initialPlay();
	}

	Session::open();
}

void RecordedSession::updateSessionInfo(const BoardMessagesSqliteReader::SessionInfo& sessionInfo)
{
	bool changed = false;
	
	if (m_sessionInfo.messageCount != sessionInfo.messageCount)
	{
		m_sessionInfo.messageCount = sessionInfo.messageCount;
		changed = true;
	}
	
	if (m_sessionInfo.parameterCount != sessionInfo.parameterCount)
	{
		m_sessionInfo.parameterCount = sessionInfo.parameterCount;
		changed = true;
	}
	
	if (m_sessionInfo.name != sessionInfo.name)
	{
		m_sessionInfo.name = sessionInfo.name;
		changed = true;
	}
	
	if (m_sessionInfo.description != sessionInfo.description)
	{
		m_sessionInfo.description = sessionInfo.description;
		changed = true;
	}
	
	if (changed)
	{
		emit sessionChanged();
	}
}

void RecordedSession::updateMessageCount(int count)
{
	if (m_sessionInfo.messageCount != count)
	{
		m_sessionInfo.messageCount = count;
		emit messageCountChanged(count);
		emit sessionChanged();
	}
}

void RecordedSession::updateParameterCount(int count)
{
	if (m_sessionInfo.parameterCount != count)
	{
		m_sessionInfo.parameterCount = count;
		emit parameterCountChanged(count);
		emit sessionChanged();
	}
}

void RecordedSession::clearStorage()
{
	if (m_treeStorage)
	{
		m_treeStorage->clear();
		m_dataLoaded = false; // Сбрасываем флаг после очистки
		qDebug() << "RecordedSession: Tree storage cleared for session" << m_sessionInfo.id;
	}
}

void RecordedSession::loadDataFromDatabase(BoardMessagesSqliteReader* reader)
{
	if (!reader || !m_treeStorage || !m_parametersModel)
	{
		qWarning() << "RecordedSession: Reader, tree storage or model is not available";
		return;
	}

	qDebug() << "RecordedSession: Loading tree data from database for session" << m_sessionInfo.id;

	if (!reader->loadSessionToTree(m_sessionInfo.id, m_treeStorage))
	{
		qWarning() << "RecordedSession: Failed to load session into tree";
		return;
	}

	// initializeWithLoadedData должен идти ПЕРЕД initialPlay,
	// т.к. initialPlay использует m_sessionStartTime
	if (m_player)
	{
		static_cast<SessionPlayer*>(m_player)->initializeWithLoadedData();
	}
	player()->initialPlay();

	m_dataLoaded = true;

	qDebug() << "RecordedSession: Tree data loaded for session" << m_sessionInfo.id;
}

void RecordedSession::loadDataFromDatabaseAsync(const QString& dbPath)
{
	if (m_dataLoaded)
	{
		// Данные уже в памяти — инициализируем плеер и готово
		auto* sp = static_cast<SessionPlayer*>(m_player);
		if (sp) sp->initializeWithLoadedData();
		player()->initialPlay();
		return;
	}

	if (m_loadThread && m_loadThread->isRunning())
	{
		qDebug() << "RecordedSession: load already in progress for session" << m_sessionInfo.id;
		return;
	}

	m_loadThread = new QThread(this);
	auto* worker = new ReadTreeWorker(dbPath, m_sessionInfo.id);
	worker->moveToThread(m_loadThread);

	connect(m_loadThread, &QThread::started, worker, &ReadTreeWorker::run);

	connect(worker, &ReadTreeWorker::progress, this, [this](int pct)
	{
		m_loadProgress = pct;
		emit sessionChanged();
	});

	connect(worker, &ReadTreeWorker::finished, this, [this, worker](bool success)
	{
		m_loadProgress = -1;

		if (success)
		{
			populateTreeFromRows(worker->result());
			m_dataLoaded = true;

			// initializeWithLoadedData должен идти ПЕРЕД initialPlay,
			// т.к. initialPlay использует m_sessionStartTime, который
			// устанавливается внутри initializeWithLoadedData
			auto* sp = static_cast<SessionPlayer*>(m_player);
			if (sp) sp->initializeWithLoadedData();
			player()->initialPlay();

			Session::open(); // помечаем сессию как открытую

			// Откладываем сигнал на следующую итерацию event loop.
			// К тому моменту Qt успеет выполнить отложенный layout в QTreeView
			// (scheduleDelayedItemsLayout из modelReset), и expandAll() в обработчике
			// mainwindow сработает корректно — d->viewItems уже будет заполнен.
			QTimer::singleShot(0, this, [this]() { emit dataLoadCompleted(); });

			qDebug() << "RecordedSession: async load finished for session" << m_sessionInfo.id;
		}
		else
		{
			qWarning() << "RecordedSession: async load failed for session" << m_sessionInfo.id;
		}

		m_loadThread->quit();
		m_loadThread = nullptr;
		emit sessionChanged();
	});

	connect(m_loadThread, &QThread::finished, worker, &QObject::deleteLater);
	connect(m_loadThread, &QThread::finished, m_loadThread, &QObject::deleteLater);

	m_loadProgress = 0;
	emit sessionChanged();
	m_loadThread->start();
}

void RecordedSession::populateTreeFromRows(const QVector<ReadRowData>& rows)
{
	if (!m_treeStorage) return;
	m_treeStorage->clear();

	for (const auto& row : rows)
	{
		const QStringList parts = row.label.split('.');
		if (parts.isEmpty()) continue;

		// Проходим по группам (всё кроме последней части)
		ParameterTreeItem* current = m_treeStorage;
		for (int i = 0; i < parts.size() - 1; ++i)
		{
			const QString& part = parts.at(i);
			auto* existing = current->findChildByLabel(part, false);
			if (!existing)
			{
				auto* group = new ParameterTreeGroupItem(part, current);
				current->appendChild(group);
				current = group;
			}
			else
			{
				current = existing;
			}
		}

		// Последняя часть — узел-история
		const QString& leaf = parts.last();
		auto* existingLeaf = current->findChildByLabel(leaf, false);
		ParameterTreeHistoryItem* historyItem = nullptr;
		if (existingLeaf && existingLeaf->type() == ParameterTreeItem::ItemType::History)
		{
			historyItem = static_cast<ParameterTreeHistoryItem*>(existingLeaf);
		}
		else if (!existingLeaf)
		{
			historyItem = new ParameterTreeHistoryItem(leaf, current);
			current->appendChild(historyItem);
		}

		if (historyItem)
			historyItem->addValue(row.value, row.timestamp);
	}
}

bool RecordedSession::isDataLoaded() const
{
	// Используем флаг вместо проверки childCount(), 
	// чтобы избежать обращения к потенциально удаленному объекту
	return m_dataLoaded;
	
	// Старая проверка через childCount() небезопасна при многопоточности
	// и при удалении объектов:
	// if (!m_treeStorage)
	// {
	//     return false;
	// }
	// return m_treeStorage->childCount() > 0;
}
