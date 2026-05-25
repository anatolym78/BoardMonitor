#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <QWidget>

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

private:
	void updateInfoLabel(double elapsedSeconds, double durationSeconds);

private:
	QToolButton* m_playPauseButton;
	QToolButton* m_stopButton;
	QSlider* m_positionSlider;
	QLabel* m_infoLabel;
	DataPlayer* m_player = nullptr;
};

#endif // PLAYERVIEW_H
