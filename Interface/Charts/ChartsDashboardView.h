#ifndef CHARTSPANEL_H
#define CHARTSPANEL_H

#include <QFrame>
#include <QList>
#include <QMap>
#include "../../ViewModel/ChatViewGridModel.h"
#include "ParametersChartView.h"
#include "../../Model/Parameters/Tree/ParameterTreeHistoryItem.h"

class QScrollArea;
class QGridLayout;
class QWidget;
class QToolButton;

/**
 * @brief Панель управления графиками (Dashboard).
 * 
 * Содержит сетку (Grid) из отдельных графиков (ParametersChartView).
 * Управляет расположением графиков (1 или 2 колонки), добавлением и удалением серий данных.
 */
class ChartsDashboardView : public QFrame
{
	Q_OBJECT
public:
	explicit ChartsDashboardView(QWidget *parent = nullptr);
	void setModel(ChatViewGridModel* chartsModel);

	void onParameterItemHovered(ParameterTreeHistoryItem* treeItem);

protected:
	void onAddChart(int chartIndex, ParameterTreeItem* parameter);
	void onParameterRemoved(int chartIndex, const QString& label);
	void onParameterMoved(int chartIndex, const QStringList& labels);
	bool eventFilter(QObject* watched, QEvent* event) override;

	ParametersChartView* getChartView(int chartIndex);

private:
	void updateCellSizes();
	void relayoutChartsGrid();

	void onToggleColumnClicked();
	void onMergeChartsClicked();

	ChatViewGridModel* m_chartsModel;
	QScrollArea* m_scrollArea;
	QWidget* m_scrollContent;
	QGridLayout* m_gridLayout;
	QToolButton* m_toggleColumnButton;
	int m_columnCount = 1;
	QMap<QtCharts::QAbstractSeries*, QColor>  m_seriesColors;

private:
	void addSeriesToChart(int chartIndex, QtCharts::QChart* chart, ParameterTreeItem* parameter);
	QList<ParametersChartView*> chartViewList() const;
	void hoverSeries(QtCharts::QAbstractSeries* series);
	void restoreSeriesColor(QtCharts::QAbstractSeries* series);
};

#endif // CHARTSPANEL_H
