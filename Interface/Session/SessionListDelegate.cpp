#include "SessionListDelegate.h"
#include "../../ViewModel/SessionsListModel.h"

#include <QPainter>
#include <QApplication>
#include <QStyle>

static constexpr int kRowHeight = 56;
static constexpr int kIconSize  = 32;
static constexpr int kMarginH   = 6;
static constexpr qreal kSelectedFontScale = 1.12;

SessionListDelegate::SessionListDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
{}

QSize SessionListDelegate::sizeHint(const QStyleOptionViewItem& /*option*/,
                                     const QModelIndex& /*index*/) const
{
	return QSize(0, kRowHeight);
}

void SessionListDelegate::paint(QPainter* painter,
                                 const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const
{
	painter->save();

	// Draw background and hover/selection state; suppress icon, text, and focus rect
	QStyleOptionViewItem opt = option;
	initStyleOption(&opt, index);
	opt.text.clear();
	opt.icon = QIcon();
	opt.state &= ~QStyle::State_HasFocus;
	QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);

	const QRect rect = option.rect;

	// Progress bar overlay (save / load)
	const int savePct = index.data(SessionsListModel::SaveProgressRole).toInt();
	const int loadPct = index.data(SessionsListModel::LoadProgressRole).toInt();
	const int pct     = (savePct >= 0) ? savePct : loadPct;

	if (pct >= 0)
	{
		const QColor fillColor = (savePct >= 0)
			? QColor(76, 175, 80, 160)
			: QColor(33, 150, 243, 160);

		painter->fillRect(rect, QColor(210, 210, 210, 180));
		const int filledWidth = rect.width() * pct / 100;
		if (filledWidth > 0)
			painter->fillRect(QRect(rect.left(), rect.top(), filledWidth, rect.height()), fillColor);

		painter->setPen(Qt::black);
		painter->drawText(rect, Qt::AlignCenter, QString("%1%").arg(pct));
		painter->restore();
		return;
	}

	// Icon
	const QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
	if (!icon.isNull())
	{
		const int iconY = rect.top() + (rect.height() - kIconSize) / 2;
		painter->drawPixmap(rect.left() + kMarginH, iconY, icon.pixmap(kIconSize, kIconSize));
	}

	const int textX = rect.left() + kMarginH + kIconSize + kMarginH;
	const int textW = rect.right() - textX - kMarginH;
	const int halfH = rect.height() / 2;

	const bool selected = option.state & QStyle::State_Selected;

	const QColor primaryColor = option.palette.color(QPalette::WindowText);
	QColor secondaryColor = primaryColor;
	secondaryColor.setAlpha(selected ? 180 : 130);

	// Line 1 - session name (bold)
	QFont nameFont = option.font;
	nameFont.setBold(true);
	if (selected)
	{
		nameFont.setPointSizeF(nameFont.pointSizeF() * kSelectedFontScale);
	}
	painter->setFont(nameFont);
	painter->setPen(primaryColor);
	const QString name = index.data(SessionsListModel::SessionNameRole).toString();
	painter->drawText(QRect(textX, rect.top(), textW, halfH),
	                  Qt::AlignVCenter | Qt::AlignLeft, name);

	// Line 2 - date and message count
	QFont subFont = option.font;
	subFont.setPointSizeF(subFont.pointSizeF() * 0.85);
	if (selected)
	{
		subFont.setPointSizeF(subFont.pointSizeF() * kSelectedFontScale);
	}
	painter->setFont(subFont);
	painter->setPen(secondaryColor);

	const QString date   = index.data(SessionsListModel::CreatedAtFormattedRole).toString();
	const int     msgCnt = index.data(SessionsListModel::MessageCountRole).toInt();
	const QString subText = date.isEmpty()
		? QString("%1 msgs").arg(msgCnt)
		: QString("%1 | %2 msgs").arg(date).arg(msgCnt);

	painter->drawText(QRect(textX, rect.top() + halfH, textW, halfH),
	                  Qt::AlignVCenter | Qt::AlignLeft, subText);

	painter->restore();
}
