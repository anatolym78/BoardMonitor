#include "TelemetryDataView.h"

#include <QHeaderView>
#include <QMouseEvent>

#include "../Tools/TelemetryDelegate.h"
#include "../../ViewModel/BoardParametersTreeModel.h"
#include "../../Model/Parameters/Tree/ParameterTreeItem.h"

TelemetryDataView::TelemetryDataView(QWidget *parent)
	: QTreeView(parent)
{
	setAlternatingRowColors(false);
	setFrameShape(QFrame::NoFrame);
	setRootIsDecorated(true);
	setItemsExpandable(true);
	setAllColumnsShowFocus(true);
	setUniformRowHeights(true);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setExpandsOnDoubleClick(true);

	header()->hide();

	setMouseTracking(true);
}

void TelemetryDataView::setModel(QAbstractItemModel* model)
{
	if (this->model())
	{
		disconnect(this->model(), nullptr, this, nullptr);
	}

	QTreeView::setModel(model);

	if (!model)
	{
		return;
	}

	model->setHeaderData(0, Qt::Horizontal, tr("label"));
	model->setHeaderData(1, Qt::Horizontal, tr("value"));

	header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

	setItemDelegateForColumn(1, new TelemetryDelegate(this));

	auto* boardModel = qobject_cast<BoardParametersTreeModel*>(model);
	if (boardModel)
	{
		connect(boardModel, &BoardParametersTreeModel::structureAboutToReset,
			this, &TelemetryDataView::saveExpandedPaths);
	}

	connect(model, &QAbstractItemModel::modelReset, this, &TelemetryDataView::restoreExpandedPaths);

	if (model->rowCount() > 0)
	{
		applyDefaultExpansion();
	}
}

void TelemetryDataView::applyDefaultExpansion()
{
	if (!model() || model()->rowCount() == 0)
	{
		return;
	}

	expandToDepth(0);
	saveExpandedPaths();
}

void TelemetryDataView::saveExpandedPaths()
{
	m_expandedPaths.clear();
	if (!model())
	{
		return;
	}

	collectExpandedPaths(QModelIndex(), 0);
}

void TelemetryDataView::collectExpandedPaths(const QModelIndex& parent, int depth)
{
	const int rows = model()->rowCount(parent);
	for (int row = 0; row < rows; ++row)
	{
		const QModelIndex index = model()->index(row, 0, parent);
		// Верхний уровень (группы) всегда восстанавливается через expandToDepth(0)
		if (isExpanded(index) && depth > 0)
		{
			auto* item = static_cast<ParameterTreeItem*>(index.internalPointer());
			if (item)
			{
				m_expandedPaths.insert(item->fullName());
			}
		}

		if (model()->hasChildren(index))
		{
			collectExpandedPaths(index, depth + 1);
		}
	}
}

void TelemetryDataView::restoreExpandedPaths()
{
	if (!model() || model()->rowCount() == 0)
	{
		return;
	}

	expandToDepth(0);

	if (!m_expandedPaths.isEmpty())
	{
		restoreExpandedPathsRecursive(QModelIndex(), 0);
	}
}

void TelemetryDataView::restoreExpandedPathsRecursive(const QModelIndex& parent, int depth)
{
	const int rows = model()->rowCount(parent);
	for (int row = 0; row < rows; ++row)
	{
		const QModelIndex index = model()->index(row, 0, parent);
		auto* item = static_cast<ParameterTreeItem*>(index.internalPointer());
		if (item && depth > 0 && m_expandedPaths.contains(item->fullName()))
		{
			expand(index);
		}

		if (model()->hasChildren(index))
		{
			restoreExpandedPathsRecursive(index, depth + 1);
		}
	}
}

void TelemetryDataView::mouseMoveEvent(QMouseEvent* event)
{
	QModelIndex index = indexAt(event->pos());

	if (index.isValid())
	{
		auto treeItem = static_cast<ParameterTreeHistoryItem*>(index.internalPointer());

		if (treeItem && treeItem->type() == ParameterTreeItem::ItemType::History)
		{
			emit itemHovered(treeItem);
		}
	}

	QTreeView::mouseMoveEvent(event);
}

void TelemetryDataView::leaveEvent(QEvent* event)
{
	emit itemHovered(nullptr);

	QTreeView::leaveEvent(event);
}
