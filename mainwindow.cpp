#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "./BoardStationApp.h"
#include "./Interface/Telemetry/TelemetryDataView.h"
#include "./Interface/Uplink/UplinkEditorView.h"
#include "./Interface/Tools/UplinkEditorDelegates.h"
#include "./Interface/Session/SessionWorkspace.h"
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
#include <QChartView>
#include <QIcon>
#include <QToolButton>

QT_CHARTS_USE_NAMESPACE

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
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::setApp(BoardStationApp *pApp)
{
	if (!pApp) return;

	m_app = pApp;

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
			for (int j = 0; j < sessions->rowCount(); ++j)
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

	// Подключаем сигнал удаления сессии для удаления соответствующего виджета
	connect(sessions, &QAbstractItemModel::rowsAboutToBeRemoved,
		this, [this](const QModelIndex& parent, int first, int last)
		{
			// Удаляем виджеты сессий, которые будут удалены из модели
			for (int i = first; i <= last; ++i)
			{
				QWidget* widget = sessionsStackView()->widget(i);
				if (widget)
				{
					sessionsStackView()->removeWidget(widget);
					widget->deleteLater();
				}
			}
		});
		
	// Подключаем сигнал добавления сессии для создания соответствующего виджета
	connect(sessions, &QAbstractItemModel::rowsInserted,
		this, [this, sessions, connectDataLoad](const QModelIndex& parent, int first, int last)
		{
			// Создаем виджеты для новых сессий
			for (int i = first; i <= last; ++i)
			{
				auto session = sessions->getSession(i);
				if (session)
				{
					auto sessionFrame = new SessionWorkspace(session, this);
					sessionsStackView()->insertWidget(i, sessionFrame);
					connectDataLoad(session);
					qDebug() << "MainWindow: Created SessionWorkspace for new session at index" << i;
				}
			}
		});
		
	// Подключаем сигнал modelReset для пересоздания всех виджетов (при сортировке и т.д.)
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
			for (auto i = 0; i < sessions->rowCount(); i++)
			{
				auto session = sessions->session(i);
				auto sessionFrame = new SessionWorkspace(session, this);
				sessionsStackView()->addWidget(sessionFrame);
				connectDataLoad(session);
			}
			qDebug() << "MainWindow: Recreated all SessionWorkspace widgets after model reset";
		});
		
	// Создаем начальные виджеты для всех сессий
	for (auto i = 0; i < sessions->rowCount(); i++)
	{
		auto session = sessions->session(i);
		auto sessionFrame = new SessionWorkspace(session, this);
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
			if (sessionWorkspace)
			{
				sessionWorkspace->playerView()->refreshFromPlayer();

				auto parametersTreeView = sessionWorkspace->parametersTree();

				auto session = app()->sessionsModel()->session(index);
				auto rowCount = session->parametersModel()->rowCount();
				if (parametersTreeView)
				{
					parametersTreeView->applyDefaultExpansion();
				}
			}
		});
		
	//sessionsListView()->selectFirstItem();
	auto rows = pApp->sessionsModel()->rowCount();

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

	// Проверяем, что это не живая сессия, перед вопросом
	auto session = app()->sessionsModel()->getSession(index.row());
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
		int deletedRow = index.row();
		app()->removeRecordFromDatabase(deletedRow);

		// После удаления явно выбираем соседний элемент, т.к. Qt не гарантирует
		// автоматическую генерацию selectionChanged с непустым selected
		int rowCount = app()->sessionsModel()->rowCount();
		if (rowCount > 0)
		{
			int newRow = qMin(deletedRow, rowCount - 1);
			QModelIndex newIndex = app()->sessionsModel()->index(newRow, 0);
			sessionsListView()->selectionModel()->select(newIndex, QItemSelectionModel::ClearAndSelect);
			sessionsListView()->selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::NoUpdate);
		}
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



