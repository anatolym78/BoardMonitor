#include "TelemetryDataView.h"

#include <QHeaderView>
#include <QMouseEvent>

#include "../Tools/TelemetryDelegate.h"

//#include "./../Model/Parameters/Tree/ParameterTreeHistoryItem.h"


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
	QTreeView::setModel(model);

	if (!model)
		return;
	
	model->setHeaderData(0, Qt::Horizontal, tr("label"));
	model->setHeaderData(1, Qt::Horizontal, tr("value"));

	header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

	// Устанавливаем делегат для колонки значений (индекс 1)
	setItemDelegateForColumn(1, new TelemetryDelegate(this));

	if (model->rowCount() > 0)
	{
		this->expandAll();
	}

	// Раскрываем дерево при каждом структурном изменении модели (добавление параметров).
	// modelReset срабатывает только на структурные изменения, не на обновления значений,
	// поэтому постоянное соединение не мешает пользователю управлять деревом вручную.
	connect(model, &QAbstractItemModel::modelReset, this, [this, model]()
	{
		if (model->rowCount() > 0)
		{
			expandToDepth(0);
		}
	});
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
	emit itemHovered(nullptr); // Отправляем пустой индекс

	QTreeView::leaveEvent(event);
}
