#ifndef CHARTSPANEL_H
#define CHARTSPANEL_H

#include <QFrame>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QDateTime>

#include "qcustomplot.h"

#include "../../ViewModel/ChartsModel.h"
#include "ChartView.h"
#include "../../Model/Parameters/Tree/ParameterTreeHistoryItem.h"
#include "../../Model/Parameters/Tree/ParameterTreeItem.h"

class QScrollArea;
class QGridLayout;
class QWidget;
class QToolButton;
class DataPlayer;
class ParameterTreeStorage;

/**
 * @brief Панель графиков сессии: presenter + view для ChartsModel.
 *
 * Создаёт ChartView/QCPGraph по сигналам модели, пишет точки из DataPlayer,
 * управляет раскладкой сетки и merge. Модель при этом остаётся без QCustomPlot.
 */
class ChartsPanel : public QFrame
{
	Q_OBJECT
public:
	struct ChartRuntime
	{
		ChartView* view = nullptr;
		QMap<QString, QPointer<QCPGraph>> seriesMap;
		QPointer<QCPAxis> timeAxis;
		QPointer<QCPAxis> valueAxis;
		QPointer<QCPItemRect> timeCursor;
		bool isAxesInitialized = false;
		bool windowInitialized = false;
		double windowBeginKey = 0.0;
		double lastAxisEndKey = 0.0;
		qint64 lastAxisUpdateMs = 0;
	};

	explicit ChartsPanel(QWidget* parent = nullptr);

	void setModel(ChartsModel* chartsModel);
	void setPlayer(DataPlayer* player);
	void setInteractionPaused(bool paused);

	void onParameterItemHovered(ParameterTreeHistoryItem* treeItem);

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void onChartAdded(int chartIndex, ParameterTreeItem* parameter);
	void onSeriesRemoved(int chartIndex, const QString& label);
	void onChartRemoved(int chartIndex);
	void onSeriesMoved(int targetChartIndex, const QStringList& labels);
	void onSelectionChanged();
	void onPlayed(ParameterTreeStorage* snapshot, bool isBackPlaying);

	void addSeriesToChart(int chartIndex, ChartView* chartView, ParameterTreeItem* parameter);
	void addHistoryDescendantsToChart(int chartIndex, ChartView* chartView, ParameterTreeItem* item);
	void fillSeriesFromStorage(int chartIndex, const QString& label);

	void updateSeries(int chartIndex, const QString& label, ParameterTreeHistoryItem* data, bool isBackPlaying);
	void syncSeriesFromHistory(int chartIndex, const QString& label, ParameterTreeHistoryItem* data);
	void filterDataOutsideRange(
		const QList<QDateTime>& times,
		const QList<QVariant>& values,
		const QDateTime& seriesStartTime,
		const QDateTime& seriesEndTime,
		bool isBackPlaying,
		QList<QDateTime>& outTimes,
		QList<QVariant>& outValues) const;

	void createTimeCursor(ChartRuntime& chart);
	void updateTimeCursor(ChartRuntime& chart, double cursorKey);
	void updateTimeWindow(ChartRuntime& chart, double cursorKey, bool throttle);
	void trimSeriesOutsideWindow(ChartRuntime& chart);
	void updateValueAxisFromVisible(ChartRuntime& chart);
	double currentCursorKey() const;

	void updateCellSizes();
	void relayoutChartsGrid();
	void reindexChartViews();
	void onToggleColumnClicked();
	void onMergeChartsClicked();

	ChartView* getChartView(int chartIndex) const;
	QList<ChartView*> chartViewList() const;
	void hoverSeries(QCPAbstractPlottable* series);
	void restoreSeriesColor(QCPAbstractPlottable* series);
	bool isLivePlayer() const;
	double visibleSpanSeconds() const { return 15.0; }

	ChartsModel* m_chartsModel = nullptr;
	DataPlayer* m_player = nullptr;
	QMetaObject::Connection m_playConnection;

	QScrollArea* m_scrollArea = nullptr;
	QWidget* m_scrollContent = nullptr;
	QGridLayout* m_gridLayout = nullptr;
	QToolButton* m_toggleColumnButton = nullptr;
	int m_columnCount = 1;

	QList<ChartRuntime> m_charts;
	QMap<QCPAbstractPlottable*, QColor> m_seriesColors;
	bool m_interactionPaused = false;
};

#endif // CHARTSPANEL_H
