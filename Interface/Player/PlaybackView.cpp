#include "PlaybackView.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QSlider>
#include <QLabel>
#include <QTime>
#include "../../ViewModel/DataPlayer.h"

PlaybackView::PlaybackView(QWidget *parent)
	: QWidget(parent)
{
	m_playPauseButton = new QToolButton(this);
	m_playPauseButton->setIcon(QIcon(":/Resources/icons8-pause-yellow-32.png"));
	m_playPauseButton->setIconSize(QSize(32, 32));
	m_playPauseButton->setToolTip(tr("Pause"));
	m_playPauseButton->setAutoRaise(true);
	m_playPauseButton->setCheckable(true);

	m_stopButton = new QToolButton(this);
	m_stopButton->setIcon(QIcon(":/Resources/icons8-stop-red-32.png"));
	m_stopButton->setIconSize(QSize(32, 32));
	m_stopButton->setToolTip(tr("Stop"));
	m_stopButton->setAutoRaise(true);


	m_positionSlider = new QSlider(Qt::Horizontal, this);
	m_positionSlider->setMinimum(0);
	m_positionSlider->setMaximum(100);
	m_positionSlider->setSingleStep(1);
	m_positionSlider->setPageStep(1);

	m_infoLabel = new QLabel(this);
	auto font = QFont();
	font.setPointSize(11);
	m_infoLabel->setFont(font);
	m_infoLabel->setText("00:00 / 00:00");

	QHBoxLayout* mainLayout = new QHBoxLayout(this);
	mainLayout->addWidget(m_playPauseButton);
	mainLayout->addWidget(m_stopButton);
	mainLayout->addWidget(m_positionSlider);
	mainLayout->addWidget(m_infoLabel);
	mainLayout->setContentsMargins(5, 5, 5, 5);
	mainLayout->setSpacing(5);

	connect(m_playPauseButton, &QToolButton::toggled, this, &PlaybackView::onPlayButtonToggled);
	connect(m_stopButton, &QToolButton::clicked, this, [this]()
	{
		if (m_player) m_player->stop();
	});

	connect(m_positionSlider, &QSlider::sliderMoved, this, [this](int value)
	{
		const double elapsedSeconds = sliderToSeconds(value);
		if (m_player)
		{
			m_player->setElapsedTime(elapsedSeconds);
		}
		updateInfoLabel(elapsedSeconds, m_player ? m_player->sessionDuration() : 0.0);
	});
}

void PlaybackView::onPlayButtonToggled()
{
	if (m_playPauseButton->isChecked())
	{
		m_playPauseButton->setIcon(QIcon(":/Resources/icons8-pause-32.png"));
		m_playPauseButton->setToolTip(tr("Pause"));
	}
	else
	{
		m_playPauseButton->setIcon(QIcon(":/Resources/icons8-play-32.png"));
		m_playPauseButton->setToolTip(tr("Play"));
	}
}

void PlaybackView::setScrubMode(AppSettings::PlayerScrubMode mode)
{
	if (m_scrubMode == mode)
	{
		return;
	}

	m_scrubMode = mode;
	updateSliderRange();
}

void PlaybackView::setTimeDisplayMode(AppSettings::PlayerTimeDisplayMode mode)
{
	if (m_timeDisplayMode == mode)
	{
		return;
	}

	m_timeDisplayMode = mode;

	const double elapsed = m_player ? m_player->elapsedTime() : 0.0;
	const double duration = m_player ? m_player->sessionDuration() : 0.0;
	updateInfoLabel(elapsed, duration);
}

void PlaybackView::updateSliderRange()
{
	const double duration = m_player ? m_player->sessionDuration() : 0.0;
	const double elapsed = m_player ? m_player->elapsedTime() : 0.0;

	if (m_scrubMode == AppSettings::PlayerScrubMode::Continuous)
	{
		// 1 единица слайдера = 1 мс: движение мыши даёт почти непрерывную перемотку
		m_positionSlider->setMaximum(qMax(0, secondsToSlider(duration)));
		m_positionSlider->setSingleStep(1);
		m_positionSlider->setPageStep(1000);
	}
	else
	{
		// Историческое поведение: диапазон в целых секундах
		m_positionSlider->setMaximum(qMax(0, secondsToSlider(duration)));
		m_positionSlider->setSingleStep(1);
		m_positionSlider->setPageStep(1);
	}

	if (!m_positionSlider->isSliderDown())
	{
		m_positionSlider->setValue(secondsToSlider(elapsed));
	}

	updateInfoLabel(elapsed, duration);
}

int PlaybackView::secondsToSlider(double seconds) const
{
	if (m_scrubMode == AppSettings::PlayerScrubMode::Continuous)
	{
		return static_cast<int>(seconds * 1000.0 + (seconds >= 0.0 ? 0.5 : -0.5));
	}

	return static_cast<int>(seconds);
}

double PlaybackView::sliderToSeconds(int value) const
{
	if (m_scrubMode == AppSettings::PlayerScrubMode::Continuous)
	{
		return static_cast<double>(value) / 1000.0;
	}

	return static_cast<double>(value);
}

QString PlaybackView::formatDuration(double seconds) const
{
	// Длительность и конец записи — без долей секунды, иначе метка перегружена
	return QTime(0, 0).addSecs(static_cast<int>(seconds)).toString("mm:ss");
}

QString PlaybackView::formatAbsoluteTime(const QDateTime& time) const
{
	if (!time.isValid())
	{
		return QStringLiteral("00:00:00");
	}

	return time.toString("hh:mm:ss");
}

QString PlaybackView::formatPosition(double elapsedSeconds) const
{
	const bool showFraction = (m_scrubMode == AppSettings::PlayerScrubMode::Continuous);

	if (m_timeDisplayMode == AppSettings::PlayerTimeDisplayMode::Real
		&& m_player
		&& m_player->sessionStartTime().isValid())
	{
		const QDateTime absolute = m_player->sessionStartTime().addMSecs(
			static_cast<qint64>(elapsedSeconds * 1000.0));

		return showFraction
			? absolute.toString("hh:mm:ss.zzz")
			: absolute.toString("hh:mm:ss");
	}

	if (showFraction)
	{
		return QTime(0, 0).addMSecs(static_cast<int>(elapsedSeconds * 1000.0)).toString("mm:ss.zzz");
	}

	return formatDuration(elapsedSeconds);
}

void PlaybackView::setPlayer(DataPlayer* player)
{
	if (m_player == player) return;
	m_player = player;
	if (!m_player) return;

	if (!m_player->isPlayable())
	{
		m_playPauseButton->setVisible(false);
		m_stopButton->setVisible(false);
	}
	else
	{
		m_playPauseButton->setVisible(true);
		m_stopButton->setVisible(true);
	}

	// Начальная инициализация состояния
	updateSliderRange();
	m_playPauseButton->setChecked(m_player->isPlaying());
	onPlayButtonToggled();

	// Управление воспроизведением с кнопки
	connect(m_playPauseButton, &QToolButton::toggled, this, [this](bool checked)
	{
		if (!m_player) return;
		if (checked) m_player->play();
		else m_player->pause();
	});

	// Слушаем изменения от плеера
	connect(m_player, &DataPlayer::isPlayingChanged, this, [this]()
	{
		if (!m_player) return;
		const bool playing = m_player->isPlaying();
		if (m_playPauseButton->isChecked() != playing)
		{
			m_playPauseButton->setChecked(playing);
		}
		onPlayButtonToggled();
	});

	connect(m_player, &DataPlayer::sessionDurationChanged, this, [this]()
	{
		if (!m_player) return;
		updateSliderRange();
	});

	connect(m_player, &DataPlayer::sessionStartTimeChanged, this, [this]()
	{
		if (!m_player) return;
		updateInfoLabel(m_player->elapsedTime(), m_player->sessionDuration());
	});

	connect(m_player, &DataPlayer::sessionEndTimeChanged, this, [this]()
	{
		if (!m_player) return;
		updateInfoLabel(m_player->elapsedTime(), m_player->sessionDuration());
	});

	connect(m_player, &DataPlayer::elapsedTimeChanged, this, [this]()
	{
		if (!m_player) return;
		if (!m_positionSlider->isSliderDown())
		{
			m_positionSlider->setValue(secondsToSlider(m_player->elapsedTime()));
		}
		updateInfoLabel(m_player->elapsedTime(), m_player->sessionDuration());
	});
}

void PlaybackView::refreshFromPlayer()
{
	if (!m_player) return;
	updateSliderRange();
}

void PlaybackView::updateInfoLabel(double elapsedSeconds, double durationSeconds)
{
	if (m_timeDisplayMode == AppSettings::PlayerTimeDisplayMode::Real
		&& m_player
		&& m_player->sessionEndTime().isValid())
	{
		// Текущее / конечное реальное время, длительность записи в скобках
		m_infoLabel->setText(QString("%1 / %2 (%3)")
			.arg(formatPosition(elapsedSeconds))
			.arg(formatAbsoluteTime(m_player->sessionEndTime()))
			.arg(formatDuration(durationSeconds)));
		return;
	}

	m_infoLabel->setText(QString("%1 / %2")
		.arg(formatPosition(elapsedSeconds))
		.arg(formatDuration(durationSeconds)));
}
