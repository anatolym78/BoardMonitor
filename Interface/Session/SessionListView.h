#ifndef SESSIONLISTVIEW_H
#define SESSIONLISTVIEW_H

#include <QTreeView>
#include <QModelIndex>
#include <QVector>

/**
 * @brief Виджет для отображения списка сессий.
 *
 * Отображает список доступных файлов записей и живых сессий.
 * Позволяет пользователю выбирать активную сессию для отображения в рабочей области.
 */
class SessionListView : public QTreeView
{
	Q_OBJECT
public:
	explicit SessionListView(QWidget *parent = nullptr);

	void createSelectionModel();
	void selectFirstItem();
	void selectFlatSessionIndex(int flatIndex);
	void clearSessionSelection();
	void revealFlatSessionIndex(int flatIndex);

signals:
	void sessionSelected(int sessionIndex);

private:
	void refreshIndexVisual(const QModelIndex& index);
	void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void onCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
	void onModelReset();
	void onRowsInserted(const QModelIndex& parent, int first, int last);
	void onRowsRemoved(const QModelIndex& parent, int first, int last);
	void onModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
};

#endif // SESSIONLISTVIEW_H
