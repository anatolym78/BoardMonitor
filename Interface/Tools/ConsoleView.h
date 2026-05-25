#ifndef DEBUGVIEW_H
#define DEBUGVIEW_H

#include <QTableView>

class QAbstractItemModel;

/**
 * @brief Виджет консоли/лога.
 * 
 * Отображает список системных сообщений, логов драйвера и ошибок.
 */
class ConsoleView : public QTableView
{
	Q_OBJECT
public:
	explicit ConsoleView(QWidget *parent = nullptr);
	
	void setModel(QAbstractItemModel *model) override;

private:
	void setupColumnWidths();
};

#endif // DEBUGVIEW_H

