#include "ChartsModel.h"

#include "./../Model/Parameters/Tree/ParameterTreeStorage.h"

#include <algorithm>

ChartsModel::ChartsModel(QObject* parent)
	: QObject(parent)
{
}

void ChartsModel::setParameterTree(ParameterTreeStorage* tree)
{
	m_tree = tree;
}

void ChartsModel::collectHistoryItems(ParameterTreeItem* item, QList<ParameterTreeHistoryItem*>& out) const
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

bool ChartsModel::hasSeries(const QString& label) const
{
	return findChartIndex(label) != -1;
}

int ChartsModel::findChartIndex(const QString& label) const
{
	for (int i = 0; i < m_charts.count(); ++i)
	{
		if (m_charts[i].seriesLabels.contains(label) || m_charts[i].rootLabel == label)
		{
			return i;
		}
	}
	return -1;
}

QStringList ChartsModel::seriesLabels(int chartIndex) const
{
	if (chartIndex < 0 || chartIndex >= m_charts.count())
	{
		return {};
	}
	return m_charts[chartIndex].seriesLabels;
}

QString ChartsModel::chartTitle(int chartIndex) const
{
	if (chartIndex < 0 || chartIndex >= m_charts.count())
	{
		return {};
	}
	return m_charts[chartIndex].title;
}

bool ChartsModel::isParameterDisplayed(ParameterTreeItem* parameter) const
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

QString ChartsModel::formatLeafDisplayName(ParameterTreeItem* item) const
{
	if (!item)
	{
		return {};
	}

	auto* parent = item->parentItem();
	if (parent && parent->type() == ParameterTreeItem::ItemType::Array)
	{
		return parent->fullName() + QLatin1Char('[') + item->label() + QLatin1Char(']');
	}
	return item->fullName();
}

QString ChartsModel::formatLeafDisplayName(const QString& fullName) const
{
	if (!m_tree)
	{
		return fullName;
	}

	ParameterTreeHistoryItem* item = m_tree->findHistoryItemByFullName(fullName);
	if (!item)
	{
		return fullName;
	}
	return formatLeafDisplayName(item);
}

void ChartsModel::compressNode(ParameterTreeItem* node, QSet<QString>& remaining, QStringList& parts) const
{
	if (!node || remaining.isEmpty())
	{
		return;
	}

	if (node->type() == ParameterTreeItem::ItemType::Root)
	{
		for (ParameterTreeItem* child : node->children())
		{
			compressNode(child, remaining, parts);
		}
		return;
	}

	if (node->type() == ParameterTreeItem::ItemType::History)
	{
		const QString name = node->fullName();
		if (remaining.contains(name))
		{
			parts.append(formatLeafDisplayName(node));
			remaining.remove(name);
		}
		return;
	}

	// Group / Array: целиком → одно имя; иначе — спускаемся к детям
	QList<ParameterTreeHistoryItem*> allHistory;
	collectHistoryItems(node, allHistory);
	if (allHistory.isEmpty())
	{
		return;
	}

	bool allPresent = true;
	for (ParameterTreeHistoryItem* historyItem : allHistory)
	{
		if (!remaining.contains(historyItem->fullName()))
		{
			allPresent = false;
			break;
		}
	}

	if (allPresent)
	{
		parts.append(node->fullName());
		for (ParameterTreeHistoryItem* historyItem : allHistory)
		{
			remaining.remove(historyItem->fullName());
		}
		return;
	}

	for (ParameterTreeItem* child : node->children())
	{
		compressNode(child, remaining, parts);
	}
}

QString ChartsModel::buildCompressedTitle(const QStringList& seriesLabels) const
{
	if (seriesLabels.isEmpty())
	{
		return {};
	}

	QSet<QString> remaining;
	for (const QString& label : seriesLabels)
	{
		remaining.insert(label);
	}
	QStringList parts;

	if (m_tree)
	{
		compressNode(m_tree, remaining, parts);
	}

	// Серии, не найденные в дереве — как есть (с попыткой формата)
	for (const QString& label : seriesLabels)
	{
		if (remaining.contains(label))
		{
			parts.append(formatLeafDisplayName(label));
			remaining.remove(label);
		}
	}

	return parts.join(QStringLiteral(", "));
}

void ChartsModel::refreshChartTitle(int chartIndex)
{
	if (chartIndex < 0 || chartIndex >= m_charts.count())
	{
		return;
	}

	m_charts[chartIndex].title = buildCompressedTitle(m_charts[chartIndex].seriesLabels);
	emit chartTitleChanged(chartIndex, m_charts[chartIndex].title);
}

void ChartsModel::showParameter(ParameterTreeItem* parameter)
{
	if (!parameter || isParameterDisplayed(parameter))
	{
		return;
	}

	ChartSlot slot;
	slot.rootLabel = parameter->fullName();

	if (parameter->type() == ParameterTreeItem::ItemType::History)
	{
		slot.seriesLabels << parameter->fullName();
	}
	else
	{
		QList<ParameterTreeHistoryItem*> historyItems;
		collectHistoryItems(parameter, historyItems);
		for (ParameterTreeHistoryItem* item : historyItems)
		{
			slot.seriesLabels << item->fullName();
		}
	}

	if (slot.seriesLabels.isEmpty())
	{
		return;
	}

	slot.title = buildCompressedTitle(slot.seriesLabels);
	m_charts.append(slot);
	emit chartAdded(m_charts.count() - 1, parameter);
}

void ChartsModel::removeSeries(const QString& label)
{
	const int chartIndex = findChartIndex(label);
	if (chartIndex == -1)
	{
		return;
	}

	m_charts[chartIndex].seriesLabels.removeAll(label);
	emit seriesRemoved(chartIndex, label);

	if (m_charts[chartIndex].seriesLabels.isEmpty())
	{
		m_charts.removeAt(chartIndex);
		emit chartRemoved(chartIndex);
	}
	else
	{
		refreshChartTitle(chartIndex);
	}
}

void ChartsModel::hideParameter(ParameterTreeItem* parameter)
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

void ChartsModel::toggleParameter(ParameterTreeItem* parameter)
{
	if (!parameter)
	{
		return;
	}

	if (hasSeries(parameter->fullName()) || isParameterDisplayed(parameter))
	{
		hideParameter(parameter);
	}
	else
	{
		showParameter(parameter);
	}
}

bool ChartsModel::selectChart(int index, bool keepSelection)
{
	if (index < 0 || index >= m_charts.count())
	{
		return false;
	}

	if (!keepSelection)
	{
		for (auto& chart : m_charts)
		{
			chart.isSelected = false;
		}
	}

	m_charts[index].isSelected = !m_charts[index].isSelected;
	emit selectionChanged();
	return true;
}

void ChartsModel::clearSelection()
{
	for (auto& chart : m_charts)
	{
		chart.isSelected = false;
	}
	emit selectionChanged();
}

QList<int> ChartsModel::selectedIndices() const
{
	QList<int> indices;
	for (int i = 0; i < m_charts.count(); ++i)
	{
		if (m_charts[i].isSelected)
		{
			indices.append(i);
		}
	}
	return indices;
}

bool ChartsModel::canMergeCharts() const
{
	return selectedIndices().count() >= 2;
}

void ChartsModel::mergeSelectedCharts()
{
	auto indices = selectedIndices();
	if (indices.count() < 2)
	{
		return;
	}

	std::sort(indices.begin(), indices.end());
	const int targetChartIndex = indices.first();
	const QList<int> indicesToRemove(indices.begin() + 1, indices.end());

	QStringList labelsToMove;
	for (int index : indicesToRemove)
	{
		labelsToMove.append(m_charts[index].seriesLabels);
		m_charts[targetChartIndex].seriesLabels.append(m_charts[index].seriesLabels);
		m_charts[index].seriesLabels.clear();
	}

	refreshChartTitle(targetChartIndex);
	emit seriesMoved(targetChartIndex, labelsToMove);
	removeEmptyCharts();
	clearSelection();
}

void ChartsModel::removeEmptyCharts()
{
	for (int i = m_charts.count() - 1; i >= 0; --i)
	{
		if (m_charts[i].seriesLabels.isEmpty())
		{
			m_charts.removeAt(i);
			emit chartRemoved(i);
		}
	}
}
