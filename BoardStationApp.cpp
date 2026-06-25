#include "BoardStationApp.h"

#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

#include "./Model/Parameters/Tree/ParameterTreeStorage.h"
#include "./Model/Parameters/Tree/ParameterTreeJsonParser.h"

#include "Model/Parameters/BoardMessagesSqliteWriter.h"
#include "Model/Parameters/BoardMessagesSqliteReader.h"
#include "Model/Parameters/WriteTreeWorker.h"
#include "Model/Parameters/Tree/ParameterTreeItem.h"
#include "Model/Parameters/Tree/ParameterTreeHistoryItem.h"


#include <iostream>

#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/system/system_error.hpp>

BoardStationApp::BoardStationApp(int &argc, char **argv)
	: QApplication(argc, argv)
	, m_driverAdapter(nullptr)
	, m_boardMessagesWriter(new BoardMessagesSqliteWriter("BoardStationData.db", this))
	, m_boardMessagesReader(new BoardMessagesSqliteReader("BoardStationData.db", this))
{
	m_sessionsListModel = new SessionsListModel(this);
	m_sessionsListModel->setReader(m_boardMessagesReader);
	
	m_driverAdapter = new DriverAdapter(this);
			 
	setupUplinkParameters();

	m_debugViewModel = new DebugViewModel(this);

	connectSignals(); 

	m_driverAdapter->startListening();
}

void BoardStationApp::setupUplinkParameters()
{
	m_uplinkParametersModel = new UplinkParametersTreeModel(this);

	QString configPath = QApplication::applicationDirPath() + "/configuration.json";

	ParameterTreeJsonParser parser(this);

	QFile configFile(configPath);
	configFile.open(QIODevice::ReadOnly | QIODevice::Text);
	QTextStream textStream(&configFile);
	auto jsonContent = textStream.readAll();
	configFile.close();
	auto storage = parser.parseJson(jsonContent);

	m_uplinkParametersModel->setSnapshot(storage, false);
}

void BoardStationApp::connectSignals()
{
	if (liveSession())
	{
		// Передаем пришедшие от драйвера данные в хранилище
		connect(m_driverAdapter, &DriverAdapter::parameterTreeReceived,
			liveSession()->storage(), &ParameterTreeStorage::appendSnapshot);

		// Увеличиваем счетчик сообщений при получении каждого сообщения от драйвера
		connect(m_driverAdapter, &DriverAdapter::parameterTreeReceived,
			liveSession(), [this](ParameterTreeStorage*)
			{
				liveSession()->incrementMessageCount();
			});

		// Логируем первое сообщение от драйвера
		static bool isFirstMessage = true;
		connect(m_driverAdapter, &DriverAdapter::parameterTreeReceived,
			this, [this](ParameterTreeStorage*)
			{
				if (isFirstMessage && m_debugViewModel)
				{
					m_debugViewModel->addInfoMessage(tr("First message received from driver"));
					isFirstMessage = false;
				}
			});
	}

	// Логируем изменения состояния драйвера
	if (m_driverAdapter && m_debugViewModel)
	{
		connect(m_driverAdapter, &DriverAdapter::driverStateChanged,
			this, [this](radio::IDriver::State state)
			{
				const QString driverMessageInfo = tr("Состояние драйвера изменилось: ");
				if (state == radio::IDriver::State::kConnected)
				{
					m_debugViewModel->addInfoMessage(
						driverMessageInfo + tr("Соединение с дроном установлено"));
				}
				else
				{
					m_debugViewModel->addWarningMessage(
						driverMessageInfo + tr("Соединение с дроном разорвано"));
				}
			});
	}

	 //connect(m_uplinkParametersModel, &UplinkParametersModel::parameterChanged,
	 //	m_driverAdapter, &DriverAdapter::sendParameter);

	connect(this, &QApplication::aboutToQuit, this, &BoardStationApp::close);
}

void BoardStationApp::close()
{
	for (auto i = 0; i < m_sessionsListModel->sessionCount(); i++)
	{
		auto session = m_sessionsListModel->getSession(i);
		session->player()->stop();
	}
}

static void collectRows(ParameterTreeItem* node, QList<ParameterRowData>& out)
{
	if (!node) return;
	if (node->type() == ParameterTreeItem::ItemType::History)
	{
		auto* h = static_cast<ParameterTreeHistoryItem*>(node);
		ParameterRowData row;
		row.label = h->fullName();
		row.values = h->values();
		row.timestamps = h->timestamps();
		out.append(row);
	}
	for (auto child : node->children())
		collectRows(child, out);
}

bool BoardStationApp::saveLiveData()
{
	if (!liveSession()) return false;

	if (m_saveThread && m_saveThread->isRunning())
	{
		qWarning() << "BoardStationApp: Save already in progress";
		return false;
	}

	auto live = m_sessionsListModel ? m_sessionsListModel->liveSession() : nullptr;
	if (!live || !m_boardMessagesWriter)
	{
		qWarning() << "BoardStationApp: Required components not available for saving live data";
		return false;
	}

	// Создаём запись сессии в БД на главном потоке (быстро)
	QString sessionName = QString(tr("Record")) + QString(" %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
	m_boardMessagesWriter->createNewSession(sessionName);

	int newSessionId = m_boardMessagesWriter->getCurrentSessionId();
	if (newSessionId <= 0)
	{
		qWarning() << "BoardStationApp: Failed to create new session";
		return false;
	}

	// Копируем данные из дерева на главном потоке (только памятный обход, быстро)
	QList<ParameterRowData> rows;
	collectRows(live->storage(), rows);

	// Запускаем запись в отдельном потоке
	m_saveThread = new QThread(this);
	auto worker = new WriteTreeWorker(m_boardMessagesWriter->getDatabasePath(), newSessionId, rows);
	worker->moveToThread(m_saveThread);

	connect(m_saveThread, &QThread::started, worker, &WriteTreeWorker::run);

	connect(worker, &WriteTreeWorker::progress, this, [this](int pct)
	{
		if (m_sessionsListModel)
			m_sessionsListModel->setSaveProgress(pct);
	});

	connect(worker, &WriteTreeWorker::finished, this, [this, newSessionId, live](bool success)
	{
		if (m_sessionsListModel)
			m_sessionsListModel->setSaveProgress(-1);

		m_saveThread->quit();

		if (success)
		{
			qDebug() << "BoardStationApp: Live tree data saved to session" << newSessionId;
			if (m_sessionsListModel && m_boardMessagesReader)
			{
				auto sessionInfo = m_boardMessagesReader->getSessionInfo(newSessionId);
				if (sessionInfo.id > 0)
				{
					// Счётчик живой сессии — по числу пакетов от борта; в БД раньше
					// COUNT(DISTINCT timestamp) завышался из-за разного времени у параметров.
					if (live)
						sessionInfo.messageCount = live->getMessageCount();
					m_sessionsListModel->addRecordedSession(sessionInfo);
				}
			}
		}
		else
		{
			qWarning() << "BoardStationApp: Failed to write tree data for session" << newSessionId;
		}
	});

	connect(m_saveThread, &QThread::finished, worker, &QObject::deleteLater);
	// Сбрасываем указатель до того, как поток удалится, чтобы не осталось висячего указателя
	connect(m_saveThread, &QThread::finished, this, [this]() { m_saveThread = nullptr; });
	connect(m_saveThread, &QThread::finished, m_saveThread, &QObject::deleteLater);

	m_sessionsListModel->setSaveProgress(0);
	m_saveThread->start();

	return true;
}

void BoardStationApp::removeRecordFromDatabase(int index)
{
	if (!m_sessionsListModel)
		return;

	// Проверяем диапазон
	if (index < 0 || index >= m_sessionsListModel->sessionCount())
		return;

	// Получаем сессию
	auto session = m_sessionsListModel->getSession(index);
	if (!session)
		return;

	// Живую сессию удалять нельзя
	if (session == liveSession())
	{
		qWarning() << "BoardStationApp: Cannot remove live session";
		return;
	}

	// Удаляем сессию
	m_sessionsListModel->removeSession(index);
}

void BoardStationApp::sendParametersToBoard()
{
	m_driverAdapter->sendParameterTreeSnapshot(m_uplinkParametersModel->storage());
	//QMessageBox::information(nullptr, "info", "send to board");

}

void BoardStationApp::onDriverDataSent(const QString& jsonString)
{
	// Формируем путь к файлу в рабочей директории
	QString filePath = QCoreApplication::applicationDirPath() + "/driver_output.json";

	QFile file(filePath);
	if (!file.open(QIODevice::Append | QIODevice::Text))
	{
		qWarning() << "BoardStationApp: Failed to open driver_output.json for writing";
		return;
	}

	QTextStream out(&file);
	out << jsonString << "\n";

	file.close();

	qDebug() << "BoardStationApp: Data sent to driver logged to" << filePath;
}

BoardStationApp::~BoardStationApp()
{
}


