#include "ChatViewGridModel.h"
#include <QDebug>
#include <QDateTime>
#include <limits>

#include "DriverDataPlayer.h"
#include "SessionPlayer.h"

namespace
{

/** Ось времени QCustomPlot в режиме ltDateTime измеряется в секундах с эпохи. */
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

bool shouldRefreshTimeAxis(const ChatViewGridModel::ChartInfo& chart, double endKey)
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

void refreshTimeAxisIfNeeded(ChatViewGridModel::ChartInfo& chart, double beginKey, double endKey, bool throttle)
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

	// QCPGraph::addData вставляет через insertMulti, поэтому дубликаты отсекаем сами
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

void replotChart(const ChatViewGridModel::ChartInfo& chart)
{
	if (chart.plot)
	{
		chart.plot->replot(QCustomPlot::rpQueued);
	}
}

} // namespace

ChatViewGridModel::ChatViewGridModel(QObject *parent) : QAbstractListModel(parent)
{
}

void ChatViewGridModel::setPlayer(DataPlayer* dataPlayer)
{
	m_dataPlayer = dataPlayer;

	if (m_playConnection)
	{
		QObject::disconnect(m_playConnection);
	}

	m_playConnection = connect(dataPlayer, &DataPlayer::played,
		this, &ChatViewGridModel::onPlayed);
}

void ChatViewGridModel::setChartInteractionPaused(bool paused)
{
	m_chartInteractionPaused = paused;
}

bool ChatViewGridModel::isLivePlayer() const
{
	return m_dataPlayer && qobject_cast<DriverDataPlayer*>(m_dataPlayer) != nullptr;
}

void ChatViewGridModel::collectHistoryItems(ParameterTreeItem* item, QList<ParameterTreeHistoryItem*>& out) const
{
	if (!item)
	{
		return;
	}

	if (item->type() == ParameterTreeItem::ItemType::History)
	{
		out.append(static_cast<ParameterTreeHistoryItem*>(item));
		return;
	}

	for (ParameterTreeItem* child : item->children())
	{
		collectHistoryItems(child, out);
	}
}

bool ChatViewGridModel::isParameterDisplayed(ParameterTreeItem* parameter) const
{
	if (!parameter)
	{
		return false;
	}

	if (parameter->type() == ParameterTreeItem::ItemType::History)
	{
		return hasSeries(parameter->fullName());
	}

	QList<ParameterTreeHistoryItem*> historyItems;
	collectHistoryItems(parameter, historyItems);
	if (historyItems.isEmpty())
	{
		return false;
	}

	for (ParameterTreeHistoryItem* historyItem : historyItems)
	{
		if (!hasSeries(historyItem->fullName()))
		{
			return false;
		}
	}

	return true;
}

void ChatViewGridModel::showParameter(ParameterTreeItem* parameter)
{
	if (!parameter || isParameterDisplayed(parameter))
	{
		return;
	}

	const QString parameterFullName = parameter->fullName();
	beginInsertRows(QModelIndex(), rowCount(QModelIndex()), rowCount(QModelIndex()));
	m_charts.append(ChartInfo{ parameterFullName, QStringList() << parameterFullName });
	endInsertRows();

	emit parameterAdded(m_charts.count() - 1, parameter);
}

void ChatViewGridModel::hideParameter(ParameterTreeItem* parameter)
{
	if (!parameter)
	{
		return;
	}

	QList<ParameterTreeHistoryItem*> historyItems;
	if (parameter->type() == ParameterTreeItem::ItemType::History)
	{
		historyItems.append(static_cast<ParameterTreeHistoryItem*>(parameter));
	}
	else
	{
		collectHistoryItems(parameter, historyItems);
	}

	for (ParameterTreeHistoryItem* historyItem : historyItems)
	{
		const QString label = historyItem->fullName();
		if (hasSeries(label))
		{
			removeSeries(label);
		}
	}
}

void ChatViewGridModel::toggleParameter(ParameterTreeItem* parameter)//QString chartName)
{
	if (parameter == nullptr) return;

	auto parameterFullName = parameter->fullName();
	if (hasSeries(parameterFullName))
	{
		removeSeries(parameterFullName);
	}
	else
	{
		showParameter(parameter);
	}
}

void ChatViewGridModel::addSeriesToChart(int chartIndex, const QString& label, const QColor& color, QCustomPlot* plot, QCPGraph* graph, QCPAxis* timeAxis, QCPAxis* valueAxis)
{
	if (chartIndex < 0 || chartIndex >= m_charts.count()) return;
	if (!graph) return;

	QPen graphPen = graph->pen();
	graphPen.setColor(color);
	graph->setPen(graphPen);
	graph->setName(label);

	m_charts[chartIndex].seriesMap[label] = QPointer<QCPGraph>(graph);
	m_charts[chartIndex].plot = plot;
	m_charts[chartIndex].timeAxis = timeAxis;
	m_charts[chartIndex].valueAxis = valueAxis;

	if (m_dataPlayer && m_dataPlayer->storage())
	{
		ParameterTreeHistoryItem* history = m_dataPlayer->storage()->findHistoryItemByFullName(label);
		if (history)
		{
			if (isLivePlayer())
			{
				updateSeries(label, history, false);
			}
			else
			{
				rebuildSeriesFromHistory(label, history);
			}
		}
	}
}

void ChatViewGridModel::onPlayed(ParameterTreeStorage* snapshot, bool isBackPlaying)
{
	if (m_chartInteractionPaused)
	{
		return;
	}

	const bool livePlayer = isLivePlayer();
	const bool seekUpdate = !livePlayer
		&& m_dataPlayer
		&& qobject_cast<SessionPlayer*>(m_dataPlayer)
		&& static_cast<SessionPlayer*>(m_dataPlayer)->isSeekUpdate();

	ParameterTreeStorage* storage = snapshot;
	if (!storage && livePlayer && m_dataPlayer)
	{
		storage = m_dataPlayer->storage();
	}
	if (!storage)
	{
		return;
	}

	for (auto& chart : m_charts)
	{
		for (auto key : chart.seriesMap.keys())
		{
			auto data = storage->findHistoryItemByFullName(key);
			if (seekUpdate)
			{
				rebuildSeriesFromHistory(key, data);
			}
			else
			{
				updateSeries(key, data, isBackPlaying);
			}
		}

		// QCustomPlot не перерисовывается сам, один replot на график за такт обновления
		replotChart(chart);
	}
}

void ChatViewGridModel::rebuildSeriesFromHistory(const QString& label, ParameterTreeHistoryItem* data)
{
	if (data == nullptr)
	{
		return;
	}

	const int chartIndex = findChartIndex(label);
	if (chartIndex == -1)
	{
		return;
	}

	auto& chart = m_charts[chartIndex];
	auto graph = chart.seriesMap[label];
	if (graph == nullptr)
	{
		return;
	}

	graph->clearData();

	const auto& times = data->timestamps();
	const auto& values = data->values();
	appendNumericPoints(graph, times, values);

	if (!graph->data()->isEmpty())
	{
		refreshTimeAxisIfNeeded(chart, graph->data()->firstKey(), graph->data()->lastKey(), false);

		if (chart.valueAxis)
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
				chart.valueAxis->setRange(localMin, localMax);
			}
		}
	}

	replotChart(chart);
}

void ChatViewGridModel::updateSeries(const QString& label, ParameterTreeHistoryItem* data, bool isBackPlaying)
{
	if (data == nullptr) return;

	// find chart
	auto chartIndex = findChartIndex(label);
	if (chartIndex == -1) return;

	// find series
	auto& chart = m_charts[chartIndex];
	auto graph = chart.seriesMap[label];
	if (graph == nullptr) return;

	// get chart axes
	auto valueAxis = chart.valueAxis;

	// get data values and times
	auto& times = data->timestamps();
	auto& values = data->values();
	if (times.isEmpty() || values.isEmpty()) return;

	const bool throttleAxis = isLivePlayer();

	// if series has no points, then add points and return
	if (graph->data()->isEmpty())
	{
		appendNumericPoints(graph, times, values);

		if (!graph->data()->isEmpty())
		{
			refreshTimeAxisIfNeeded(chart, graph->data()->firstKey(), graph->data()->lastKey(), throttleAxis);
		}

		return;
	}

	const QDateTime seriesStatTime = fromPlotKey(graph->data()->firstKey());
	const QDateTime seriesEndTime = fromPlotKey(graph->data()->lastKey());

	// Фильтруем данные, оставляя только те, что за пределами диапазона серии
	QList<QDateTime> newTimes;
	QList<QVariant> newValues;
	filterDataOutsideRange(times, values, seriesStatTime, seriesEndTime, isBackPlaying, newTimes, newValues);

	// Если нет новых данных для добавления
	if (newTimes.isEmpty())
	{
		return;
	}

	// QCPDataMap хранит точки отсортированными по ключу, поэтому направление
	// воспроизведения на порядок вставки не влияет
	for (auto i = 0; i < newTimes.count(); i++)
	{
		bool ok = false;
		const double y = newValues[i].toDouble(&ok);
		if (!ok)
		{
			continue;
		}

		const double key = toPlotKey(newTimes[i]);
		if (graph->data()->contains(key))
		{
			continue;
		}

		graph->addData(key, y);
	}

	// Удаление лишних точек (Trim)
	if (isBackPlaying)
	{
		graph->removeDataAfter(graph->data()->firstKey() + visibleSpanSeconds());
	}
	else
	{
		graph->removeDataBefore(graph->data()->lastKey() - visibleSpanSeconds());
	}

	if (graph->data()->isEmpty())
	{
		return;
	}

	// Обновляем оси
	refreshTimeAxisIfNeeded(chart, graph->data()->firstKey(), graph->data()->lastKey(), throttleAxis);

	// Ось значений расширяем по новым точкам: полный пересчёт по всей серии
	// на каждом такте слишком дорог
	double localMin = std::numeric_limits<double>::max();
	double localMax = std::numeric_limits<double>::lowest();
	bool hasNew = false;

	for (const auto& v : newValues)
	{
		bool ok = false;
		const double d = v.toDouble(&ok);
		if (!ok)
		{
			continue;
		}

		localMin = qMin(localMin, d);
		localMax = qMax(localMax, d);
		hasNew = true;
	}

	if (hasNew && valueAxis)
	{
		QCPRange range = valueAxis->range();
		bool rangeChanged = false;

		if (range.lower > localMin)
		{
			range.lower = localMin;
			rangeChanged = true;
		}

		if (range.upper < localMax)
		{
			range.upper = localMax;
			rangeChanged = true;
		}

		if (rangeChanged)
		{
			valueAxis->setRange(range);
		}
	}
}

void ChatViewGridModel::filterDataOutsideRange(
	const QList<QDateTime>& times,
	const QList<QVariant>& values,
	const QDateTime& seriesStartTime,
	const QDateTime& seriesEndTime,
	bool isBackPlaying,
	QList<QDateTime>& outTimes,
	QList<QVariant>& outValues)
{
	if (times.isEmpty() || values.isEmpty() || times.count() != values.count())
	{
		return;
	}

	outTimes.clear();
	outValues.clear();
	outTimes.reserve(times.count());
	outValues.reserve(values.count());

	for (int i = 0; i < times.count(); ++i)
	{
		const auto& time = times[i];
		bool isOutside = false;

		if (isBackPlaying)
		{
			// При обратном воспроизведении нас интересуют данные, которые РАНЬШЕ начала серии
			// (или позже конца, если мы "прыгнули", но основной кейс - расширение влево)
			// times при backPlaying обычно идут в обратном порядке (от новых к старым), но проверим каждое значение.
			if (time < seriesStartTime)
			{
				isOutside = true;
			}
		}
		else
		{
			// При прямом воспроизведении нас интересуют данные, которые ПОЗЖЕ конца серии
			if (time > seriesEndTime)
			{
				isOutside = true;
			}
		}

		if (isOutside)
		{
			outTimes.append(time);
			outValues.append(values[i]);
		}
	}
}

void ChatViewGridModel::toggleParameter(const QString &label, const QColor &color)
{
	if(hasSeries(label))
	{
		removeSeries(label);
	}
	else
	{
		addSeries(label, color);
	}
}

void ChatViewGridModel::addSeries(const QString &label, const QColor& color)
{
	//if (!parameterExistsInHistory(label))
	//{
	//	qWarning() << "ChatViewGridModel: Parameter" << label << "does not exist in history";
	//	return;
	//}
	//
	//if (hasSeries(label))
	//{
	//	//qDebug() << "ChatViewGridModel: Chart with label" << label << "already exists";
	//	return;
	//}

	//beginInsertRows(QModelIndex(), rowCount(QModelIndex()), rowCount(QModelIndex()));
	//m_charts.append(ChartInfo{ "", QList<QString>() << label, color, false});
	//endInsertRows();

	//emit parameterAdded(m_charts.count() - 1, label, color);
}

void ChatViewGridModel::removeSeries(const QString &label)
{
	auto chartIndex = findChartIndex(label);
	if (chartIndex == -1) return;

	// remove sereis from qml
	emit parameterNeedToRemove(chartIndex, label);

	// remove series from cpp model
	m_charts[chartIndex].seriesMap.remove(label);

	// remove empty chart
	if (m_charts[chartIndex].seriesMap.isEmpty())
	{
		this->beginRemoveRows(QModelIndex(), chartIndex, chartIndex);
		m_charts.removeAt(chartIndex);
		this->endRemoveRows();
	}
	else
	{
		updateValueAxisRange(chartIndex);
	}
}

void ChatViewGridModel::moveSeriesToChart(int targetChartIndex, const QString& label, QCustomPlot* plot, QCPGraph* graph)
{
	if (targetChartIndex < 0 || targetChartIndex >= m_charts.count()) return;

	// add series to target chart
	m_charts[targetChartIndex].seriesMap[label] = QPointer<QCPGraph>(graph);
	m_charts[targetChartIndex].plot = plot;

	// remove moved series from chart
	for (auto index = 0; index < m_charts.count(); index++)
	{
		if(index == targetChartIndex) continue;

		auto& chart = m_charts[index];
		if (chart.seriesMap.contains(label))
		{
			chart.seriesMap.remove(label);

			break;
		}
	}

	removeEmptyCharts();
}

int ChatViewGridModel::countEmptyCharts() const
{
	return std::count_if(m_charts.begin(), m_charts.end(), [](ChartInfo chartInfo)
		{
			return chartInfo.seriesMap.isEmpty();
		});
}

int ChatViewGridModel::firstEmptyChart() const
{
	for (auto i = 0; i < m_charts.count(); i++)
	{
		if (m_charts[i].seriesMap.isEmpty()) return i;
	}

	return -1;
}

void ChatViewGridModel::removeEmptyCharts()
{
	while (countEmptyCharts() > 0)
	{
		int indexToRemove = firstEmptyChart();
		this->beginRemoveRows(QModelIndex(), indexToRemove, indexToRemove);
		m_charts.removeAt(indexToRemove);
		this->endRemoveRows();
	}
}

void ChatViewGridModel::fillSeries(const QString& label, QColor color, bool isInitialFill)
{
	//auto chartIndex = findChartIndex(label);
	//if (chartIndex == -1) return;

	//auto seriesMap = m_charts[chartIndex].seriesMap;
	//auto timeAxis = m_charts[chartIndex].timeAxis;
	//auto valueAxis = m_charts[chartIndex].valueAxis;

	//// extract parameters in [-1, 1] minute range
	//auto playerTime = player()->currentPosition();
	//auto startPos = playerTime.addMSecs(-minuteIntervalMsec());
	//auto endPos = playerTime.addMSecs(minuteIntervalMsec());
	//auto parameters = storage()->getParametersInTimeRange(startPos, endPos, label);

	//if (parameters.isEmpty()) return;

	//// fill sereis with extracted parameters
	//auto series = seriesMap[label];
	//for (auto p : parameters)
	//{
	//	bool ok;
	//	auto value = p->value().toDouble(&ok);

	//	if (ok)
	//	{
	//		series->append(p->timestamp().toMSecsSinceEpoch(), value);
	//	}
	//}

	//// set color
	//series->setColor(color);

	//// calc time range
	//if (isInitialFill)
	//{
	//	QDateTime firstTime = playerTime;	//    
	//	QDateTime firstParameterTime = parameters.first()->timestamp();
	//	QDateTime lastParameterTime = parameters.last()->timestamp();
	//	for (auto p : parameters)
	//	{
	//		//      ,    
	//		if (p->timestamp().msecsTo(playerTime) < minuteIntervalMsec()/2)
	//		{
	//			firstTime = p->timestamp();

	//			break;
	//		}

	//		//      ,    
	//		if (p->timestamp().msecsTo(lastParameterTime) >= minuteIntervalMsec())
	//		{
	//			firstTime = p->timestamp();

	//			break;
	//		}
	//	}

	//	timeAxis->setMin(firstTime);
	//	timeAxis->setMax(firstTime.addMSecs(minuteIntervalMsec()));
	//}

	//updateValueAxisRange(chartIndex);
}

QColor ChatViewGridModel::labelColor(QString label)
{
	auto chartIndex = findChartIndex(label);

	if (chartIndex == -1) return QColor(Qt::black);

	auto graph = m_charts[chartIndex].seriesMap[label];

	return graph ? graph->pen().color() : QColor(Qt::black);
}

void ChatViewGridModel::updateValueAxisRange(int chartIndex)
{
	if (chartIndex < 0 || chartIndex >= m_charts.count())
	{
		return;
	}

	auto& chart = m_charts[chartIndex];
	auto valueAxis = chart.valueAxis;

	if (valueAxis == nullptr)
	{
		return;
	}

	if (chart.seriesMap.isEmpty())
	{
		valueAxis->setRange(0, 10);
		replotChart(chart);
		return;
	}

	double minValue = std::numeric_limits<double>::max();
	double maxValue = std::numeric_limits<double>::lowest();
	bool hasPoints = false;

	for (auto graph : chart.seriesMap)
	{
		if (graph)
		{
			for (const QCPData& point : *graph->data())
			{
				minValue = std::min(minValue, point.value);
				maxValue = std::max(maxValue, point.value);
				hasPoints = true;
			}
		}
	}

	if (hasPoints)
	{
		auto range = maxValue - minValue;
		if (qFuzzyIsNull(range))
		{
			valueAxis->setRange(minValue - 1, maxValue + 1);
		}
		else
		{
			valueAxis->setRange(minValue - range * 0.1, maxValue + range * 0.1);
		}
	}
	else
	{
		valueAxis->setRange(0, 10);
	}

	replotChart(chart);
}

void ChatViewGridModel::mergeCharts()
{
	auto indices = selectedIndices();

	if (indices.count() < 2) return;

	auto targetChartIndex = indices.first();
	auto indicesToRemove = QList(indices.begin() + 1, indices.end());

	QStringList labelsToMove;
	for (auto index : indicesToRemove)
	{
		auto chart = m_charts[index];
		auto labels = chart.seriesMap.keys();
		labelsToMove.append(labels);
		for (auto label : labels)
		{
			m_charts[targetChartIndex].seriesMap.insert(label, chart.seriesMap[label]);
		}
		m_charts[index].seriesMap.clear();
	}

	removeEmptyCharts();

	emit parametersNeedToMove(targetChartIndex, labelsToMove);

	clearSelection();
}


void ChatViewGridModel::mergeSelectedCharts()
{
	auto indices = selectedIndices();

	if (indices.count() < 2) return;

	auto targetChartIndex = indices.first();
	auto indicesToRemove = QList(indices.begin() + 1, indices.end());

	QStringList labelsToMove;
	for (auto index : indicesToRemove)
	{
		auto chart = m_charts[index];
		labelsToMove.append(chart.seriesMap.keys());
	}

	emit parametersNeedToMove(targetChartIndex, labelsToMove);

	clearSelection();
}

QList<int> ChatViewGridModel::selectedIndices() const
{
	QList<int> indices;
	for (auto index = 0; index < m_charts.count(); index++)
	{
		if (m_charts[index].isSelected)
		{
			indices.append(index);
		}
	}

	return indices;
}

int ChatViewGridModel::countSelectedIndices() const
{
	return std::count_if(m_charts.begin(), m_charts.end(), [](ChartInfo chart) { return chart.isSelected; });
}


bool ChatViewGridModel::isSeriesCreated(const QString& label) const
{
	for (const auto& chart : m_charts)
	{
		if (chart.seriesMap.contains(label)) return true;
	}

	return false;
}

int ChatViewGridModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid()) return 0;

	return m_charts.count();
}

QVariant ChatViewGridModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())
		return {};

	auto cellIndex = index.row();// cellToIndex(index);

	if(cellIndex >= m_charts.count()) 
		return {};

	if(m_charts.isEmpty()) 
		return {};

	if (role == LabelsRole)
	{
		return m_charts[cellIndex].series;
	}

	if (role == LabelRole)
	{
		return m_charts[cellIndex].series.last();
	}

	if (role == SelectionRole)
	{
		return m_charts[cellIndex].isSelected;
	}

	if (role == HoverRole)
	{
		return m_hoverIndex == cellIndex;
	}

	if (role == ColorRole)
	{
		return QVariant::fromValue(m_charts[cellIndex].color);
	}

	if (role == seriesMapRole)
	{
		return QVariant::fromValue(m_charts[cellIndex].seriesMap);
	}


	return m_charts[cellIndex].series.last();
}

bool ChatViewGridModel::parameterExistsInHistory(const QString& label) const
{
	return false; // !!!
	//return m_dataPlayer->storage()->containsParameter(label);
}

void ChatViewGridModel::splitSeries(int chartIndex)
{
	return; // not realized

	if(chartIndex >= 0 && chartIndex < m_charts.count())
	{
		auto series = m_charts[chartIndex].series;

		m_charts.removeAt(chartIndex);
		for(auto s : series)
		{
			m_charts.append(ChartInfo{ "", QStringList() << s, Qt::darkGray, false});
		}
	}

	beginInsertRows(QModelIndex(), m_charts.size(), m_charts.size());
	endInsertRows();
}

void ChatViewGridModel::clearCharts()
{
	return; // !!!
	if (m_charts.isEmpty())
		return;
		
	beginResetModel();
	m_charts.clear();
	endResetModel();
}

QStringList ChatViewGridModel::getChartSeriesLabels(int chartIndex) const
{
	if(chartIndex < 0 || chartIndex >= rowCount()) return QStringList();

	return m_charts[chartIndex].series;
}

QStringList ChatViewGridModel::chartLabels() const
{
	QStringList labels;
	for(auto& chart : m_charts)
	{
		labels.append(chart.series);
	}

	return labels;
}

bool ChatViewGridModel::hasSeries(const QString &label) const
{
	for(auto& chart : m_charts)
	{
		if(chart.seriesMap.contains(label)) return true;
	}

	return false;
}

bool ChatViewGridModel::selectElement(int index, bool keepSelection)
{
	if (keepSelection == false)
	{
		clearSelection();
	}

	if (index >= 0 && index < m_charts.count())
	{
		m_charts[index].isSelected =  !m_charts[index].isSelected;
	}


	emit isCanMergeChartsChanged();

	updateAllCells();

	return true;
}

void ChatViewGridModel::clearSelection()
{
	for (auto& chart : m_charts)
	{
		chart.isSelected = false;
	}

	emit isCanMergeChartsChanged();

	updateAllCells();
}

bool ChatViewGridModel::hoverElement(int index)
{
	m_hoverIndex = index;
	updateAllCells();
	return true;
}

void ChatViewGridModel::clearHover()
{
	m_hoverIndex = -1;

	updateAllCells();
}

bool ChatViewGridModel::isCanMergeCharts() const
{
	auto countSelectedCharts = 0;
	for (auto& chart : m_charts)
	{
		countSelectedCharts += (int)(chart.isSelected == true);
	}

	return countSelectedCharts > 1;
}

void ChatViewGridModel::updateAllCells()
{
	for (auto i = 0; i < m_charts.count(); i++)
	{
		emit dataChanged(this->createIndex(i, 0), this->createIndex(i, 0));
	}
}

int ChatViewGridModel::findChartIndex(const QString &label) const
{ 
	for (auto i=0;i<m_charts.count();i++)
	{
		if (m_charts[i].seriesMap.contains(label))
		{
			return i;
		}
	}

	return -1;
}

QHash<int, QByteArray> ChatViewGridModel::roleNames() const
{
	QHash<int, QByteArray> roles;

	roles[LabelsRole] = "labels";
	roles[LabelRole] = "label";
	roles[ChartIndexRole] = "chartIndex";
	roles[DepthRole] = "depth";
	roles[SelectionRole] = "selection";
	roles[HoverRole] = "hover";
	roles[ColorRole] = "parameterColor";
	roles[seriesMapRole] = "seriesMap";

	return roles;
}
