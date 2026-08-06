#pragma once

#include <QString>
#include <QStringList>

class AppSettings
{
public:
	enum class PlayerScrubMode
	{
		DiscreteSecond, // шаг слайдера = 1 с
		Continuous      // шаг слайдера = 1 мс (по сути «по пикселю»)
	};

	enum class PlayerTimeDisplayMode
	{
		Local, // время от начала сессии
		Real   // настенные часы момента записи
	};

	static QString settingsFilePath();

	bool load();
	bool save() const;

	QStringList plugins() const { return m_plugins; }
	QString currentPlugin() const { return m_currentPlugin; }
	void setCurrentPlugin(const QString& pluginName);

	bool hasPlugin(const QString& pluginName) const;

	PlayerScrubMode playerScrubMode() const { return m_playerScrubMode; }
	void setPlayerScrubMode(PlayerScrubMode mode);

	PlayerTimeDisplayMode playerTimeDisplayMode() const { return m_playerTimeDisplayMode; }
	void setPlayerTimeDisplayMode(PlayerTimeDisplayMode mode);

	static QString playerScrubModeToString(PlayerScrubMode mode);
	static PlayerScrubMode playerScrubModeFromString(const QString& value);

	static QString playerTimeDisplayModeToString(PlayerTimeDisplayMode mode);
	static PlayerTimeDisplayMode playerTimeDisplayModeFromString(const QString& value);

private:
	QStringList m_plugins;
	QString m_currentPlugin;
	PlayerScrubMode m_playerScrubMode = PlayerScrubMode::DiscreteSecond;
	PlayerTimeDisplayMode m_playerTimeDisplayMode = PlayerTimeDisplayMode::Local;
};
