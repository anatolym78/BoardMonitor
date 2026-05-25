#include "SessionListView.h"
#include "SessionListDelegate.h"

#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include "../../ViewModel/SessionSelectionModel.h"

SessionListView::SessionListView(QWidget *parent) : QListView(parent)
{
	setSelectionMode(QAbstractItemView::SingleSelection);

	setItemDelegate(new SessionListDelegate(this));

	auto selectionModel = this->selectionModel();
	connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SessionListView::onSelectionChanged);
}

void SessionListView::createSelectionModel()
{
	// Создаём и назначаем специализированную модель выделения на основе текущей модели данных
	auto dataModel = model();
	if (!dataModel) return;

	auto selModel = new SessionSelectionModel(dataModel, this);
	setSelectionModel(selModel);

	// Переподключаем сигнал на новый selectionModel
	disconnect(this->selectionModel(), nullptr, this, nullptr);
	connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SessionListView::onSelectionChanged);

}

void SessionListView::selectFirstItem()
{
	selectionModel()->select(this->model()->index(0, 0), QItemSelectionModel::ClearAndSelect);
}

void SessionListView::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	auto selectedIndexes = selected.indexes();

	if (selectedIndexes.count() == 1)
	{
		emit sessionSelected(selectedIndexes.first().row());
	}
}

