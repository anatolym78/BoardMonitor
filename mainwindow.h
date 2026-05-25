#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMdiSubWindow>
#include <QMap>
#include <QStackedWidget>
#include <QChartView>
#include "Interface/Session/SessionStackView.h"
#include "Interface/Session/SessionListView.h"

class BoardStationApp;
class ChartBuilder;
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
};

#endif // MAINWINDOW_H
