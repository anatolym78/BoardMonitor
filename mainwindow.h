#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMdiSubWindow>
#include <QMap>
#include <QStackedWidget>
#include "Interface/Session/SessionStackView.h"
#include "Interface/Session/SessionListView.h"
#include "Model/AppSettings.h"

class BoardStationApp;
class ChartBuilder;
class QActionGroup;
class QMenu;
class SessionWorkspace;
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Методы для работы с приложением
    void setApp(BoardStationApp *app);
    BoardStationApp* app() const;
    
    void saveRecord();
    void deleteRecord();
    void sendMessageToBoard();

    void expandLiveDataTelemetry();

private:
    Ui::MainWindow *ui;
    BoardStationApp *m_app;

private:
    SessionStackView* sessionsStackView() const;
    SessionListView* sessionsListView() const;

private:
    void onCheckBoxSetDataImmediately(int state);
    void setupPluginsMenu();
    void setupPlayerSettingsMenu(QMenu* settingsMenu);
    void setupChartsSettingsMenu(QMenu* settingsMenu);
    void syncPluginsMenuSelection(const QString& pluginName);
    void syncPlayerScrubMenuSelection(AppSettings::PlayerScrubMode mode);
    void syncPlayerTimeDisplayMenuSelection(AppSettings::PlayerTimeDisplayMode mode);
    void applyPlayerSettingsToAllViews();
    void applyPlayerSettingsToWorkspace(SessionWorkspace* workspace);
    void applyChartsSettingsToAllViews();
    void applyChartsSettingsToWorkspace(SessionWorkspace* workspace);

private:
    QActionGroup* m_pluginsActionGroup = nullptr;
    QActionGroup* m_playerScrubActionGroup = nullptr;
    QActionGroup* m_playerTimeDisplayActionGroup = nullptr;
    QAction* m_chartsShowTimeCursorAction = nullptr;
    QAction* m_chartsValueAxisExpandOnlyAction = nullptr;
};

#endif // MAINWINDOW_H
