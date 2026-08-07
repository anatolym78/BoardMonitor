#ifndef SESSIONFRAME_H
#define SESSIONFRAME_H

#include <QFrame>
#include <QPointer>

#include "../../ViewModel/Session.h"

class TelemetryDataView;
class ChartsPanel;
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
    ChartsPanel* chartsPanel() const { return m_chartsPanel; }

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onShowChartButtonClicked();
    void onHideChartButtonClicked();

private:
    void setChartsInteractionPaused(bool paused);
    void pauseChartsUntilLayoutSettles();

    QPointer<Session> m_session; 
    TelemetryDataView* m_parametersTree;
    ChartsPanel* m_chartsPanel;
    PlaybackView* m_playerView;
    QToolButton* m_showChartButton;
    QToolButton* m_hideChartButton;
    /** Отложенное возобновление отрисовки: общий для перетаскивания сплиттера и изменения размеров окна. */
    QTimer* m_layoutSettleTimer = nullptr;
    bool m_chartsInteractionPaused = false;
};

#endif // SESSIONFRAME_H
