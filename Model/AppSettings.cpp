#include "AppSettings.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

QString AppSettings::settingsFilePath()
{
	return QApplication::applicationDirPath() + QStringLiteral("/settings.json");
}

QString AppSettings::playerScrubModeToString(PlayerScrubMode mode)
{
	switch (mode)
	{
	case PlayerScrubMode::Continuous:
		return QStringLiteral("continuous");
	case PlayerScrubMode::DiscreteSecond:
	default:
		return QStringLiteral("discrete");
	}
}

AppSettings::PlayerScrubMode AppSettings::playerScrubModeFromString(const QString& value)
{
	if (value.compare(QStringLiteral("continuous"), Qt::CaseInsensitive) == 0)
	{
		return PlayerScrubMode::Continuous;
	}

	return PlayerScrubMode::DiscreteSecond;
}

QString AppSettings::playerTimeDisplayModeToString(PlayerTimeDisplayMode mode)
{
	switch (mode)
	{
	case PlayerTimeDisplayMode::Real:
		return QStringLiteral("real");
	case PlayerTimeDisplayMode::Local:
	default:
		return QStringLiteral("local");
	}
}

AppSettings::PlayerTimeDisplayMode AppSettings::playerTimeDisplayModeFromString(const QString& value)
{
	if (value.compare(QStringLiteral("real"), Qt::CaseInsensitive) == 0)
	{
		return PlayerTimeDisplayMode::Real;
	}

	return PlayerTimeDisplayMode::Local;
}

bool AppSettings::load()
{
	const QString path = settingsFilePath();
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly))
	{
		qWarning() << "AppSettings: failed to open" << path;
		return false;
	}

	const QByteArray data = file.readAll();
	file.close();

	QJsonParseError error;
	const QJsonDocument doc = QJsonDocument::fromJson(data, &error);
	if (error.error != QJsonParseError::NoError)
	{
		qWarning() << "AppSettings: JSON parse error:" << error.errorString();
		return false;
	}

	if (!doc.isObject())
	{
		qWarning() << "AppSettings: root must be a JSON object";
		return false;
	}

	const QJsonObject root = doc.object();
	m_plugins.clear();

	const QJsonArray pluginsArray = root.value(QStringLiteral("plugins")).toArray();
	for (const QJsonValue& value : pluginsArray)
	{
		const QString name = value.toString().trimmed();
		if (!name.isEmpty() && !m_plugins.contains(name))
		{
			m_plugins.append(name);
		}
	}

	m_currentPlugin = root.value(QStringLiteral("currentPlugin")).toString().trimmed();
	if (m_currentPlugin.isEmpty() && !m_plugins.isEmpty())
	{
		m_currentPlugin = m_plugins.first();
	}

	if (!m_currentPlugin.isEmpty() && !m_plugins.contains(m_currentPlugin))
	{
		qWarning() << "AppSettings: currentPlugin" << m_currentPlugin
		            << "is not listed in plugins; keeping it as selected";
		m_plugins.prepend(m_currentPlugin);
	}

	const QJsonObject player = root.value(QStringLiteral("player")).toObject();
	m_playerScrubMode = playerScrubModeFromString(
		player.value(QStringLiteral("scrubMode")).toString());
	m_playerTimeDisplayMode = playerTimeDisplayModeFromString(
		player.value(QStringLiteral("timeDisplay")).toString());

	const QJsonObject charts = root.value(QStringLiteral("charts")).toObject();
	m_chartsShowTimeCursor = charts.value(QStringLiteral("showTimeCursor")).toBool(true);
	m_chartsValueAxisExpandOnly = charts.value(QStringLiteral("valueAxisExpandOnly")).toBool(true);

	qInfo() << "AppSettings: loaded" << m_plugins.size() << "plugins, current =" << m_currentPlugin
	        << ", player.scrubMode =" << playerScrubModeToString(m_playerScrubMode)
	        << ", player.timeDisplay =" << playerTimeDisplayModeToString(m_playerTimeDisplayMode)
	        << ", charts.showTimeCursor =" << m_chartsShowTimeCursor
	        << ", charts.valueAxisExpandOnly =" << m_chartsValueAxisExpandOnly;
	return !m_plugins.isEmpty() && !m_currentPlugin.isEmpty();
}

bool AppSettings::save() const
{
	QJsonObject root;
	QJsonArray pluginsArray;
	for (const QString& plugin : m_plugins)
	{
		pluginsArray.append(plugin);
	}
	root.insert(QStringLiteral("plugins"), pluginsArray);
	root.insert(QStringLiteral("currentPlugin"), m_currentPlugin);

	QJsonObject player;
	player.insert(QStringLiteral("scrubMode"), playerScrubModeToString(m_playerScrubMode));
	player.insert(QStringLiteral("timeDisplay"), playerTimeDisplayModeToString(m_playerTimeDisplayMode));
	root.insert(QStringLiteral("player"), player);

	QJsonObject charts;
	charts.insert(QStringLiteral("showTimeCursor"), m_chartsShowTimeCursor);
	charts.insert(QStringLiteral("valueAxisExpandOnly"), m_chartsValueAxisExpandOnly);
	root.insert(QStringLiteral("charts"), charts);

	const QString path = settingsFilePath();
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		qWarning() << "AppSettings: failed to write" << path;
		return false;
	}

	file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
	file.close();
	qInfo() << "AppSettings: saved currentPlugin =" << m_currentPlugin
	        << ", player.scrubMode =" << playerScrubModeToString(m_playerScrubMode)
	        << ", player.timeDisplay =" << playerTimeDisplayModeToString(m_playerTimeDisplayMode)
	        << ", charts.showTimeCursor =" << m_chartsShowTimeCursor
	        << ", charts.valueAxisExpandOnly =" << m_chartsValueAxisExpandOnly;
	return true;
}

void AppSettings::setCurrentPlugin(const QString& pluginName)
{
	m_currentPlugin = pluginName;
}

void AppSettings::setPlayerScrubMode(PlayerScrubMode mode)
{
	m_playerScrubMode = mode;
}

void AppSettings::setPlayerTimeDisplayMode(PlayerTimeDisplayMode mode)
{
	m_playerTimeDisplayMode = mode;
}

void AppSettings::setChartsShowTimeCursor(bool enabled)
{
	m_chartsShowTimeCursor = enabled;
}

void AppSettings::setChartsValueAxisExpandOnly(bool enabled)
{
	m_chartsValueAxisExpandOnly = enabled;
}

bool AppSettings::hasPlugin(const QString& pluginName) const
{
	return m_plugins.contains(pluginName);
}
