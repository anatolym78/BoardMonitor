#ifndef SESSIONFRAME_H
#define SESSIONFRAME_H

#include <QFrame>
#include <QPointer>

#include "../../ViewModel/Session.h"

class TelemetryDataView;
class ChartsPanel;
class PlaybackView;
class PlayerTemplate;
class QToolButton;

/**
 * @brief Основное рабочее пространство для одной сессии.
 *
 * Агрегирует в себе все инструменты для работы с конкретной сессией:
 * - Дерево телеметрии (TelemetryDataView)
 * - Панель графиков и траектории (QTabWidget)
 * - Управление воспроизведением (PlaybackView)
 * - MVP-шкала плеера (PlayerTemplate) под стандартным плеером
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
    void syncPlayerTimeline();

private:
    void setChartsInteractionPaused(bool paused);
    void pauseChartsUntilLayoutSettles();

    QPointer<Session> m_session;
    TelemetryDataView* m_parametersTree;
    ChartsPanel* m_chartsPanel;
    PlaybackView* m_playerView;
    PlayerTemplate* m_playerTemplate = nullptr;
    QToolButton* m_showChartButton;
    QToolButton* m_hideChartButton;
    QTimer* m_layoutSettleTimer = nullptr;
    bool m_chartsInteractionPaused = false;
    /** Курсор MVP-шкалы, замороженный на паузе live (playhead при этом может идти дальше). */
    double m_frozenPlayerCursorSeconds = 0.0;
    bool m_playerCursorFrozen = false;
};

#endif // SESSIONFRAME_H