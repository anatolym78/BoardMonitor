#include "TelemetryDelegate.h"
#include "./../../ViewModel/BoardParametersTreeModel.h"
#include <QPainter>
#include <QApplication>

TelemetryDelegate::TelemetryDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
{
}

void TelemetryDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItem opt = option;
	initStyleOption(&opt, index);

	// 1. Рисуем фон (стандартный стиль)
	// Создаем копию опций и убираем текст, чтобы drawControl нарисовал только фон и выделение
	QStyleOptionViewItem optBackground = opt;
	optBackground.text = QString(); 
	QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &optBackground, painter, option.widget);

	// 2. Маркер (Квадрат)
	bool isChartVisible = index.data(BoardParametersTreeModel::ChartVisibilityRole).toBool();
	int offset = 0;

	if (isChartVisible)
	{
		// Рассчитываем размер квадрата
		int size = opt.rect.height() - 8; // Отступы сверху и снизу по 4px
		if (size > 14) size = 14;         // Максимальный размер
		if (size < 10) size = 10;         // Минимальный размер

		int marginLeft = 4;               // Отступ слева
		offset = marginLeft + size + 6;   // Смещение для текста (маркер + отступ)

		QColor chartColor = index.data(BoardParametersTreeModel::ColorRole).value<QColor>();

		if (chartColor.isValid())
		{
			painter->save();
			painter->setRenderHint(QPainter::Antialiasing);

			// Центрируем квадрат по вертикали
			int top = opt.rect.center().y() - size / 2;
			QRect markerRect(opt.rect.left() + marginLeft, top, size, size);

			// Рисуем
			painter->setBrush(chartColor);
			painter->setPen(Qt::NoPen);
			painter->drawRoundedRect(markerRect, 2, 2); // Скругленные углы

			painter->restore();
		}
	}

	// 3. Рисуем текст (смещение вправо, если есть маркер)
	if (!opt.text.isEmpty())
	{
		QRect textRect = opt.rect;
		textRect.setLeft(textRect.left() + offset); // Сдвигаем область отрисовки текста

		QApplication::style()->drawItemText(painter, textRect, opt.displayAlignment, opt.palette,
			opt.state & QStyle::State_Enabled, opt.text);
	}
}
