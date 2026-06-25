#include "DriverAdapter.h"
#include "./Parameters/Tree/ParameterTreeJsonParser.h"
#include "./Parameters/Tree/ParameterTreeStorage.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>

#include <boost/filesystem.hpp>

#include <optional>
#include <vector>

namespace
{

std::optional<boost::filesystem::path> findUniqueDriverPlugin()
{
	namespace fs = boost::filesystem;

	const fs::path pluginDir = boost::dll::program_location().parent_path();
	const QString pluginDirQt = QString::fromStdString(pluginDir.string());
	const QString libSuffix = QString::fromStdString(boost::dll::shared_library::suffix().string());

	std::vector<fs::path> matches;

	const QFileInfoList entries = QDir(pluginDirQt).entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
	for (const QFileInfo& info : entries)
	{
		if (!info.fileName().endsWith(libSuffix, Qt::CaseInsensitive))
		{
			continue;
		}

		const QString stem = info.completeBaseName();
		if (!stem.endsWith(QLatin1String("_plugin")))
		{
			continue;
		}

		matches.push_back(pluginDir / stem.toStdString());
	}

	if (matches.empty())
	{
		qCritical() << "DriverAdapter: no *_plugin library found in" << pluginDirQt;
		return std::nullopt;
	}

	if (matches.size() > 1)
	{
		QStringList names;
		for (const auto& match : matches)
		{
			names << QString::fromStdString(match.filename().string());
		}
		qCritical() << "DriverAdapter: expected exactly one *_plugin library, found"
		            << matches.size() << ":" << names.join(", ");
		return std::nullopt;
	}

	return matches.front();
}

} // namespace

DriverAdapter::DriverAdapter(QObject *parent)
    : QObject(parent)
    , m_treeJsonParser(new ParameterTreeJsonParser(this))
    , m_isConnected(false)
{
    createDriver();
}

void DriverAdapter::createDriver()
{
	const auto pluginPath = findUniqueDriverPlugin();
	if (!pluginPath)
	{
		return;
	}

	qInfo() << "DriverAdapter: loading plugin"
	        << QString::fromStdString(pluginPath->filename().string());

	std::unique_ptr<radio::IDriverBuilder> builder;
	try
	{
		if (!m_pluginLibrary.is_loaded())
		{
			m_pluginLibrary.load(*pluginPath,
				boost::dll::load_mode::append_decorations);
		}

		using CreateBuilderFn = radio::IDriverBuilder* (*)();
		CreateBuilderFn create_builder =
			m_pluginLibrary.get<CreateBuilderFn>("dllCreateBuilder");

		builder.reset(create_builder());
	}
	catch (const boost::system::system_error& ex)
	{
        qCritical() << ex.what();
	}

	if (!builder)
	{
		qCritical() << "DriverAdapter: failed to create driver builder";
		return;
	}

    m_driver = builder->setDelay(1).enableAck(false).enableShutdown(true).get();

	// Убираем отключение драйвера для имитатора, чтобы он не отключался через случайные интервалы
    m_driver->enableShutdown(false);

    qDebug() << "The driver object has been constructed";

	connectToDriver();
}

void DriverAdapter::startListening()
{
	if (m_driver)
	{
		m_driver->start();
		qInfo() << "DriverAdapter: Started listening";
	}
}

void DriverAdapter::stopListening()
{
    if (m_driver)
    {
        m_driver->stop();
        qInfo() << "DriverAdapter: Stopped listening";
    }
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

    createTreeParameters(data);
}

void DriverAdapter::onDriverStateChanged(radio::IDriver::State state)
{
    if (state == radio::IDriver::State::kConnected)
    {
		qInfo() << "DriverAdapter: Driver connected";

        //emit driverConnected();

    }
    else
    {
		qInfo() << "DriverAdapter: Driver disconnected";

        //emit driverDisconnected();
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
    // Группированный формат: объект с ключами групп (motion, flags, ...)
    auto message = m_treeJsonParser->toBoardJson(snapshot);

    m_driver->write(message);
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

DriverAdapter::~DriverAdapter()
{
    disconnectFromDriver();
}
