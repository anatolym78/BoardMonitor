#ifndef BOARDSTATIONAPP_H
#define BOARDSTATIONAPP_H

#include <QApplication>
#include <QThread>

//#include "./Services/RCDriver/driverinterface.h"

#include "./ViewModel/SessionsListModel.h"
#include "./ViewModel/LiveSession.h"
#include "./ViewModel/RecordedSession.h"

#include "./ViewModel/ChatViewGridModel.h"

#include "./ViewModel/UplinkParametersTreeModel.h"
#include "./ViewModel/DebugViewModel.h"

#include "./Model/DriverAdapter.h"

#include "./Model/Parameters/AppConfigurationReader.h"

#include "./Services/RCImitator/src/driver.hh"
#include "./Services/RCImitator/src/builder.hh"

#include <boost/dll/shared_library.hpp>

class EventPrinter : public QObject {
	Q_OBJECT

public slots:
	void onDataAvailable(const QString& data) {
		std::cout << "[SIGNAL] dataAvailable: length=" << data.length()
			<< ", preview: " << data.left(50).toStdString()
			<< (data.length() > 50 ? "..." : "") << std::endl;
	}

	void onStateChanged(radio::IDriver::State state) {
		std::cout << "[SIGNAL] stateChanged: "
			<< (state == radio::IDriver::State::kConnected ? "Connected" : "Disconnected")
			<< std::endl;
	}
};

class BoardMessagesSqliteWriter;

class BoardStationApp : public QApplication
{
	Q_OBJECT

public:
	BoardStationApp(int &argc, char **argv);
	~BoardStationApp();

	// Сохранение живых данных в базу
	bool saveLiveData();

	// Удаление записи из базы данных
	void removeRecordFromDatabase(int index);
	
	// Отправка параметров на борт
	void sendParametersToBoard();
	
	// Метод для корректного закрытия приложения
	void close();

public:	   
	SessionsListModel* sessionsModel() const { return m_sessionsListModel; }

	UplinkParametersTreeModel* getUplinkParametersModel() const { return m_uplinkParametersModel; }
	
	DebugViewModel* getDebugViewModel() const { return m_debugViewModel; }
	  
	BoardMessagesSqliteWriter* getBoardMessagesWriter() const { return m_boardMessagesWriter;}

	BoardMessagesSqliteReader* getBoardMessagesReader() const { return m_boardMessagesReader; }
	
	DriverAdapter* getDriverAdapter() const { return m_driverAdapter; }

	LiveSession* liveSession() const { return m_sessionsListModel->liveSession(); }

private slots:
	void onDriverDataSent(const QString& jsonString);
   
private:
	EventPrinter printer;
	std::unique_ptr<radio::IDriverBuilder> m_driverBuilder;
	/** Удерживает RCImitator_plugin.dll загруженной, пока живёт драйвер из плагина. */
	boost::dll::shared_library m_rcImitatorPluginLibrary;
	std::shared_ptr<radio::IDriver> m_driverHolder;
	DriverAdapter *m_driverAdapter;
	SessionsListModel *m_sessionsListModel;

	UplinkParametersTreeModel *m_uplinkParametersModel;
	DebugViewModel *m_debugViewModel;

	BoardMessagesSqliteWriter* m_boardMessagesWriter;
	BoardMessagesSqliteReader* m_boardMessagesReader;

	QThread* m_saveThread = nullptr;   // активный поток сохранения (nullptr = нет)

private:
	void loadUplinkParameters();
	void connectSignals();
};



#endif // BOARDSTATIONAPP_H
