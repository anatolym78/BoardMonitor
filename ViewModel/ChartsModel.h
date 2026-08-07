#ifndef CHARTSMODEL_H
#define CHARTSMODEL_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>

#include "./../Model/Parameters/Tree/ParameterTreeItem.h"
#include "./../Model/Parameters/Tree/ParameterTreeHistoryItem.h"

/**
 * @brief Логическая модель набора графиков одной сессии.
 *
 * Хранит только структуру: слоты графиков, имена серий, selection.
 * Не знает про QCustomPlot и не пишет точки — это делает ChartsPanel (presenter/view).
 *
 * Добавление:
 *  Session → showParameter() → chartAdded → ChartsPanel создаёт ChartView и серии.
 */
class ChartsModel : public QObject
{
	Q_OBJECT

public:
	struct ChartSlot
	{
		QString rootLabel;
		QStringList seriesLabels;
		bool isSelected = false;
	};

	explicit ChartsModel(QObject* parent = nullptr);

	void showParameter(ParameterTreeItem* parameter);
	void hideParameter(ParameterTreeItem* parameter);
	void toggleParameter(ParameterTreeItem* parameter);
	bool isParameterDisplayed(ParameterTreeItem* parameter) const;
	bool hasSeries(const QString& label) const;

	int chartCount() const { return m_charts.count(); }
	QStringList seriesLabels(int chartIndex) const;
	int findChartIndex(const QString& label) const;

	bool selectChart(int index, bool keepSelection);
	void clearSelection();
	QList<int> selectedIndices() const;
	bool canMergeCharts() const;
	void mergeSelectedCharts();

signals:
	void chartAdded(int chartIndex, ParameterTreeItem* parameter);
	void seriesRemoved(int chartIndex, const QString& label);
	void chartRemoved(int chartIndex);
	void seriesMoved(int targetChartIndex, const QStringList& labels);
	void selectionChanged();

private:
	void collectHistoryItems(ParameterTreeItem* item, QList<ParameterTreeHistoryItem*>& out) const;
	void removeSeries(const QString& label);
	void removeEmptyCharts();

	QList<ChartSlot> m_charts;
};

#endif // CHARTSMODEL_H
