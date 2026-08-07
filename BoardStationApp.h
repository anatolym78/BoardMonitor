#ifndef BOARDSTATIONAPP_H
#define BOARDSTATIONAPP_H

#include <QApplication>
#include <QThread>

//#include "./Services/RCDriver/driverinterface.h"

#include "./ViewModel/SessionsListModel.h"
#include "./ViewModel/LiveSession.h"
#include "./ViewModel/RecordedSession.h"

#include "./ViewModel/UplinkParametersTreeModel.h"
#include "./ViewModel/DebugViewModel.h"

#include "./Model/DriverAdapter.h"

#include "./Model/Telemetry/TelemetryIngestService.h"

#include "./Model/Parameters/AppConfigurationReader.h"

#include "./Model/AppSettings.h"

#include "./Services/RCImitator/src/driver.hh"
#include "./Services/RCImitator/src/builder.hh"

#include <boost/dll/shared_library.hpp>

class BoardMessagesSqliteWriter;

class BoardStationApp : public QApplication
{
	Q_OBJECT

public:
	BoardStationApp(int &argc, char **argv);
	~BoardStationApp();

	// Create and configure uplink parameters based on the current configuration
	void setupUplinkParameters();

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

	AppSettings& settings() { return m_settings; }
	const AppSettings& settings() const { return m_settings; }

	LiveSession* liveSession() const { return m_sessionsListModel->liveSession(); }

private slots:
	void onDriverDataSent(const QString& jsonString);
   
private:
	std::unique_ptr<radio::IDriverBuilder> m_driverBuilder;
	boost::dll::shared_library m_rcImitatorPluginLibrary;
	std::shared_ptr<radio::IDriver> m_driverHolder;
	AppSettings m_settings;
	DriverAdapter *m_driverAdapter;
	TelemetryIngestService* m_telemetryIngestService = nullptr;
	SessionsListModel *m_sessionsListModel;

	UplinkParametersTreeModel *m_uplinkParametersModel;
	DebugViewModel *m_debugViewModel;

	BoardMessagesSqliteWriter* m_boardMessagesWriter;
	BoardMessagesSqliteReader* m_boardMessagesReader;

	QThread* m_saveThread = nullptr;   // активный поток сохранения (nullptr = нет)

private:
	void connectSignals();
};



#endif // BOARDSTATIONAPP_H
