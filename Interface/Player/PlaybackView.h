#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <QWidget>
#include <QDateTime>

#include "../../Model/AppSettings.h"

class QToolButton;
class QSlider;
class QLabel;
class DataPlayer;

/**
 * @brief Виджет управления воспроизведением.
 * 
 * Предоставляет элементы управления плеером (Play, Pause, Stop, Slider)
 * для навигации по записанным данным сессии.
 */
class PlaybackView : public QWidget
{
	Q_OBJECT
public:
	explicit PlaybackView(QWidget *parent = nullptr);

	void onPlayButtonToggled();

	// Подключение к модели плеера
	void setPlayer(DataPlayer* player);

	// Принудительное обновление отображения из текущего состояния плеера
	// (используется после async-загрузки, когда сигналы могли не дойти)
	void refreshFromPlayer();

	void setScrubMode(AppSettings::PlayerScrubMode mode);
	AppSettings::PlayerScrubMode scrubMode() const { return m_scrubMode; }

	void setTimeDisplayMode(AppSettings::PlayerTimeDisplayMode mode);
	AppSettings::PlayerTimeDisplayMode timeDisplayMode() const { return m_timeDisplayMode; }

private:
	void updateInfoLabel(double elapsedSeconds, double durationSeconds);
	void updateSliderRange();
	int secondsToSlider(double seconds) const;
	double sliderToSeconds(int value) const;
	QString formatDuration(double seconds) const;
	QString formatAbsoluteTime(const QDateTime& time) const;
	QString formatPosition(double elapsedSeconds) const;

private:
	QToolButton* m_playPauseButton;
	QToolButton* m_stopButton;
	QSlider* m_positionSlider;
	QLabel* m_infoLabel;
	DataPlayer* m_player = nullptr;
	AppSettings::PlayerScrubMode m_scrubMode = AppSettings::PlayerScrubMode::DiscreteSecond;
	AppSettings::PlayerTimeDisplayMode m_timeDisplayMode = AppSettings::PlayerTimeDisplayMode::Local;
};

#endif // PLAYERVIEW_H
