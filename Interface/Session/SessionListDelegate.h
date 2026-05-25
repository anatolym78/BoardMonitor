#ifndef SESSIONLISTDELEGATE_H
#define SESSIONLISTDELEGATE_H

#include <QStyledItemDelegate>

/**
 * @brief Делегат для SessionListView.
 *
 * Рисует двухстрочную строку сессии: первая строка — название (жирным),
 * вторая — дата и количество сообщений. При активном сохранении/загрузке
 * (SaveProgressRole / LoadProgressRole >= 0) отображает полосу прогресса.
 */
class SessionListDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	explicit SessionListDelegate(QObject* parent = nullptr);

	void paint(QPainter* painter,
	           const QStyleOptionViewItem& option,
	           const QModelIndex& index) const override;

	QSize sizeHint(const QStyleOptionViewItem& option,
	               const QModelIndex& index) const override;
};

#endif // SESSIONLISTDELEGATE_H
