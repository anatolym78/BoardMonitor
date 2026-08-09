#ifndef VALUECOLUMNDELEGATE_H
#define VALUECOLUMNDELEGATE_H

#include <QStyledItemDelegate>

/**
 * @brief Делегат для отображения значений телеметрии.
 * 
 * Отвечает за кастомную отрисовку значений в TelemetryDataView,
 * включая отрисовку цветовых маркеров (легенды графиков) рядом со значениями.
 */
class TelemetryDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	explicit TelemetryDelegate(QObject *parent = nullptr);

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

	/** Место под квадрат легенды + отступы (всегда резервируем у History). */
	static int chartMarkerReserve();
};

#endif // VALUECOLUMNDELEGATE_H

