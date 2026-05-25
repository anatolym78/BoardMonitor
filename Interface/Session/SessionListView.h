#ifndef SESSIONLISTVIEW_H
#define SESSIONLISTVIEW_H

#include <QListView>

/**
 * @brief Виджет для отображения списка сессий.
 * 
 * Отображает список доступных файлов записей и живых сессий.
 * Позволяет пользователю выбирать активную сессию для отображения в рабочей области.
 */
class SessionListView : public QListView
{
	Q_OBJECT
public:
	explicit SessionListView(QWidget *parent = nullptr);

public:
	void createSelectionModel();
	void selectFirstItem();

signals:
	void sessionSelected(int sessionIndex);

private:
	void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
};

#endif // SESSIONLISTVIEW_H

