#pragma once

#include <QString>
#include <QStringList>

class AppSettings
{
public:
	static QString settingsFilePath();

	bool load();
	bool save() const;

	QStringList plugins() const { return m_plugins; }
	QString currentPlugin() const { return m_currentPlugin; }
	void setCurrentPlugin(const QString& pluginName);

	bool hasPlugin(const QString& pluginName) const;

private:
	QStringList m_plugins;
	QString m_currentPlugin;
};
