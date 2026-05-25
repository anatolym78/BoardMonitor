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
};

#endif // VALUECOLUMNDELEGATE_H

