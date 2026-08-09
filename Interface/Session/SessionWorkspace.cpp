#include "SessionWorkspace.h"
#include "../Telemetry/TelemetryDataView.h"
#include "../Charts/ChartsPanel.h"
#include "../Player/PlaybackView.h"
#include "../../Mvc/PlayerTemplate.h"
#include "../../Mvc/PlayerDocument.h"
#include "../../Mvc/PlayerView.h"
#include "../../ViewModel/DataPlayer.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QItemSelectionModel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QTimer>
#include <QResizeEvent>
#include <QIcon>
#include "../../Model/Parameters/Tree/ParameterTreeStorage.h"
#include "../../ViewModel/DriverDataPlayer.h"

SessionWorkspace::SessionWorkspace(Session* session, QWidget *parent) : QFrame(parent)
{
	m_session = session;

	m_showChartButton = new QToolButton(this);
	m_showChartButton->setIcon(QIcon(":/Resources/icons8-chart-32.png"));
	m_showChartButton->setIconSize(QSize(32, 32));
	m_showChartButton->setToolTip(tr("Show parameter"));
	m_showChartButton->setAutoRaise(true);
	connect(m_showChartButton, &QToolButton::clicked, this, &SessionWorkspace::onShowChartButtonClicked);

	m_hideChartButton = new QToolButton(this);
	m_hideChartButton->setIcon(QIcon(":/Resources/icons8-hide-32.png"));
	m_hideChartButton->setIconSize(QSize(32, 32));
	m_hideChartButton->setToolTip(tr("Hide parameter"));
	m_hideChartButton->setAutoRaise(true);
	connect(m_hideChartButton, &QToolButton::clicked, this, &SessionWorkspace::onHideChartButtonClicked);

	QHBoxLayout* chartButtonsLayout = new QHBoxLayout();
	chartButtonsLayout->setContentsMargins(0, 0, 0, 0);
	chartButtonsLayout->setSpacing(2);
	chartButtonsLayout->addWidget(m_showChartButton, 0, Qt::AlignLeft);
	chartButtonsLayout->addWidget(m_hideChartButton, 0, Qt::AlignLeft);
	chartButtonsLayout->addStretch(1);

	m_parametersTree = new TelemetryDataView(this);

	QVBoxLayout* parametersLayout = new QVBoxLayout();
	parametersLayout->setContentsMargins(0, 0, 0, 0);
	parametersLayout->setSpacing(5);
	parametersLayout->addLayout(chartButtonsLayout);
	parametersLayout->addWidget(m_parametersTree, 1);

	QWidget* parametersContainer = new QWidget(this);
	parametersContainer->setLayout(parametersLayout);

	m_chartsPanel = new ChartsPanel(this);
	m_playerView = new PlaybackView(this);

	m_playerTemplate = new PlayerTemplate(this);
	m_playerTemplate->create(m_playerView);
	if (auto* strip = m_playerTemplate->playerView())
	{
		m_playerView->setTimelineStrip(strip);
	}

	auto* sessionTabs = new QTabWidget(this);
	sessionTabs->addTab(m_chartsPanel, tr("Графики"));
	sessionTabs->addTab(new QWidget(this), tr("Траектория"));
	sessionTabs->setCurrentIndex(0);

	QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
	splitter->addWidget(parametersContainer);
	splitter->addWidget(sessionTabs);
	splitter->setStretchFactor(0, 1);
	splitter->setStretchFactor(1, 8);
	splitter->setChildrenCollapsible(false);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);
	mainLayout->addWidget(splitter, 1);
	mainLayout->addWidget(m_playerView);

	m_layoutSettleTimer = new QTimer(this);
	m_layoutSettleTimer->setSingleShot(true);
	m_layoutSettleTimer->setInterval(120);
	connect(m_layoutSettleTimer, &QTimer::timeout, this, [this]()
	{
		setChartsInteractionPaused(false);
	});

	connect(splitter, &QSplitter::splitterMoved, this, [this]()
	{
		pauseChartsUntilLayoutSettles();
	});

	attachModels(m_session);

	m_parametersTree->setExpandsOnDoubleClick(false);
	connect(m_parametersTree, &QTreeView::doubleClicked,
		this, &SessionWorkspace::onParameterDoubleClicked);
	connect(m_parametersTree, &TelemetryDataView::itemHovered, m_chartsPanel, &ChartsPanel::onParameterItemHovered);
}

void SessionWorkspace::attachModels(Session* session)
{
	auto model = session->parametersModel();
	m_parametersTree->setModel(model);

	auto selectionModel = session->parametersSelectionModel();
	if (selectionModel)
	{
		m_parametersTree->setSelectionModel(selectionModel);
	}

	m_chartsPanel->setModel(session->chartsModel());

	if (session->player())
	{
		m_playerView->setPlayer(session->player());
		m_chartsPanel->setPlayer(session->player());

		connect(session->player(), &DataPlayer::elapsedTimeChanged,
			this, &SessionWorkspace::syncPlayerTimeline);
		connect(session->player(), &DataPlayer::sessionDurationChanged,
			this, &SessionWorkspace::syncPlayerTimeline);
		connect(session->player(), &DataPlayer::currentPositionChanged,
			this, &SessionWorkspace::syncPlayerTimeline);
		connect(session->player(), &DataPlayer::isPlayingChanged,
			this, &SessionWorkspace::syncPlayerTimeline);

		if (auto* doc = m_playerTemplate->playerDocument())
		{
			connect(doc, &PlayerDocument::cursorSeeked, this, [this](double seconds)
			{
				m_frozenPlayerCursorSeconds = seconds;
				m_playerCursorFrozen = true;
				if (m_session && m_session->player())
				{
					m_session->player()->setElapsedTime(seconds);
				}
			});
		}

		m_playerCursorFrozen = false;
		syncPlayerTimeline();
	}
}

void SessionWorkspace::syncPlayerTimeline()
{
	if (!m_session || !m_playerTemplate)
	{
		return;
	}

	auto* doc = m_playerTemplate->playerDocument();
	auto* player = m_session->player();
	if (!doc || !player)
	{
		return;
	}

	const double duration = player->sessionDuration();
	const double elapsed = player->elapsedTime();
	doc->setLiveMode(qobject_cast<DriverDataPlayer*>(player) != nullptr);
	doc->setPlaying(player->isPlaying());

	// Запись: данные есть на всём диапазоне — playhead в конце шкалы → вся полоса синяя
	if (player->isPlayable())
	{
		m_playerCursorFrozen = false;
		doc->setTimeline(duration, elapsed, duration);
		return;
	}

	if (player->isPlaying())
	{
		m_playerCursorFrozen = false;
		m_frozenPlayerCursorSeconds = elapsed;
		doc->setTimeline(duration, elapsed, elapsed);
		return;
	}

	// Пауза live: кружок остаётся на месте (или куда его перетащили), playhead идёт с данными
	if (!m_playerCursorFrozen)
	{
		m_frozenPlayerCursorSeconds = elapsed;
		m_playerCursorFrozen = true;
	}
	doc->setTimeline(duration, m_frozenPlayerCursorSeconds, elapsed);
}

void SessionWorkspace::onShowChartButtonClicked()
{
	if (m_session)
	{
		m_session->showChartFromSelectedParameter();
	}
}

void SessionWorkspace::onHideChartButtonClicked()
{
	if (m_session)
	{
		m_session->hideChartFromSelectedParameter();
	}
}

void SessionWorkspace::onParameterDoubleClicked(const QModelIndex& index)
{
	if (m_session)
	{
		m_session->toggleChartAtIndex(index);
	}
}

void SessionWorkspace::resizeEvent(QResizeEvent* event)
{
	QFrame::resizeEvent(event);
	pauseChartsUntilLayoutSettles();
}

void SessionWorkspace::pauseChartsUntilLayoutSettles()
{
	if (!m_layoutSettleTimer)
	{
		return;
	}

	if (!m_chartsInteractionPaused)
	{
		setChartsInteractionPaused(true);
	}

	m_layoutSettleTimer->start();
}

void SessionWorkspace::setChartsInteractionPaused(bool paused)
{
	if (m_chartsInteractionPaused == paused)
	{
		return;
	}

	m_chartsInteractionPaused = paused;

	if (m_chartsPanel)
	{
		m_chartsPanel->setInteractionPaused(paused);
	}

	if (!m_session)
	{
		return;
	}

	if (m_session->getType() != Session::LiveSession)
	{
		return;
	}

	if (auto* driverPlayer = qobject_cast<DriverDataPlayer*>(m_session->player()))
	{
		driverPlayer->setRefreshPaused(paused);
	}
}
