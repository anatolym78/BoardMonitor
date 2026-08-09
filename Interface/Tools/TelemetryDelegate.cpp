#include "TelemetryDelegate.h"
#include "./../../ViewModel/BoardParametersTreeModel.h"
#include "./../../Model/Parameters/Tree/ParameterTreeItem.h"
#include <QPainter>
#include <QApplication>
#include <QFontMetrics>

namespace
{
constexpr int kMarkerMaxSize = 14;
constexpr int kMarkerMarginLeft = 4;
constexpr int kMarkerGapAfter = 6;
}

TelemetryDelegate::TelemetryDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
{
}

int TelemetryDelegate::chartMarkerReserve()
{
	return kMarkerMarginLeft + kMarkerMaxSize + kMarkerGapAfter;
}

void TelemetryDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItem opt = option;
	initStyleOption(&opt, index);

	// 1. Рисуем фон (стандартный стиль)
	QStyleOptionViewItem optBackground = opt;
	optBackground.text = QString();
	QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &optBackground, painter, option.widget);

	// 2. Маркер: место всегда зарезервировано у History, рисуем только если график показан
	int offset = 0;
	auto* treeItem = static_cast<ParameterTreeItem*>(index.internalPointer());
	const bool isHistory = treeItem && treeItem->type() == ParameterTreeItem::ItemType::History;
	if (isHistory)
	{
		offset = chartMarkerReserve();
	}

	const bool isChartVisible = index.data(BoardParametersTreeModel::ChartVisibilityRole).toBool();
	if (isHistory && isChartVisible)
	{
		int size = opt.rect.height() - 8;
		if (size > kMarkerMaxSize) size = kMarkerMaxSize;
		if (size < 10) size = 10;

		QColor chartColor = index.data(BoardParametersTreeModel::ColorRole).value<QColor>();
		if (chartColor.isValid())
		{
			painter->save();
			painter->setRenderHint(QPainter::Antialiasing);

			const int top = opt.rect.center().y() - size / 2;
			const QRect markerRect(opt.rect.left() + kMarkerMarginLeft, top, size, size);

			painter->setBrush(chartColor);
			painter->setPen(Qt::NoPen);
			painter->drawRoundedRect(markerRect, 2, 2);

			painter->restore();
		}
	}

	// 3. Текст справа от резерва маркера
	if (!opt.text.isEmpty())
	{
		QRect textRect = opt.rect;
		textRect.setLeft(textRect.left() + offset);

		QApplication::style()->drawItemText(painter, textRect, opt.displayAlignment, opt.palette,
			opt.state & QStyle::State_Enabled, opt.text);
	}
}

QSize TelemetryDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QSize size = QStyledItemDelegate::sizeHint(option, index);

	auto* treeItem = static_cast<ParameterTreeItem*>(index.internalPointer());
	if (treeItem && treeItem->type() == ParameterTreeItem::ItemType::History)
	{
		// Колонка значений Stretch: не раздувать ширину дерева из‑за "live (scrub)"
		if (index.column() == 1)
		{
			QFontMetrics fm(option.font);
			const int typical = fm.horizontalAdvance(QStringLiteral("0000.0000")) + chartMarkerReserve();
			size.setWidth(typical);
		}
		else
		{
			size.setWidth(size.width() + chartMarkerReserve());
		}
	}

	return size;
}
