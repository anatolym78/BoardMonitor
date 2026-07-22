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

	qInfo() << "AppSettings: loaded" << m_plugins.size() << "plugins, current =" << m_currentPlugin;
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

	const QString path = settingsFilePath();
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		qWarning() << "AppSettings: failed to write" << path;
		return false;
	}

	file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
	file.close();
	qInfo() << "AppSettings: saved currentPlugin =" << m_currentPlugin;
	return true;
}

void AppSettings::setCurrentPlugin(const QString& pluginName)
{
	m_currentPlugin = pluginName;
}

bool AppSettings::hasPlugin(const QString& pluginName) const
{
	return m_plugins.contains(pluginName);
}
