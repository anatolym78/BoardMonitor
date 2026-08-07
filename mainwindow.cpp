#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "./BoardStationApp.h"
#include "./Interface/Telemetry/TelemetryDataView.h"
#include "./Interface/Uplink/UplinkEditorView.h"
#include "./Interface/Tools/UplinkEditorDelegates.h"
#include "./Interface/Session/SessionWorkspace.h"
#include "./Interface/Charts/ChartsPanel.h"
#include "./Interface/Player/PlaybackView.h"
#include "./ViewModel/SessionsListModel.h"
#include "./ViewModel/DebugViewModel.h"
#include "./ViewModel/RecordedSession.h"

#include <QDebug>
#include <QTimer>
#include <QCheckBox>
#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QTextEdit>
#include <QMessageBox>
#include <QIcon>
#include <QToolButton>
#include <QMenu>
#include <QMenuBar>
#include <QActionGroup>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, m_app(nullptr)
{
	ui->setupUi(this);

	ui->buttonSaveRecord->setIcon(QIcon(":/Resources/icons8-save-32.png"));
	ui->buttonSaveRecord->setIconSize(QSize(32, 32));

	ui->buttonDeleteRecord->setIcon(QIcon(":/Resources/icons8-delete-32.png"));
	ui->buttonDeleteRecord->setIconSize(QSize(32, 32));

	ui->buttonSendToBoard->setIcon(QIcon(":/Resources/icons8-send-32.png"));
	ui->buttonSendToBoard->setIconSize(QSize(32, 32));

	connect(ui->buttonSaveRecord, &QToolButton::clicked, this, &MainWindow::saveRecord);
	connect(ui->buttonDeleteRecord, &QToolButton::clicked, this, &MainWindow::deleteRecord);
	connect(ui->buttonSendToBoard, &QToolButton::clicked, this, &MainWindow::sendMessageToBoard);
	connect(ui->checkboxSendDataImmediately, &QCheckBox::stateChanged, this, &MainWindow::onCheckBoxSetDataImmediately);

	ui->action->setCheckable(true);
	ui->action->setChecked(ui->consoleDockWidget->isVisible());
	connect(ui->action, &QAction::toggled, ui->consoleDockWidget, &QWidget::setVisible);
	connect(ui->consoleDockWidget, &QDockWidget::visibilityChanged, ui->action, &QAction::setChecked);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::setApp(BoardStationApp *pApp)
{
	if (!pApp) return;

	m_app = pApp;
	setupPluginsMenu();

	auto sessions = pApp->sessionsModel();

	// Лямбда: подключает сигнал dataLoadCompleted конкретной RecordedSession к явному
	// обновлению соответствующего workspace. Нужно вызывать для каждой новой сессии.
	// Постоянное соединение: срабатывает при каждой новой async-загрузке этой сессии.
	auto connectDataLoad = [this, sessions](Session* session)
	{
		auto* rs = qobject_cast<RecordedSession*>(session);
		if (!rs) return;
		connect(rs, &RecordedSession::dataLoadCompleted, this, [this, sessions, rs]()
		{
			// Ищем текущий индекс этой сессии (после сортировки он мог сдвинуться)
			for (int j = 0; j < sessions->sessionCount(); ++j)
			{
				if (sessions->session(j) == rs)
				{
					auto* ws = sessionsStackView()->getSessionFrame(j);
					if (ws)
					{
						ws->parametersTree()->applyDefaultExpansion();
						ws->playerView()->refreshFromPlayer();
					}
					break;
				}
			}
		});
	};

	// Подключаем добавление/удаление сессий для стека workspace (плоский индекс)
	connect(sessions, &SessionsListModel::sessionInsertedAt,
		this, [this, connectDataLoad](int flatIndex, Session* session)
		{
			auto sessionFrame = new SessionWorkspace(session, this);
			applyPlayerSettingsToWorkspace(sessionFrame);
			sessionsStackView()->insertWidget(flatIndex, sessionFrame);
			connectDataLoad(session);
			sessionsListView()->revealFlatSessionIndex(flatIndex);
		});

	connect(sessions, &SessionsListModel::sessionRemovedAt,
		this, [this](int flatIndex)
		{
			QWidget* widget = sessionsStackView()->widget(flatIndex);
			if (widget)
			{
				sessionsStackView()->removeWidget(widget);
				widget->deleteLater();
			}
		});

	connect(sessions, &SessionsListModel::sessionAboutToBeRemoved,
		this, [this](int flatIndex)
		{
			if (!app() || !app()->sessionsModel())
			{
				return;
			}

			if (app()->sessionsModel()->sessionCount() <= 1)
			{
				sessionsListView()->clearSessionSelection();
				return;
			}

			const int newFlatIndex = flatIndex > 0 ? flatIndex - 1 : 0;
			sessionsListView()->selectFlatSessionIndex(newFlatIndex);
		});
		
	// Полное пересоздание стека только при refreshSessions (modelReset)
	connect(sessions, &QAbstractItemModel::modelReset,
		this, [this, sessions, connectDataLoad]()
		{
			// Очищаем все существующие виджеты
			while (sessionsStackView()->count() > 0)
			{
				QWidget* widget = sessionsStackView()->widget(0);
				sessionsStackView()->removeWidget(widget);
				widget->deleteLater();
			}
				
			// Создаем виджеты заново для всех сессий
			for (auto i = 0; i < sessions->sessionCount(); i++)
			{
				auto session = sessions->session(i);
				auto sessionFrame = new SessionWorkspace(session, this);
				applyPlayerSettingsToWorkspace(sessionFrame);
				sessionsStackView()->addWidget(sessionFrame);
				connectDataLoad(session);
			}
			qDebug() << "MainWindow: Recreated all SessionWorkspace widgets after model reset";
		});
		
	// Создаем начальные виджеты для всех сессий
	for (auto i = 0; i < sessions->sessionCount(); i++)
	{
		auto session = sessions->session(i);
		auto sessionFrame = new SessionWorkspace(session, this);
		applyPlayerSettingsToWorkspace(sessionFrame);
		sessionsStackView()->addWidget(sessionFrame);
		connectDataLoad(session);
	}
		
	sessionsListView()->setModel(pApp->sessionsModel());
	sessionsListView()->createSelectionModel();
	connect(sessionsListView(), &SessionListView::sessionSelected, this,
		[this](int index) 
		{
			sessionsStackView()->setCurrentIndex(index);
			app()->sessionsModel()->selectSession(index);

			auto sessionWorkspace = sessionsStackView()->getSessionFrame(index);
			if (!sessionWorkspace)
			{
				return;
			}

			sessionWorkspace->playerView()->refreshFromPlayer();

			if (sessionWorkspace->parametersTree())
			{
				QTimer::singleShot(0, sessionWorkspace->parametersTree(),
					&TelemetryDataView::applyDefaultExpansion);
			}
		});
		
	//sessionsListView()->selectFirstItem();
	auto rows = pApp->sessionsModel()->sessionCount();

	// Устанавливаем модель для uplink параметров
	if (pApp->getUplinkParametersModel())
	{
		ui->uplinkParametersView->setModel(pApp->getUplinkParametersModel());
		ui->uplinkParametersView->expandToDepth(1);
		
		// Настраиваем делегат для отправки данных
		auto delegate = ui->uplinkParametersView->getDelegate();
		if (delegate)
		{
			delegate->setDriverAdapter(pApp->getDriverAdapter());
			delegate->setParametersModel(pApp->getUplinkParametersModel());
		}
		
		// Настраиваем UplinkEditorView для отправки данных
		ui->uplinkParametersView->setDriverAdapter(pApp->getDriverAdapter());
		ui->uplinkParametersView->setParametersModel(pApp->getUplinkParametersModel());
		ui->uplinkParametersView->setSendDataImmediately(ui->checkboxSendDataImmediately->isChecked());
	}
		
	// Устанавливаем модель для консоли
	if (pApp->getDebugViewModel())
	{
		ui->consoleTableView->setModel(pApp->getDebugViewModel());
	}

	// Выделяем первый элемент списка (живую сессию) после полной инициализации UI
	QTimer::singleShot(0, this, &MainWindow::expandLiveDataTelemetry);
}

BoardStationApp* MainWindow::app() const
{
	return m_app;
}

void MainWindow::saveRecord()
{
	app()->saveLiveData();
}

void MainWindow::deleteRecord()
{
	if (!app()) return;

	QModelIndex index = sessionsListView()->currentIndex();
	if (!index.isValid()) return;

	const int flatIndex = index.data(SessionsListModel::FlatSessionIndexRole).toInt();
	if (flatIndex < 0)
	{
		return;
	}

	auto session = app()->sessionsModel()->getSession(flatIndex);
	if (session == app()->liveSession())
	{
		QMessageBox::warning(this, tr("Warning"), tr("Cannot delete live session"));
		return;
	}

	auto reply = QMessageBox::question(this, tr("Delete Session"),
		tr("Are you sure you want to delete this session?"),
		QMessageBox::Yes | QMessageBox::No);

	if (reply == QMessageBox::Yes)
	{
		app()->removeRecordFromDatabase(flatIndex);
	}
}

void MainWindow::sendMessageToBoard()
{
	//expandLiveDataTelemetry();

	//return;

	app()->sendParametersToBoard();
}

void MainWindow::expandLiveDataTelemetry()
{
	sessionsListView()->selectFirstItem();
}

SessionStackView *MainWindow::sessionsStackView() const
{
	return ui->sessionsStackView;
}

SessionListView* MainWindow::sessionsListView() const
{
	return ui->listView;
}

void MainWindow::onCheckBoxSetDataImmediately(int state)
{
	ui->uplinkParametersView->setSendDataImmediately(state > 0);
}

void MainWindow::setupPluginsMenu()
{
	if (!m_app)
	{
		return;
	}

	auto* adapter = m_app->getDriverAdapter();
	if (!adapter)
	{
		return;
	}

	auto* settingsMenu = menuBar()->addMenu(tr("Settings"));
	auto* pluginsMenu = settingsMenu->addMenu(tr("Plugins"));

	m_pluginsActionGroup = new QActionGroup(this);
	m_pluginsActionGroup->setExclusive(true);

	const QStringList plugins = adapter->availablePlugins();
	if (plugins.isEmpty())
	{
		auto* emptyAction = pluginsMenu->addAction(tr("(no plugins in settings.json)"));
		emptyAction->setEnabled(false);
	}
	else
	{
		for (const QString& pluginName : plugins)
		{
			auto* action = pluginsMenu->addAction(pluginName);
			action->setCheckable(true);
			action->setData(pluginName);
			action->setChecked(pluginName == adapter->currentPlugin());
			m_pluginsActionGroup->addAction(action);

			connect(action, &QAction::triggered, this, [this, pluginName](bool checked)
			{
				if (!checked || !m_app)
				{
					return;
				}

				auto* driverAdapter = m_app->getDriverAdapter();
				if (!driverAdapter)
				{
					return;
				}

				if (pluginName == driverAdapter->currentPlugin())
				{
					return;
				}

				if (!driverAdapter->switchPlugin(pluginName))
				{
					QMessageBox::warning(
						this,
						tr("Plugins"),
						tr("Failed to load plugin \"%1\".").arg(pluginName));
					syncPluginsMenuSelection(driverAdapter->currentPlugin());
				}
			});
		}
	}

	connect(adapter, &DriverAdapter::currentPluginChanged,
		this, &MainWindow::syncPluginsMenuSelection);

	setupPlayerSettingsMenu(settingsMenu);
	setupChartsSettingsMenu(settingsMenu);
	applyPlayerSettingsToAllViews();
	applyChartsSettingsToAllViews();
}

void MainWindow::setupPlayerSettingsMenu(QMenu* settingsMenu)
{
	if (!settingsMenu || !m_app)
	{
		return;
	}

	auto& settings = m_app->settings();
	auto* playerMenu = settingsMenu->addMenu(tr("Player"));

	auto* scrubMenu = playerMenu->addMenu(tr("Scrub step"));
	m_playerScrubActionGroup = new QActionGroup(this);
	m_playerScrubActionGroup->setExclusive(true);

	auto addScrubAction = [this, scrubMenu, &settings](const QString& text, AppSettings::PlayerScrubMode mode)
	{
		auto* action = scrubMenu->addAction(text);
		action->setCheckable(true);
		action->setData(static_cast<int>(mode));
		action->setChecked(settings.playerScrubMode() == mode);
		m_playerScrubActionGroup->addAction(action);

		connect(action, &QAction::triggered, this, [this, mode](bool checked)
		{
			if (!checked || !m_app)
			{
				return;
			}

			auto& appSettings = m_app->settings();
			if (appSettings.playerScrubMode() == mode)
			{
				return;
			}

			appSettings.setPlayerScrubMode(mode);
			if (!appSettings.save())
			{
				QMessageBox::warning(this, tr("Player"), tr("Failed to save settings.json."));
			}

			applyPlayerSettingsToAllViews();
		});
	};

	addScrubAction(tr("Discrete (1 s)"), AppSettings::PlayerScrubMode::DiscreteSecond);
	addScrubAction(tr("Continuous"), AppSettings::PlayerScrubMode::Continuous);

	auto* timeMenu = playerMenu->addMenu(tr("Time display"));
	m_playerTimeDisplayActionGroup = new QActionGroup(this);
	m_playerTimeDisplayActionGroup->setExclusive(true);

	auto addTimeAction = [this, timeMenu, &settings](const QString& text, AppSettings::PlayerTimeDisplayMode mode)
	{
		auto* action = timeMenu->addAction(text);
		action->setCheckable(true);
		action->setData(static_cast<int>(mode));
		action->setChecked(settings.playerTimeDisplayMode() == mode);
		m_playerTimeDisplayActionGroup->addAction(action);

		connect(action, &QAction::triggered, this, [this, mode](bool checked)
		{
			if (!checked || !m_app)
			{
				return;
			}

			auto& appSettings = m_app->settings();
			if (appSettings.playerTimeDisplayMode() == mode)
			{
				return;
			}

			appSettings.setPlayerTimeDisplayMode(mode);
			if (!appSettings.save())
			{
				QMessageBox::warning(this, tr("Player"), tr("Failed to save settings.json."));
			}

			applyPlayerSettingsToAllViews();
		});
	};

	addTimeAction(tr("Local"), AppSettings::PlayerTimeDisplayMode::Local);
	addTimeAction(tr("Real"), AppSettings::PlayerTimeDisplayMode::Real);
}

void MainWindow::setupChartsSettingsMenu(QMenu* settingsMenu)
{
	if (!settingsMenu || !m_app)
	{
		return;
	}

	auto& settings = m_app->settings();
	auto* chartsMenu = settingsMenu->addMenu(tr("Charts"));

	m_chartsShowTimeCursorAction = chartsMenu->addAction(tr("Show time cursor"));
	m_chartsShowTimeCursorAction->setCheckable(true);
	m_chartsShowTimeCursorAction->setChecked(settings.chartsShowTimeCursor());
	m_chartsShowTimeCursorAction->setToolTip(
		tr("Vertical time mark on recorded charts. Always hidden for live sessions."));

	connect(m_chartsShowTimeCursorAction, &QAction::toggled, this, [this](bool checked)
	{
		if (!m_app)
		{
			return;
		}

		auto& appSettings = m_app->settings();
		if (appSettings.chartsShowTimeCursor() == checked)
		{
			return;
		}

		appSettings.setChartsShowTimeCursor(checked);
		if (!appSettings.save())
		{
			QMessageBox::warning(this, tr("Charts"), tr("Failed to save settings.json."));
		}

		applyChartsSettingsToAllViews();
	});

	m_chartsValueAxisExpandOnlyAction = chartsMenu->addAction(tr("Y-axis: expand only"));
	m_chartsValueAxisExpandOnlyAction->setCheckable(true);
	m_chartsValueAxisExpandOnlyAction->setChecked(settings.chartsValueAxisExpandOnly());
	m_chartsValueAxisExpandOnlyAction->setToolTip(
		tr("Value axis range can grow to fit data, but never shrink."));

	connect(m_chartsValueAxisExpandOnlyAction, &QAction::toggled, this, [this](bool checked)
	{
		if (!m_app)
		{
			return;
		}

		auto& appSettings = m_app->settings();
		if (appSettings.chartsValueAxisExpandOnly() == checked)
		{
			return;
		}

		appSettings.setChartsValueAxisExpandOnly(checked);
		if (!appSettings.save())
		{
			QMessageBox::warning(this, tr("Charts"), tr("Failed to save settings.json."));
		}

		applyChartsSettingsToAllViews();
	});
}

void MainWindow::syncPlayerScrubMenuSelection(AppSettings::PlayerScrubMode mode)
{
	if (!m_playerScrubActionGroup)
	{
		return;
	}

	const QList<QAction*> actions = m_playerScrubActionGroup->actions();
	for (QAction* action : actions)
	{
		action->setChecked(static_cast<AppSettings::PlayerScrubMode>(action->data().toInt()) == mode);
	}
}

void MainWindow::syncPlayerTimeDisplayMenuSelection(AppSettings::PlayerTimeDisplayMode mode)
{
	if (!m_playerTimeDisplayActionGroup)
	{
		return;
	}

	const QList<QAction*> actions = m_playerTimeDisplayActionGroup->actions();
	for (QAction* action : actions)
	{
		action->setChecked(static_cast<AppSettings::PlayerTimeDisplayMode>(action->data().toInt()) == mode);
	}
}

void MainWindow::applyPlayerSettingsToWorkspace(SessionWorkspace* workspace)
{
	if (!workspace || !m_app)
	{
		return;
	}

	auto* playerView = workspace->playerView();
	playerView->setScrubMode(m_app->settings().playerScrubMode());
	playerView->setTimeDisplayMode(m_app->settings().playerTimeDisplayMode());
	applyChartsSettingsToWorkspace(workspace);
}

void MainWindow::applyChartsSettingsToWorkspace(SessionWorkspace* workspace)
{
	if (!workspace || !m_app)
	{
		return;
	}

	auto* chartsPanel = workspace->chartsPanel();
	if (!chartsPanel)
	{
		return;
	}

	const auto& settings = m_app->settings();
	chartsPanel->setShowTimeCursor(settings.chartsShowTimeCursor());
	chartsPanel->setValueAxisExpandOnly(settings.chartsValueAxisExpandOnly());
}

void MainWindow::applyPlayerSettingsToAllViews()
{
	if (!m_app)
	{
		return;
	}

	const auto& settings = m_app->settings();
	syncPlayerScrubMenuSelection(settings.playerScrubMode());
	syncPlayerTimeDisplayMenuSelection(settings.playerTimeDisplayMode());

	auto* stack = sessionsStackView();
	if (!stack)
	{
		return;
	}

	for (int i = 0; i < stack->count(); ++i)
	{
		if (auto* workspace = stack->getSessionFrame(i))
		{
			applyPlayerSettingsToWorkspace(workspace);
		}
	}
}

void MainWindow::applyChartsSettingsToAllViews()
{
	if (!m_app)
	{
		return;
	}

	const auto& settings = m_app->settings();
	if (m_chartsShowTimeCursorAction)
	{
		m_chartsShowTimeCursorAction->setChecked(settings.chartsShowTimeCursor());
	}
	if (m_chartsValueAxisExpandOnlyAction)
	{
		m_chartsValueAxisExpandOnlyAction->setChecked(settings.chartsValueAxisExpandOnly());
	}

	auto* stack = sessionsStackView();
	if (!stack)
	{
		return;
	}

	for (int i = 0; i < stack->count(); ++i)
	{
		if (auto* workspace = stack->getSessionFrame(i))
		{
			applyChartsSettingsToWorkspace(workspace);
		}
	}
}

void MainWindow::syncPluginsMenuSelection(const QString& pluginName)
{
	if (!m_pluginsActionGroup)
	{
		return;
	}

	const QList<QAction*> actions = m_pluginsActionGroup->actions();
	for (QAction* action : actions)
	{
		action->setChecked(action->data().toString() == pluginName);
	}
}



