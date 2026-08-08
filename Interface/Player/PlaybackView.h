#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <QWidget>
#include <QDateTime>

#include "../../Model/AppSettings.h"

class QToolButton;
class QSlider;
class QLabel;
class QHBoxLayout;
class DataPlayer;

/**
 * @brief Виджет управления воспроизведением.
 *
 * Кнопки + MVP-шкала времени (вместо QSlider) + информационная метка.
 */
class PlaybackView : public QWidget
{
	Q_OBJECT
public:
	explicit PlaybackView(QWidget *parent = nullptr);

	void onPlayButtonToggled();

	void setPlayer(DataPlayer* player);

	/** Вставить кастомную шкалу (PlayerView) на место скрытого QSlider. */
	void setTimelineStrip(QWidget* strip);

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
	QHBoxLayout* m_mainLayout = nullptr;
	QToolButton* m_playPauseButton;
	QToolButton* m_stopButton;
	QSlider* m_positionSlider;
	QWidget* m_timelineStrip = nullptr;
	QLabel* m_infoLabel;
	DataPlayer* m_player = nullptr;
	AppSettings::PlayerScrubMode m_scrubMode = AppSettings::PlayerScrubMode::DiscreteSecond;
	AppSettings::PlayerTimeDisplayMode m_timeDisplayMode = AppSettings::PlayerTimeDisplayMode::Local;
};

#endif // PLAYERVIEW_H
