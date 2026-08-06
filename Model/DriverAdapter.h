#ifndef DRIVERADAPTER_H
#define DRIVERADAPTER_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "./../Services/RCImitator/src/driver.hh"
#include "./../Services/RCImitator/src/builder.hh"

#include <boost/dll/import.hpp>
#include <boost/dll/shared_library.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/filesystem.hpp>
#include <boost/system/system_error.hpp>

#include <memory>
#include <optional>
#include <string>

#include "AppSettings.h"
#include "Parameters/Tree/ParameterTreeStorage.h"

class ParameterTreeJsonParser;
class TelemetryIngestService;

class DriverAdapter : public QObject
{
	Q_OBJECT

public:
	explicit DriverAdapter(AppSettings* settings, QObject *parent = nullptr);
	~DriverAdapter();

	void startListening();
	void stopListening();

	void sendParameterTreeSnapshot(ParameterTreeStorage* snapshot);
	void setIngestService(TelemetryIngestService* ingestService);

	QString currentPlugin() const { return m_currentPlugin; }
	QStringList availablePlugins() const;
	bool switchPlugin(const QString& pluginName);

signals:
	void parameterTreeReceived(ParameterTreeStorage* root);
	void driverStateChanged(radio::IDriver::State state);
	void driverConnected();
	void driverDisconnected();
	void currentPluginChanged(const QString& pluginName);

private:
	void onDriverDataAvailable(QString data);
	void onDriverStateChanged(radio::IDriver::State state);

	bool createDriver(const QString& pluginName);
	void destroyDriver();
	void createTreeParameters(const QString& data);

	void connectToDriver();
	void disconnectFromDriver();

	std::optional<boost::filesystem::path> resolvePluginPath(const QString& pluginName) const;

private:
	AppSettings* m_settings = nullptr;
	QString m_currentPlugin;
	boost::dll::shared_library m_pluginLibrary;
	std::shared_ptr<radio::IDriver> m_driver;
	ParameterTreeJsonParser* m_treeJsonParser;
	TelemetryIngestService* m_ingestService = nullptr;
	bool m_isConnected = false;
	bool m_isListening = false;
};

#endif // DRIVERADAPTER_H
