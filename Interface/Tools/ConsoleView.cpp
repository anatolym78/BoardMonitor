#include "ConsoleView.h"
#include <QHeaderView>
#include <QAbstractItemModel>

ConsoleView::ConsoleView(QWidget *parent) : QTableView(parent)
{
	// Настройка заголовков
	verticalHeader()->setVisible(false);
	horizontalHeader()->setStretchLastSection(false); // Отключаем растягивание последней колонки
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
}

void ConsoleView::setModel(QAbstractItemModel *model)
{
	QTableView::setModel(model);
	setupColumnWidths();
}

void ConsoleView::setupColumnWidths()
{
	if (model() && model()->columnCount() >= 3)
	{
		setColumnWidth(0, 120);  // Колонка "Тип"
		setColumnWidth(1, 80);  // Колонка "Сообщение"
		setColumnWidth(2, 640);  // Колонка "Время"
	}
}

