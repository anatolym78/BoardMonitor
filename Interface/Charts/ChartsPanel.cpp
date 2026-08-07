#include "ChartsPanel.h"

#include <QScrollArea>
#include <QGridLayout>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QIcon>
#include <QSet>
#include <algorithm>
#include <limits>

#include "../../ViewModel/DataPlayer.h"
#include "../../ViewModel/DriverDataPlayer.h"
#include "../../ViewModel/SessionPlayer.h"
#include "../../Model/Parameters/Tree/ParameterTreeStorage.h"

namespace
{

double toPlotKey(const QDateTime& time)
{
	return time.toMSecsSinceEpoch() / 1000.0;
}

QDateTime fromPlotKey(double key)
{
	return QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(key * 1000.0));
}

void applyTimeAxisRange(QCPAxis* axis, double beginKey, double endKey)
{
	if (!axis)
	{
		return;
	}

	const double span = qMax(endKey - beginKey, 1.0);
	const double pad = span / 20.0;
	axis->setRange(beginKey - pad, endKey + pad);
}

bool shouldRefreshTimeAxis(const ChartsPanel::ChartRuntime& chart, double endKey)
{
	if (!chart.isAxesInitialized)
	{
		return true;
	}

	const qint64 now = QDateTime::currentMSecsSinceEpoch();
	if (endKey > chart.lastAxisEndKey + 1.0)
	{
		return true;
	}

	return (now - chart.lastAxisUpdateMs) >= 250;
}

void refreshTimeAxisIfNeeded(ChartsPanel::ChartRuntime& chart, double beginKey, double endKey, bool throttle)
{
	if (throttle && !shouldRefreshTimeAxis(chart, endKey))
	{
		return;
	}

	applyTimeAxisRange(chart.timeAxis, beginKey, endKey);
	chart.isAxesInitialized = true;
	chart.lastAxisEndKey = endKey;
	chart.lastAxisUpdateMs = QDateTime::currentMSecsSinceEpoch();
}

bool appendNumericPoints(QCPGraph* graph, const QList<QDateTime>& times, const QList<QVariant>& values)
{
	if (!graph || times.isEmpty() || times.count() != values.count())
	{
		return false;
	}

	double lastKey = graph->data()->isEmpty()
		? std::numeric_limits<double>::lowest()
		: graph->data()->lastKey();

	bool added = false;
	for (int i = 0; i < values.count(); ++i)
	{
		bool ok = false;
		const double y = values[i].toDouble(&ok);
		if (!ok)
		{
			continue;
		}

		const double key = toPlotKey(times[i]);
		if (key <= lastKey)
		{
			continue;
		}

		graph->addData(key, y);
		lastKey = key;
		added = true;
	}

	return added;
}

} // namespace

ChartsPanel::ChartsPanel(QWidget* parent)
	: QFrame(parent)
{
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	QHBoxLayout* topBarLayout = new QHBoxLayout();
	topBarLayout->setContentsMargins(2, 2, 2, 0);

	m_toggleColumnButton = new QToolButton(this);
	m_toggleColumnButton->setIcon(QIcon(":/Resources/icons8-two_column-32.png"));
	m_toggleColumnButton->setIconSize(QSize(32, 32));
	m_toggleColumnButton->setToolTip(tr("Switch to two columns"));
	m_toggleColumnButton->setAutoRaise(true);
	connect(m_toggleColumnButton, &QToolButton::clicked, this, &ChartsPanel::onToggleColumnClicked);

	QToolButton* mergeButton = new QToolButton(this);
	mergeButton->setIcon(QIcon(":/Resources/icons8-merge-documents-32.png"));
	mergeButton->setIconSize(QSize(32, 32));
	mergeButton->setToolTip(tr("Merge selected charts"));
	mergeButton->setAutoRaise(true);
	connect(mergeButton, &QToolButton::clicked, this, &ChartsPanel::onMergeChartsClicked);

	topBarLayout->addWidget(m_toggleColumnButton, 0, Qt::AlignLeft);
	topBarLayout->addSpacing(2);
	topBarLayout->addWidget(mergeButton, 0, Qt::AlignLeft);
	topBarLayout->addStretch(1);
	mainLayout->addLayout(topBarLayout);

	m_scrollArea = new QScrollArea(this);
	m_scrollArea->setWidgetResizable(true);
	m_scrollArea->setFrameShape(QFrame::NoFrame);

	m_scrollContent = new QWidget();
	m_gridLayout = new QGridLayout(m_scrollContent);
	m_gridLayout->setSpacing(2);
	m_gridLayout->setContentsMargins(2, 2, 2, 2);
	m_scrollArea->setWidget(m_scrollContent);
	mainLayout->addWidget(m_scrollArea);
	m_scrollArea->viewport()->installEventFilter(this);
}

void ChartsPanel::setInteractionPaused(bool paused)
{
	if (m_interactionPaused == paused)
	{
		return;
	}
	m_interactionPaused = paused;
	if (!paused)
	{
		updateCellSizes();
	}
}

void ChartsPanel::setModel(ChartsModel* chartsModel)
{
	if (m_chartsModel == chartsModel)
	{
		return;
	}
	if (m_chartsModel)
	{
		disconnect(m_chartsModel, nullptr, this, nullptr);
	}
	m_chartsModel = chartsModel;
	if (!m_chartsModel)
	{
		return;
	}
	connect(m_chartsModel, &ChartsModel::chartAdded, this, &ChartsPanel::onChartAdded);
	connect(m_chartsModel, &ChartsModel::seriesRemoved, this, &ChartsPanel::onSeriesRemoved);
	connect(m_chartsModel, &ChartsModel::chartRemoved, this, &ChartsPanel::onChartRemoved);
	connect(m_chartsModel, &ChartsModel::seriesMoved, this, &ChartsPanel::onSeriesMoved);
	connect(m_chartsModel, &ChartsModel::selectionChanged, this, &ChartsPanel::onSelectionChanged);
}

void ChartsPanel::setPlayer(DataPlayer* player)
{
	if (m_playConnection)
	{
		disconnect(m_playConnection);
	}
	m_player = player;
	if (!m_player)
	{
		return;
	}
	m_playConnection = connect(m_player, &DataPlayer::played, this, &ChartsPanel::onPlayed);
}

bool ChartsPanel::isLivePlayer() const
{
	return m_player && qobject_cast<DriverDataPlayer*>(m_player) != nullptr;
}

void ChartsPanel::onParameterItemHovered(ParameterTreeHistoryItem* treeItem)
{
	for (auto& chart : m_charts)
	{
		if (!chart.view)
		{
			continue;
		}
		for (auto it = chart.seriesMap.begin(); it != chart.seriesMap.end(); ++it)
		{
			QCPGraph* graph = it.value();
			if (!graph)
			{
				continue;
			}
			if (treeItem && treeItem->fullName() == it.key())
			{
				chart.view->setHovered(true);
				hoverSeries(graph);
			}
			else
			{
				chart.view->setHovered(false);
				restoreSeriesColor(graph);
			}
		}
	}
}

void ChartsPanel::restoreSeriesColor(QCPAbstractPlottable* series)
{
	auto* graph = qobject_cast<QCPGraph*>(series);
	if (!graph)
	{
		return;
	}
	auto seriesPen = graph->pen();
	if (m_seriesColors.contains(series))
	{
		seriesPen.setColor(m_seriesColors[series]);
	}
	seriesPen.setStyle(Qt::PenStyle::SolidLine);
	graph->setPen(seriesPen);
	if (graph->parentPlot())
	{
		graph->parentPlot()->replot(QCustomPlot::rpQueued);
	}
}

void ChartsPanel::hoverSeries(QCPAbstractPlottable* series)
{
	auto* graph = qobject_cast<QCPGraph*>(series);
	if (!graph)
	{
		return;
	}
	if (!m_seriesColors.contains(series))
	{
		m_seriesColors.insert(series, graph->pen().color());
	}
	auto seriesPen = graph->pen();
	seriesPen.setStyle(Qt::PenStyle::DashLine);
	graph->setPen(seriesPen);
	if (graph->parentPlot())
	{
		graph->parentPlot()->replot(QCustomPlot::rpQueued);
	}
}

void ChartsPanel::onChartAdded(int chartIndex, ParameterTreeItem* parameter)
{
	if (!parameter || !m_chartsModel)
	{
		return;
	}

	const int row = chartIndex / m_columnCount;
	const int column = chartIndex % m_columnCount;

	auto* chartView = new ChartView(chartIndex, row, column, this);
	chartView->setModel(m_chartsModel);
	m_gridLayout->addWidget(chartView, row, column);
	chartView->legend->setVisible(false);

	connect(chartView, &ChartView::graphHovered, this, [this](QCPGraph* graph, bool state)
	{
		if (state) hoverSeries(graph);
		else restoreSeriesColor(graph);
	});

	auto* timeAxis = chartView->xAxis;
	timeAxis->setTickLabelType(QCPAxis::ltDateTime);
	timeAxis->setDateTimeFormat("hh:mm:ss");
	timeAxis->setDateTimeSpec(Qt::LocalTime);
	timeAxis->setAutoTickCount(3);

	auto* valueAxis = chartView->yAxis;
	valueAxis->setAutoTickCount(3);
	valueAxis->setRange(0, 1);

	ChartRuntime runtime;
	runtime.view = chartView;
	runtime.timeAxis = timeAxis;
	runtime.valueAxis = valueAxis;
	while (m_charts.size() <= chartIndex)
	{
		m_charts.append(ChartRuntime{});
	}
	m_charts[chartIndex] = runtime;

	if (parameter->type() == ParameterTreeItem::ItemType::History)
	{
		addSeriesToChart(chartIndex, chartView, parameter);
	}
	else if (parameter->type() == ParameterTreeItem::ItemType::Array
		|| parameter->type() == ParameterTreeItem::ItemType::Group)
	{
		chartView->plotLayout()->insertRow(0);
		chartView->plotLayout()->addElement(0, 0, new QCPPlotTitle(chartView, parameter->fullName()));
		addHistoryDescendantsToChart(chartIndex, chartView, parameter);
	}

	chartView->replot(QCustomPlot::rpQueued);
	updateCellSizes();
}

void ChartsPanel::addHistoryDescendantsToChart(int chartIndex, ChartView* chartView, ParameterTreeItem* item)
{
	if (!item) return;
	if (item->type() == ParameterTreeItem::ItemType::History)
	{
		addSeriesToChart(chartIndex, chartView, item);
		return;
	}
	for (ParameterTreeItem* child : item->children())
	{
		addHistoryDescendantsToChart(chartIndex, chartView, child);
	}
}

void ChartsPanel::addSeriesToChart(int chartIndex, ChartView* chartView, ParameterTreeItem* parameter)
{
	if (!parameter || chartIndex < 0 || chartIndex >= m_charts.count()) return;
	const QString label = parameter->fullName();
	if (m_charts[chartIndex].seriesMap.contains(label)) return;

	auto* graph = chartView->addGraph(chartView->xAxis, chartView->yAxis);
	QPen seriesPen = graph->pen();
	seriesPen.setCapStyle(Qt::PenCapStyle::RoundCap);
	seriesPen.setColor(parameter->color());
	graph->setPen(seriesPen);
	graph->setName(label);
	m_charts[chartIndex].seriesMap[label] = graph;
	fillSeriesFromStorage(chartIndex, label);
}

void ChartsPanel::fillSeriesFromStorage(int chartIndex, const QString& label)
{
	if (!m_player || !m_player->storage()) return;
	ParameterTreeHistoryItem* history = m_player->storage()->findHistoryItemByFullName(label);
	if (!history) return;
	if (isLivePlayer()) updateSeries(chartIndex, label, history, false);
	else rebuildSeriesFromHistory(chartIndex, label, history);
}

void ChartsPanel::onSeriesRemoved(int chartIndex, const QString& label)
{
	if (chartIndex < 0 || chartIndex >= m_charts.count()) return;
	auto& chart = m_charts[chartIndex];
	if (!chart.view) return;
	if (auto graph = chart.seriesMap.take(label))
	{
		m_seriesColors.remove(graph);
		chart.view->removeGraph(graph);
		chart.view->replot(QCustomPlot::rpQueued);
	}
}

void ChartsPanel::onChartRemoved(int chartIndex)
{
	if (chartIndex < 0 || chartIndex >= m_charts.count()) return;
	ChartView* view = m_charts[chartIndex].view;
	if (view)
	{
		for (auto graph : m_charts[chartIndex].seriesMap)
		{
			if (graph) m_seriesColors.remove(graph);
		}
		m_gridLayout->removeWidget(view);
		view->deleteLater();
	}
	m_charts.removeAt(chartIndex);
	reindexChartViews();
	updateCellSizes();
}

void ChartsPanel::onSeriesMoved(int targetChartIndex, const QStringList& labels)
{
	if (targetChartIndex < 0 || targetChartIndex >= m_charts.count()) return;
	ChartView* targetView = m_charts[targetChartIndex].view;
	if (!targetView) return;

	const QSet<QString> labelsSet(labels.begin(), labels.end());
	for (int sourceIndex = 0; sourceIndex < m_charts.count(); ++sourceIndex)
	{
		if (sourceIndex == targetChartIndex) continue;
		auto& source = m_charts[sourceIndex];
		if (!source.view) continue;

		QList<QString> labelsInSource;
		for (auto it = source.seriesMap.begin(); it != source.seriesMap.end(); ++it)
		{
			if (labelsSet.contains(it.key())) labelsInSource.append(it.key());
		}

		for (const QString& label : labelsInSource)
		{
			QCPGraph* sourceGraph = source.seriesMap.take(label);
			if (!sourceGraph) continue;
			auto* targetGraph = targetView->addGraph(targetView->xAxis, targetView->yAxis);
			targetGraph->setName(label);
			targetGraph->setPen(sourceGraph->pen());
			targetGraph->addData(*sourceGraph->data());
			m_charts[targetChartIndex].seriesMap[label] = targetGraph;
			m_seriesColors.remove(sourceGraph);
			source.view->removeGraph(sourceGraph);
		}
	}
	targetView->setSelected(false);
	targetView->replot(QCustomPlot::rpQueued);
	updateCellSizes();
}

void ChartsPanel::onSelectionChanged()
{
	if (!m_chartsModel) return;
	const QList<int> selected = m_chartsModel->selectedIndices();
	for (int i = 0; i < m_charts.count(); ++i)
	{
		if (m_charts[i].view)
		{
			m_charts[i].view->setSelected(selected.contains(i));
		}
	}
}

void ChartsPanel::onPlayed(ParameterTreeStorage* snapshot, bool isBackPlaying)
{
	if (m_interactionPaused) return;

	const bool livePlayer = isLivePlayer();
	const bool seekUpdate = !livePlayer
		&& m_player
		&& qobject_cast<SessionPlayer*>(m_player)
		&& static_cast<SessionPlayer*>(m_player)->isSeekUpdate();

	ParameterTreeStorage* storage = snapshot;
	if (!storage && livePlayer && m_player) storage = m_player->storage();
	if (!storage) return;

	for (int chartIndex = 0; chartIndex < m_charts.count(); ++chartIndex)
	{
		auto& chart = m_charts[chartIndex];
		for (auto key : chart.seriesMap.keys())
		{
			auto* data = storage->findHistoryItemByFullName(key);
			if (seekUpdate) rebuildSeriesFromHistory(chartIndex, key, data);
			else updateSeries(chartIndex, key, data, isBackPlaying);
		}
		if (chart.view) chart.view->replot(QCustomPlot::rpQueued);
	}
}

void ChartsPanel::rebuildSeriesFromHistory(int chartIndex, const QString& label, ParameterTreeHistoryItem* data)
{
	if (!data || chartIndex < 0 || chartIndex >= m_charts.count()) return;
	auto& chart = m_charts[chartIndex];
	QCPGraph* graph = chart.seriesMap.value(label);
	if (!graph) return;

	graph->clearData();
	appendNumericPoints(graph, data->timestamps(), data->values());
	if (graph->data()->isEmpty()) return;

	const double span = visibleSpanSeconds();
	double endKey = graph->data()->firstKey() + span;
	if (m_player) endKey = qMax(endKey, toPlotKey(m_player->currentPosition()));
	const double beginKey = endKey - span;
	graph->removeDataBefore(beginKey);
	graph->removeDataAfter(endKey);
	refreshTimeAxisIfNeeded(chart, beginKey, endKey, false);

	if (chart.valueAxis && !graph->data()->isEmpty())
	{
		double localMin = std::numeric_limits<double>::max();
		double localMax = std::numeric_limits<double>::lowest();
		for (const QCPData& point : *graph->data())
		{
			localMin = qMin(localMin, point.value);
			localMax = qMax(localMax, point.value);
		}
		if (localMin <= localMax)
		{
			const double valueRange = localMax - localMin;
			if (qFuzzyIsNull(valueRange)) chart.valueAxis->setRange(localMin - 1, localMax + 1);
			else chart.valueAxis->setRange(localMin, localMax);
		}
	}
}

void ChartsPanel::updateSeries(int chartIndex, const QString& label, ParameterTreeHistoryItem* data, bool isBackPlaying)
{
	if (!data || chartIndex < 0 || chartIndex >= m_charts.count()) return;
	auto& chart = m_charts[chartIndex];
	QCPGraph* graph = chart.seriesMap.value(label);
	if (!graph) return;

	const auto& times = data->timestamps();
	const auto& values = data->values();
	if (times.isEmpty() || values.isEmpty()) return;

	const bool throttleAxis = isLivePlayer();
	if (graph->data()->isEmpty())
	{
		appendNumericPoints(graph, times, values);
		if (!graph->data()->isEmpty())
		{
			refreshTimeAxisIfNeeded(chart, graph->data()->firstKey(), graph->data()->lastKey(), throttleAxis);
		}
		return;
	}

	QList<QDateTime> newTimes;
	QList<QVariant> newValues;
	filterDataOutsideRange(times, values,
		fromPlotKey(graph->data()->firstKey()),
		fromPlotKey(graph->data()->lastKey()),
		isBackPlaying, newTimes, newValues);
	if (newTimes.isEmpty()) return;

	for (int i = 0; i < newTimes.count(); ++i)
	{
		bool ok = false;
		const double y = newValues[i].toDouble(&ok);
		if (!ok) continue;
		const double key = toPlotKey(newTimes[i]);
		if (graph->data()->contains(key)) continue;
		graph->addData(key, y);
	}

	if (isBackPlaying) graph->removeDataAfter(graph->data()->firstKey() + visibleSpanSeconds());
	else graph->removeDataBefore(graph->data()->lastKey() - visibleSpanSeconds());
	if (graph->data()->isEmpty()) return;

	refreshTimeAxisIfNeeded(chart, graph->data()->firstKey(), graph->data()->lastKey(), throttleAxis);

	double localMin = std::numeric_limits<double>::max();
	double localMax = std::numeric_limits<double>::lowest();
	bool hasNew = false;
	for (const auto& v : newValues)
	{
		bool ok = false;
		const double d = v.toDouble(&ok);
		if (!ok) continue;
		localMin = qMin(localMin, d);
		localMax = qMax(localMax, d);
		hasNew = true;
	}
	if (hasNew && chart.valueAxis)
	{
		QCPRange range = chart.valueAxis->range();
		bool changed = false;
		if (range.lower > localMin) { range.lower = localMin; changed = true; }
		if (range.upper < localMax) { range.upper = localMax; changed = true; }
		if (changed) chart.valueAxis->setRange(range);
	}
}

void ChartsPanel::filterDataOutsideRange(
	const QList<QDateTime>& times, const QList<QVariant>& values,
	const QDateTime& seriesStartTime, const QDateTime& seriesEndTime,
	bool isBackPlaying, QList<QDateTime>& outTimes, QList<QVariant>& outValues) const
{
	outTimes.clear();
	outValues.clear();
	if (times.isEmpty() || values.isEmpty() || times.count() != values.count()) return;
	for (int i = 0; i < times.count(); ++i)
	{
		const QDateTime& t = times[i];
		if (isBackPlaying)
		{
			if (t < seriesStartTime) { outTimes.append(t); outValues.append(values[i]); }
		}
		else if (t > seriesEndTime)
		{
			outTimes.append(t);
			outValues.append(values[i]);
		}
	}
}

ChartView* ChartsPanel::getChartView(int chartIndex) const
{
	if (chartIndex < 0 || chartIndex >= m_charts.count()) return nullptr;
	return m_charts[chartIndex].view;
}

QList<ChartView*> ChartsPanel::chartViewList() const
{
	QList<ChartView*> views;
	for (const auto& chart : m_charts)
	{
		if (chart.view) views.append(chart.view);
	}
	return views;
}

void ChartsPanel::reindexChartViews()
{
	for (int i = 0; i < m_charts.count(); ++i)
	{
		if (m_charts[i].view) m_charts[i].view->setChartIndex(i);
	}
	relayoutChartsGrid();
}

bool ChartsPanel::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == m_scrollArea->viewport() && event->type() == QEvent::Resize)
	{
		if (!m_interactionPaused) updateCellSizes();
	}
	return QFrame::eventFilter(watched, event);
}

void ChartsPanel::updateCellSizes()
{
	if (!m_gridLayout || !m_scrollArea) return;
	const int viewportWidth = m_scrollArea->viewport()->width();
	const QMargins margins = m_gridLayout->contentsMargins();
	const int hSpacing = m_gridLayout->horizontalSpacing();
	const int contentWidth = viewportWidth - margins.left() - margins.right();
	if (contentWidth <= 0 || m_columnCount <= 0) return;
	const int cellWidth = (contentWidth - hSpacing * (m_columnCount - 1)) / m_columnCount;
	if (cellWidth <= 0) return;
	for (int i = 0; i < m_gridLayout->count(); ++i)
	{
		if (auto* item = m_gridLayout->itemAt(i))
		{
			if (auto* w = item->widget())
			{
				w->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
				w->setFixedSize(cellWidth, cellWidth);
			}
		}
	}
	m_scrollContent->adjustSize();
}

void ChartsPanel::onToggleColumnClicked()
{
	if (m_columnCount == 1)
	{
		m_columnCount = 2;
		m_toggleColumnButton->setIcon(QIcon(":/Resources/icons8-one_column-32.png"));
		m_toggleColumnButton->setToolTip(tr("Switch to one column"));
	}
	else
	{
		m_columnCount = 1;
		m_toggleColumnButton->setIcon(QIcon(":/Resources/icons8-two_column-32.png"));
		m_toggleColumnButton->setToolTip(tr("Switch to two columns"));
	}
	relayoutChartsGrid();
}

void ChartsPanel::onMergeChartsClicked()
{
	if (m_chartsModel) m_chartsModel->mergeSelectedCharts();
}

void ChartsPanel::relayoutChartsGrid()
{
	if (!m_gridLayout) return;
	QList<ChartView*> views = chartViewList();
	std::sort(views.begin(), views.end(), [](ChartView* a, ChartView* b) {
		return a->chartIndex() < b->chartIndex();
	});
	for (auto* v : views) m_gridLayout->removeWidget(v);
	for (int idx = 0; idx < views.size(); ++idx)
	{
		m_gridLayout->addWidget(views.at(idx),
			m_columnCount > 0 ? idx / m_columnCount : 0,
			m_columnCount > 0 ? idx % m_columnCount : 0);
	}
	updateCellSizes();
}
