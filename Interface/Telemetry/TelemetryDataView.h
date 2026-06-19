#ifndef PARAMETERSTREEVIEW_H
#define PARAMETERSTREEVIEW_H

#include <QTreeView>
#include <QSet>
#include <QString>

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
    void applyDefaultExpansion();

    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

signals:
    void itemHovered(ParameterTreeHistoryItem* treeItem);

private:
    void saveExpandedPaths();
    void restoreExpandedPaths();
    void collectExpandedPaths(const QModelIndex& parent, int depth);
    void restoreExpandedPathsRecursive(const QModelIndex& parent, int depth);

    QSet<QString> m_expandedPaths;
};

#endif // PARAMETERS