#ifndef PARAMETERSTREEVIEW_H
#define PARAMETERSTREEVIEW_H

#include <QTreeView>
#include "./../../Model/Parameters/Tree/ParameterTreeHistoryItem.h"

/**
 * @brief Виджет для просмотра данных телеметрии.
 * 
 * Отображает иерархическое дерево параметров (read-only).
 * Используется для мониторинга текущих значений и выбора параметров для построения графиков.
 */
class TelemetryDataView : public QTreeView
{
    Q_OBJECT
public:
    explicit TelemetryDataView(QWidget *parent = nullptr);
    void setModel(QAbstractItemModel* model) override;

    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

signals:
    void itemHovered(ParameterTreeHistoryItem* treeItem);
};

#endif // PARAMETERSTREEVIEW_H
