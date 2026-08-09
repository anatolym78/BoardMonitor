#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QSet>
#include <QPointer>

#include "./../Model/Parameters/Tree/ParameterTreeItem.h"
#include "./../Model/Parameters/Tree/ParameterTreeHistoryItem.h"

class ParameterTreeStorage;

/**
 * @brief Логическая модель набора графиков одной сессии.
 *
 * Хранит только структуру: слоты графиков, имена серий, selection, title.
 * Не знает про QCustomPlot и не пишет точки — это делает ChartsPanel.
 */
class ChartsModel : public QObject
{
	Q_OBJECT

public:
	struct ChartSlot
	{
		QString rootLabel;
		QString title;
		QStringList seriesLabels;
		bool isSelected = false;
	};

	explicit ChartsModel(QObject* parent = nullptr);

	void setParameterTree(ParameterTreeStorage* tree);

	void showParameter(ParameterTreeItem* parameter);
	void hideParameter(ParameterTreeItem* parameter);
	void toggleParameter(ParameterTreeItem* parameter);
	bool isParameterDisplayed(ParameterTreeItem* parameter) const;
	bool hasSeries(const QString& label) const;

	int chartCount() const { return m_charts.count(); }
	QStringList seriesLabels(int chartIndex) const;
	QString chartTitle(int chartIndex) const;
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
	void chartTitleChanged(int chartIndex, const QString& title);
	void selectionChanged();

private:
	void collectHistoryItems(ParameterTreeItem* item, QList<ParameterTreeHistoryItem*>& out) const;
	void removeSeries(const QString& label);
	void removeEmptyCharts();
	void refreshChartTitle(int chartIndex);
	QString buildCompressedTitle(const QStringList& seriesLabels) const;
	void compressNode(ParameterTreeItem* node, QSet<QString>& remaining, QStringList& parts) const;
	QString formatLeafDisplayName(ParameterTreeItem* item) const;
	QString formatLeafDisplayName(const QString& fullName) const;

	QList<ChartSlot> m_charts;
	QPointer<ParameterTreeStorage> m_tree;
};
