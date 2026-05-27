#include "SessionWorkspace.h"
#include "../Telemetry/TelemetryDataView.h"
#include "../Charts/ChartsDashboardView.h"
#include "../Player/PlaybackView.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QItemSelectionModel>
#include <QPushButton>
#include "../../Model/Parameters/Tree/ParameterTreeStorage.h"

SessionWorkspace::SessionWorkspace(Session* session, QWidget *parent) : QFrame(parent)
{
	m_session = session;

	m_addChartButton = new QPushButton(tr("Toggle chart"), this);
	m_addChartButton->setIcon(QIcon(":/Resources/toggle_parameter_16.png"));
	connect(m_addChartButton, &QPushButton::clicked, this, &SessionWorkspace::onAddChartButtonClicked);

	m_parametersTree = new TelemetryDataView(this);

	QVBoxLayout* parametersLayout = new QVBoxLayout();
	parametersLayout->setContentsMargins(0, 0, 0, 0);
	parametersLayout->setSpacing(5);
	parametersLayout->addWidget(m_addChartButton);
	parametersLayout->addWidget(m_parametersTree, 1);

	QWidget* parametersContainer = new QWidget(this);
	parametersContainer->setLayout(parametersLayout);

	m_chartsPanel = new ChartsDashboardView(this);
	m_playerView = new PlaybackView(this);

	QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
	splitter->addWidget(parametersContainer);
	splitter->addWidget(m_chartsPanel);
	splitter->setStretchFactor(0, 1);
	splitter->setStretchFactor(1, 8);
	splitter->setChildrenCollapsible(false);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);
	mainLayout->addWidget(splitter, 1);
	mainLayout->addWidget(m_playerView);

	attachModels(m_session);

	connect(m_parametersTree, &TelemetryDataView::itemHovered, m_chartsPanel, &ChartsDashboardView::onParameterItemHovered);

}

void SessionWorkspace::attachModels(Session* session)
{
	auto model = session->parametersModel();
	m_parametersTree->setModel(model);
	
	// Устанавливаем selectionModel из Session в представление
	auto selectionModel = session->parametersSelectionModel();
	if (selectionModel)
	{
		m_parametersTree->setSelectionModel(selectionModel);
	}

	m_chartsPanel->setModel(session->chartsModel());

	// Подключаем плеер к PlayerView
	if (session->player())
	{
		m_playerView->setPlayer(session->player());
	}
}

void SessionWorkspace::onAddChartButtonClicked()
{
	if (m_session)
	{
		m_session->createChartFromSelectedParameter();
	}
}
