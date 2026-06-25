#include "ChatViewGridModel.h"
#include <QDebug>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <QDateTime>
#include <limits>

#include "DriverDataPlayer.h"
#include "SessionPlayer.h"

namespace
{

void applyTimeAxisRange(QtCharts::QDateTimeAxis* axis, qint64 beginMs, qint64 endMs)
{
	if (!axis)
	{
		return;
	}

	const qint64 span = qMax<qint64>(endMs - beginMs, 1000);
	const qint64 pad = span / 20;
	axis->setMin(QDateTime::fromMSecsSinceEpoch(beginMs - pad));
	axis->setMax(QDateTime::fromMSecsSinceEpoch(endMs + pad));
}

bool shouldRefreshTimeAxis(const ChatViewGridModel::ChartInfo& chart, qint64 endMs)
{
	if (!chart.isAxesInitialized)
	{
		return true;
	}

	const qint64 now = QDateTime::currentMSecsSinceEpoch();
	if (endMs > chart.lastAxisEndMs + 1000)
	{
		return true;
	}

	return (now - chart.lastAxisUpdateMs) >= 250;
}

void refreshTimeAxisIfNeeded(ChatViewGridModel::ChartInfo& chart, qint64 beginMs, qint64 endMs, bool throttle)
{
	if (throttle && !shouldRefreshTimeAxis(chart, endMs))
	{
		return;
	}

	applyTimeAxisRange(chart.timeAxis, beginMs, endMs);
	chart.isAxesInitialized = true;
	chart.lastAxisEndMs = endMs;
	chart.lastAxisUpdateMs = QDateTime::currentMSecsSinceEpoch();
}

bool appendNumericPoints(QtCharts::QLineSeries* series, const QList<QDateTime>& times, const QList<QVariant>& values)
{
	if (!series || times.isEmpty() || times.count() != values.count())
	{
		return false;
	}

	qint64 lastX = series->count() > 0
		? static_cast<qint64>(series->at(series->count() - 1).x())
		: std::numeric_limits<qint64>::min();

	bool added = false;
	for (int i = 0; i < values.count(); ++i)
	{
		bool ok = false;
		const double y = values[i].toDouble(&ok);
		if (!ok)
		{
			continue;
		}

		const qint64 x = times[i].toMSecsSinceEpoch();
		if (x <= lastX)
		{
			continue;
		}

		series->append(x, y);
		lastX = x;
		added = true;
	}

	return added;
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

void ChatViewGridModel::addSeriesToChart(int chartIndex, const QString& label, const QColor& color, QtCharts::QLineSeries* series, QtCharts::QDateTimeAxis* timeAxis, QtCharts::QValueAxis* valueAxis)
{
	if (chartIndex < 0 || chartIndex >= m_charts.count()) return;

	series->setColor(color);
	series->setName(label);
	m_charts[chartIndex].seriesMap[label] = QPointer<QtCharts::QLineSeries>(series);
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
	auto series = chart.seriesMap[label];
	if (series == nullptr)
	{
		return;
	}

	series->clear();

	const auto& times = data->timestamps();
	const auto& values = data->values();
	appendNumericPoints(series, times, values);

	if (series->count() > 0)
	{
		const qint64 beginMs = static_cast<qint64>(series->at(0).x());
		const qint64 endMs = static_cast<qint64>(series->at(series->count() - 1).x());
		refreshTimeAxisIfNeeded(chart, beginMs, endMs, false);

		if (chart.valueAxis)
		{
			double localMin = std::numeric_limits<double>::max();
			double localMax = std::numeric_limits<double>::lowest();
			for (int i = 0; i < series->count(); ++i)
			{
				const double y = series->at(i).y();
				localMin = qMin(localMin, y);
				localMax = qMax(localMax, y);
			}
			if (localMin <= localMax)
			{
				chart.valueAxis->setMin(localMin);
				chart.valueAxis->setMax(localMax);
			}
		}
	}
}

void ChatViewGridModel::updateSeries(const QString& label, ParameterTreeHistoryItem* data, bool isBackPlaying)
{
	if (data == nullptr) return;

	// find chart
	auto chartIndex = findChartIndex(label);
	if (chartIndex == -1) return;

	// find series
	auto& chart = m_charts[chartIndex];
	auto series = chart.seriesMap[label];
	if (series == nullptr) return;

	// get chart axes
	auto timeAxis = chart.timeAxis;
	auto valueAxis = chart.valueAxis;

	// get data values and times
	auto& times = data->timestamps();
	auto& values = data->values();
	if (times.isEmpty() || values.isEmpty()) return;

	const bool throttleAxis = isLivePlayer();

	// if series has no points, then add points and return
	if (series->count() == 0)
	{
		appendNumericPoints(series, times, values);

		if (series->count() > 0)
		{
			const qint64 beginMs = static_cast<qint64>(series->at(0).x());
			const qint64 endMs = static_cast<qint64>(series->at(series->count() - 1).x());
			refreshTimeAxisIfNeeded(chart, beginMs, endMs, throttleAxis);
		}

		return;
	}

	auto playerCurrentTime = player()->currentPosition();

	auto axisStartTime = timeAxis->min();
	auto axisEndTime = timeAxis->max();

	QDateTime seriesStatTime;
	QDateTime seriesEndTime;

	if (series->count() > 0)
	{
		seriesStatTime = QDateTime::fromMSecsSinceEpoch(series->at(0).x());
		seriesEndTime = QDateTime::fromMSecsSinceEpoch(series->at(series->count() - 1).x());
	}

	// Фильтруем данные, оставляя только те, что за пределами диапазона серии
	QList<QDateTime> newTimes;
	QList<QVariant> newValues;
	
	if (series->count() == 0)
	{
		newTimes = times;
		newValues = values;
	}
	else
	{
		filterDataOutsideRange(times, values, seriesStatTime, seriesEndTime, isBackPlaying, newTimes, newValues);
	}

	// Если нет новых данных для добавления
	if (newTimes.isEmpty())
	{
		return;
	}

	// Добавляем отфильтрованные данные в серию
	if (isBackPlaying)
	{
		for (auto i = 0; i < newTimes.count(); i++)
		{
			bool ok = false;
			const double y = newValues[i].toDouble(&ok);
			if (!ok)
			{
				continue;
			}
			series->insert(0, QPointF(newTimes[i].toMSecsSinceEpoch(), y));
		}
	}
	else
	{
		qint64 lastX = static_cast<qint64>(series->at(series->count() - 1).x());
		for (auto i = 0; i < newTimes.count(); i++)
		{
			bool ok = false;
			const double y = newValues[i].toDouble(&ok);
			if (!ok)
			{
				continue;
			}

			const qint64 x = newTimes[i].toMSecsSinceEpoch();
			if (x <= lastX)
			{
				continue;
			}
			series->append(x, y);
			lastX = x;
		}
	}
	
	// Удаление лишних точек (Trim)
	qint64 intervalMs = minuteIntervalMsec();
	if (isBackPlaying)
	{
		// Добавляли в начало, удаляем с конца
		while (series->count() > 0)
		{
			qint64 first = (qint64)series->at(0).x();
			qint64 last = (qint64)series->at(series->count() - 1).x();
			
			if (last - first > intervalMs)
			{
				series->remove(series->count() - 1);
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		// Добавляли в конец, удаляем с начала
		while (series->count() > 0)
		{
			qint64 first = (qint64)series->at(0).x();
			qint64 last = (qint64)series->at(series->count() - 1).x();
			
			if (last - first > intervalMs)
			{
				series->remove(0);
			}
			else
			{
				break;
			}
		}
	}

	// Обновляем оси
	if (series->count() > 0)
	{
		const qint64 beginTime = static_cast<qint64>(series->at(0).x());
		const qint64 endTime = static_cast<qint64>(series->at(series->count() - 1).x());
		refreshTimeAxisIfNeeded(chart, beginTime, endTime, throttleAxis);
		
		// Для оси значений нужно найти min/max среди всех точек (или только новых?)
		// Лучше среди всех, так как диапазон мог сместиться
		// Но это дорого для большого количества точек. 
		// Оптимизация: при добавлении мы знаем новые значения. При удалении старых - сложнее.
		// Пока оставим старую логику обновления ValueAxis, но она берет lastValue
		// Обновим с учетом новых добавленных значений.
		
		// Упрощенно обновляем по последнему пришедшему (как было), или лучше по всей видимой серии?
		// В оригинале:
		// auto doubleValue = lastValue.toDouble();
		// if (valueAxis->min() > doubleValue) ...
		
		// Обновим границы по новым данным
		double localMin = std::numeric_limits<double>::max();
		double localMax = std::numeric_limits<double>::min();
		bool hasNew = false;
		
		for(const auto& v : newValues) 
		{
			double d = v.toDouble();
			if(d < localMin) localMin = d;
			if(d > localMax) localMax = d;
			hasNew = true;
		}
		
		if(hasNew)
		{
			if (valueAxis->min() > localMin) valueAxis->setMin(localMin);
			if (valueAxis->max() < localMax) valueAxis->setMax(localMax);
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

void ChatViewGridModel::moveSeriesToChart(int targetChartIndex, const QString& label, QtCharts::QLineSeries* series)
{
	auto color = labelColor(label);

	// add series to target chart
	m_charts[targetChartIndex].seriesMap[label] = QPointer<QtCharts::QLineSeries>(series);

	fillSeries(label, color, false);

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

	if (chartIndex == -1) return QColor().black();

	return m_charts[chartIndex].seriesMap[label]->color();
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
		valueAxis->setMin(0);
		valueAxis->setMax(10);
		return;
	}

	double minValue = std::numeric_limits<double>::max();
	double maxValue = std::numeric_limits<double>::min();
	bool hasPoints = false;

	for (auto series : chart.seriesMap)
	{
		if (series)
		{
			for (const auto& point : series->points())
			{
				minValue = std::min(minValue, point.y());
				maxValue = std::max(maxValue, point.y());
				hasPoints = true;
			}
		}
	}

	if (hasPoints)
	{
		auto range = maxValue - minValue;
		if (qFuzzyCompare(range, 0.0))
		{
			valueAxis->setMin(minValue - 1);
			valueAxis->setMax(maxValue + 1);
		}
		else
		{
			valueAxis->setMin(minValue - range * 0.1);
			valueAxis->setMax(maxValue + range * 0.1);
		}
	}
	else
	{
		valueAxis->setMin(0);
		valueAxis->setMax(10);
	}
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
