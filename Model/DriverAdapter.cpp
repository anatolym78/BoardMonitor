#include "DriverAdapter.h"
#include "./Parameters/Tree/ParameterTreeJsonParser.h"
#include "./Parameters/Tree/ParameterTreeStorage.h"
#include "Model/Telemetry/TelemetryIngestService.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include <boost/filesystem.hpp>

DriverAdapter::DriverAdapter(QObject *parent)
	: QObject(parent)
	, m_treeJsonParser(new ParameterTreeJsonParser(this))
{
	if (!m_settings.load())
	{
		qCritical() << "DriverAdapter: failed to load settings from"
		            << AppSettings::settingsFilePath();
		return;
	}

	createDriver(m_settings.currentPlugin());
}

DriverAdapter::~DriverAdapter()
{
	destroyDriver();
}

std::optional<boost::filesystem::path> DriverAdapter::resolvePluginPath(const QString& pluginName) const
{
	if (pluginName.isEmpty())
	{
		return std::nullopt;
	}

	namespace fs = boost::filesystem;
	const fs::path pluginDir = boost::dll::program_location().parent_path();
	const fs::path pluginPath = pluginDir / pluginName.toStdString();

	const QString pluginDirQt = QString::fromStdString(pluginDir.string());
	const QString libSuffix = QString::fromStdString(boost::dll::shared_library::suffix().string());
	const QString expectedFile = pluginName + libSuffix;

	if (!QDir(pluginDirQt).exists(expectedFile))
	{
		qCritical() << "DriverAdapter: plugin file not found:"
		            << QDir(pluginDirQt).filePath(expectedFile);
		return std::nullopt;
	}

	return pluginPath;
}

bool DriverAdapter::createDriver(const QString& pluginName)
{
	const auto pluginPath = resolvePluginPath(pluginName);
	if (!pluginPath)
	{
		return false;
	}

	qInfo() << "DriverAdapter: loading plugin" << pluginName;

	std::unique_ptr<radio::IDriverBuilder> builder;
	try
	{
		if (m_pluginLibrary.is_loaded())
		{
			m_pluginLibrary.unload();
		}

		m_pluginLibrary.load(*pluginPath, boost::dll::load_mode::append_decorations);

		using CreateBuilderFn = radio::IDriverBuilder* (*)();
		CreateBuilderFn create_builder =
			m_pluginLibrary.get<CreateBuilderFn>("dllCreateBuilder");

		builder.reset(create_builder());
	}
	catch (const boost::system::system_error& ex)
	{
		qCritical() << "DriverAdapter: plugin load failed:" << ex.what();
		if (m_pluginLibrary.is_loaded())
		{
			m_pluginLibrary.unload();
		}
		return false;
	}

	if (!builder)
	{
		qCritical() << "DriverAdapter: failed to create driver builder";
		if (m_pluginLibrary.is_loaded())
		{
			m_pluginLibrary.unload();
		}
		return false;
	}

	m_driver = builder->setDelay(1).enableAck(false).enableShutdown(true).get();
	m_driver->enableShutdown(false);
	m_currentPlugin = pluginName;

	qDebug() << "DriverAdapter: driver object constructed for" << pluginName;
	connectToDriver();
	return true;
}

void DriverAdapter::destroyDriver()
{
	stopListening();
	disconnectFromDriver();
	m_driver.reset();

	if (m_pluginLibrary.is_loaded())
	{
		try
		{
			m_pluginLibrary.unload();
			qInfo() << "DriverAdapter: plugin unloaded";
		}
		catch (const boost::system::system_error& ex)
		{
			qWarning() << "DriverAdapter: unload failed:" << ex.what();
		}
	}

	m_currentPlugin.clear();
}

void DriverAdapter::startListening()
{
	if (m_driver)
	{
		m_driver->start();
		m_isListening = true;
		qInfo() << "DriverAdapter: Started listening";
	}
}

void DriverAdapter::stopListening()
{
	if (m_driver)
	{
		m_driver->stop();
		m_isListening = false;
		qInfo() << "DriverAdapter: Stopped listening";
	}
}

bool DriverAdapter::switchPlugin(const QString& pluginName)
{
	if (pluginName.isEmpty())
	{
		return false;
	}

	if (pluginName == m_currentPlugin && m_driver)
	{
		return true;
	}

	if (!m_settings.hasPlugin(pluginName))
	{
		qWarning() << "DriverAdapter: plugin is not listed in settings:" << pluginName;
		return false;
	}

	const QString previousPlugin = m_currentPlugin;
	const bool wasListening = m_isListening;

	destroyDriver();

	if (!createDriver(pluginName))
	{
		qCritical() << "DriverAdapter: failed to switch to" << pluginName
		            << "; trying to restore" << previousPlugin;
		if (!previousPlugin.isEmpty())
		{
			createDriver(previousPlugin);
			if (m_driver && wasListening)
			{
				startListening();
			}
		}
		return false;
	}

	m_settings.setCurrentPlugin(pluginName);
	if (!m_settings.save())
	{
		qWarning() << "DriverAdapter: plugin switched, but settings.json was not saved";
	}

	if (wasListening)
	{
		startListening();
	}

	emit currentPluginChanged(pluginName);
	qInfo() << "DriverAdapter: switched plugin to" << pluginName;
	return true;
}

void DriverAdapter::onDriverDataAvailable(QString data)
{
	if (!m_driver)
	{
		qWarning() << "DriverAdapter: Driver is not available";
		return;
	}

	if (data.isEmpty())
	{
		qDebug() << "DriverAdapter: Driver data is empty";
		return;
	}

	if (m_ingestService)
	{
		m_ingestService->enqueue(data);
		return;
	}

	createTreeParameters(data);
}

void DriverAdapter::onDriverStateChanged(radio::IDriver::State state)
{
	if (state == radio::IDriver::State::kConnected)
	{
		qInfo() << "DriverAdapter: Driver connected";
	}
	else
	{
		qInfo() << "DriverAdapter: Driver disconnected";
	}

	emit driverStateChanged(state);
}

void DriverAdapter::createTreeParameters(const QString &data)
{
	ParameterTreeStorage* snapshot = m_treeJsonParser->parseJson(data);
	QString error = m_treeJsonParser->getLastError();
	if (!error.isEmpty())
	{
		qDebug() << "DriverAdapter: " << error;
		delete snapshot;
	}
	else
	{
		emit parameterTreeReceived(snapshot);
	}
}

void DriverAdapter::sendParameterTreeSnapshot(ParameterTreeStorage* snapshot)
{
	if (!m_driver)
	{
		qWarning() << "DriverAdapter: cannot send snapshot, driver is not loaded";
		return;
	}

	auto message = m_treeJsonParser->toBoardJson(snapshot);
	m_driver->write(message);
}

void DriverAdapter::setIngestService(TelemetryIngestService* ingestService)
{
	m_ingestService = ingestService;
}

void DriverAdapter::connectToDriver()
{
	if (m_driver && !m_isConnected)
	{
		QObject::connect(m_driver.get(), &radio::IDriver::dataAvailable,
			this, &DriverAdapter::onDriverDataAvailable);

		QObject::connect(m_driver.get(), &radio::IDriver::stateChanged,
			this, &DriverAdapter::onDriverStateChanged);

		m_isConnected = true;
		qInfo() << "DriverAdapter: Connected to driver";
	}
}

void DriverAdapter::disconnectFromDriver()
{
	if (m_driver && m_isConnected)
	{
		QObject::disconnect(m_driver.get(), &radio::IDriver::dataAvailable,
			this, &DriverAdapter::onDriverDataAvailable);

		QObject::disconnect(m_driver.get(), &radio::IDriver::stateChanged,
			this, &DriverAdapter::onDriverStateChanged);
		m_isConnected = false;
		qInfo() << "DriverAdapter: Disconnected from driver";
	}
}
