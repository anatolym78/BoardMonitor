#ifndef SESSIONFRAME_H
#define SESSIONFRAME_H

#include <QFrame>
#include <QPointer>

#include "../../ViewModel/Session.h"

class TelemetryDataView;
class ChartsDashboardView;
class PlaybackView;
class QToolButton;

/**
 * @brief Основное рабочее пространство для одной сессии.
 * 
 * Агрегирует в себе все инструменты для работы с конкретной сессией:
 * - Дерево телеметрии (TelemetryDataView)
 * - Панель графиков и траектории (QTabWidget)
 * - Управление воспроизведением (PlaybackView)
 */
class SessionWorkspace : public QFrame
{
    Q_OBJECT
public:
    explicit SessionWorkspace(Session* session, QWidget *parent = nullptr);

    void attachModels(Session* session);

    TelemetryDataView* parametersTree() const { return m_parametersTree; }
    PlaybackView* playerView() const { return m_playerView; }

private slots:
    void onShowChartButtonClicked();
    void onHideChartButtonClicked();

private:
    QPointer<Session> m_session; 
    TelemetryDataView* m_parametersTree;
    ChartsDashboardView* m_chartsPanel;
    PlaybackView* m_playerView;
    QToolButton* m_showChartButton;
    QToolButton* m_hideChartButton;
};

#endif // SESSIONFRAME_H
