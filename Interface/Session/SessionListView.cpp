#include "SessionListView.h"
#include "SessionListDelegate.h"

#include "../../ViewModel/SessionSelectionModel.h"
#include "../../ViewModel/SessionsListModel.h"

#include <QItemSelectionModel>

SessionListView::SessionListView(QWidget *parent) : QTreeView(parent)
{
	setSelectionMode(QAbstractItemView::SingleSelection);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setRootIsDecorated(true);
	setItemsExpandable(true);
	setHeaderHidden(true);
	setUniformRowHeights(true);
	setExpandsOnDoubleClick(true);

	setItemDelegate(new SessionListDelegate(this));
}

void SessionListView::createSelectionModel()
{
	auto dataModel = model();
	if (!dataModel)
	{
		return;
	}

	auto selModel = new SessionSelectionModel(dataModel, this);
	setSelectionModel(selModel);

	disconnect(selectionModel(), nullptr, this, nullptr);
	connect(selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &SessionListView::onSelectionChanged);
	connect(selectionModel(), &QItemSelectionModel::currentChanged,
		this, &SessionListView::onCurrentChanged);

	disconnect(dataModel, nullptr, this, nullptr);
	connect(dataModel, &QAbstractItemModel::modelReset, this, &SessionListView::onModelReset);
	connect(dataModel, &QAbstractItemModel::rowsInserted,
		this, &SessionListView::onRowsInserted);
	connect(dataModel, &QAbstractItemModel::rowsRemoved,
		this, &SessionListView::onRowsRemoved);
	connect(dataModel, &QAbstractItemModel::dataChanged,
		this, &SessionListView::onModelDataChanged);
	onModelReset();
}

void SessionListView::onRowsInserted(const QModelIndex& parent, int first, int last)
{
	Q_UNUSED(first)
	Q_UNUSED(last)

	scheduleDelayedItemsLayout();
	updateGeometry();

	if (parent.isValid())
	{
		expand(parent);
	}

	if (viewport())
	{
		viewport()->update();
	}
}

void SessionListView::onRowsRemoved(const QModelIndex& parent, int first, int last)
{
	Q_UNUSED(parent)
	Q_UNUSED(first)
	Q_UNUSED(last)

	scheduleDelayedItemsLayout();
	updateGeometry();

	if (viewport())
	{
		viewport()->update();
	}
}

void SessionListView::refreshIndexVisual(const QModelIndex& index)
{
	if (!index.isValid())
	{
		return;
	}

	update(index);

	if (viewport())
	{
		const QRect area = visualRect(index);
		if (area.isValid())
		{
			viewport()->update(area);
		}
	}
}

void SessionListView::onModelDataChanged(const QModelIndex& topLeft,
	const QModelIndex& bottomRight, const QVector<int>& roles)
{
	Q_UNUSED(roles)

	refreshIndexVisual(topLeft);

	if (bottomRight.isValid() && bottomRight != topLeft)
	{
		refreshIndexVisual(bottomRight);
	}
}

void SessionListView::onModelReset()
{
	expandToDepth(1);
}

void SessionListView::selectFirstItem()
{
	if (!model() || model()->rowCount() == 0)
	{
		return;
	}

	const QModelIndex firstIndex = model()->index(0, 0);
	selectionModel()->setCurrentIndex(
		firstIndex,
		QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void SessionListView::selectFlatSessionIndex(int flatIndex)
{
	auto* sessionsModel = qobject_cast<SessionsListModel*>(model());
	if (!sessionsModel)
	{
		return;
	}

	const QModelIndex index = sessionsModel->indexForFlatSession(flatIndex);
	if (!index.isValid())
	{
		return;
	}

	expand(index.parent());
	selectionModel()->setCurrentIndex(
		index,
		QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void SessionListView::revealFlatSessionIndex(int flatIndex)
{
	auto* sessionsModel = qobject_cast<SessionsListModel*>(model());
	if (!sessionsModel)
	{
		return;
	}

	const QModelIndex sessionIndex = sessionsModel->indexForFlatSession(flatIndex);
	if (!sessionIndex.isValid())
	{
		return;
	}

	const QModelIndex parentIndex = sessionIndex.parent();
	if (parentIndex.isValid())
	{
		expand(parentIndex);
	}

	scrollTo(sessionIndex);
	refreshIndexVisual(sessionIndex);

	if (parentIndex.isValid())
	{
		refreshIndexVisual(parentIndex);
	}
}

void SessionListView::clearSessionSelection()
{
	if (!selectionModel())
	{
		return;
	}

	selectionModel()->clearSelection();
	selectionModel()->setCurrentIndex(QModelIndex(), QItemSelectionModel::Clear);
}

void SessionListView::onCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
	refreshIndexVisual(previous);
	refreshIndexVisual(current);
}

void SessionListView::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	for (const QModelIndex& index : deselected.indexes())
	{
		refreshIndexVisual(index);
	}

	for (const QModelIndex& index : selected.indexes())
	{
		refreshIndexVisual(index);
	}

	const auto selectedIndexes = selected.indexes();
	if (selectedIndexes.count() != 1)
	{
		return;
	}

	const int flatIndex = selectedIndexes.first().data(SessionsListModel::FlatSessionIndexRole).toInt();
	if (flatIndex >= 0)
	{
		emit sessionSelected(flatIndex);
	}
}
