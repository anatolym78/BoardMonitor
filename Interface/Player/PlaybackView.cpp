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
		if (m_player)
		{
			m_player->setElapsedTime(static_cast<double>(value));
		}
		updateInfoLabel(value, m_player ? m_player->sessionDuration() : 0.0);
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

void PlaybackView::setPlayer(DataPlayer* player)
{
	if (m_player == player) return;
	m_player = player;
	if (!m_player) return;

	if (!m_player->isPlayable())
	{
		m_stopButton->setVisible(false);
	}

	// Начальная инициализация состояния
	m_positionSlider->setMaximum(static_cast<int>(m_player->sessionDuration()));
	m_positionSlider->setValue(static_cast<int>(m_player->elapsedTime()));
	m_playPauseButton->setChecked(m_player->isPlaying());
	onPlayButtonToggled();
	updateInfoLabel(m_player->elapsedTime(), m_player->sessionDuration());

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
		m_positionSlider->setMaximum(static_cast<int>(m_player->sessionDuration()));
		updateInfoLabel(m_player->elapsedTime(), m_player->sessionDuration());
	});

	connect(m_player, &DataPlayer::elapsedTimeChanged, this, [this]()
	{
		if (!m_player) return;
		const int elapsed = static_cast<int>(m_player->elapsedTime());
		if (!m_positionSlider->isSliderDown())
		{
			m_positionSlider->setValue(elapsed);
		}
		updateInfoLabel(m_player->elapsedTime(), m_player->sessionDuration());
	});
}

void PlaybackView::refreshFromPlayer()
{
	if (!m_player) return;
	m_positionSlider->setMaximum(static_cast<int>(m_player->sessionDuration()));
	m_positionSlider->setValue(static_cast<int>(m_player->elapsedTime()));
	updateInfoLabel(m_player->elapsedTime(), m_player->sessionDuration());
}

void PlaybackView::updateInfoLabel(double elapsedSeconds, double durationSeconds)
{
	const QTime elapsed = QTime(0,0).addSecs(static_cast<int>(elapsedSeconds));
	const QTime duration = QTime(0,0).addSecs(static_cast<int>(durationSeconds));
	m_infoLabel->setText(QString("%1 / %2").arg(elapsed.toString("mm:ss")).arg(duration.toString("mm:ss")));
}
