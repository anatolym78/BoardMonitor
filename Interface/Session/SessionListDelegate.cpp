#include "SessionListDelegate.h"
#include "../../ViewModel/SessionsListModel.h"

#include <QPainter>

static constexpr int kSessionRowHeight = 44;
static constexpr int kIconSize = 24;
static constexpr int kMarginH = 6;
static constexpr qreal kSelectedFontScale = 1.12;

static void fillRowBackground(QPainter* painter, const QRect& rect,
	const QStyleOptionViewItem& option)
{
	painter->fillRect(rect, option.palette.base());

	if (option.state & QStyle::State_Selected)
	{
		// Лёгкий оттенок вместо насыщенного QPalette::Highlight
		painter->fillRect(rect, QColor(33, 150, 243, 55));
	}
	else if (option.state & QStyle::State_MouseOver)
	{
		painter->fillRect(rect, QColor(0, 0, 0, 18));
	}
}

SessionListDelegate::SessionListDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
{}

QSize SessionListDelegate::sizeHint(const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const
{
	Q_UNUSED(option)
	Q_UNUSED(index)
	return QSize(0, kSessionRowHeight);
}

void SessionListDelegate::paint(QPainter* painter,
                                 const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const
{
	painter->save();

	const QRect rect = option.rect;
	fillRowBackground(painter, rect, option);

	const bool isFolder = index.data(SessionsListModel::IsDayFolderRole).toBool();

	const int savePct = index.data(SessionsListModel::SaveProgressRole).toInt();
	const int loadPct = index.data(SessionsListModel::LoadProgressRole).toInt();
	const int pct = (savePct >= 0) ? savePct : loadPct;

	if (!isFolder && pct >= 0)
	{
		const QColor fillColor = (savePct >= 0)
			? QColor(76, 175, 80, 160)
			: QColor(33, 150, 243, 160);

		painter->fillRect(rect, QColor(210, 210, 210, 180));
		const int filledWidth = rect.width() * pct / 100;
		if (filledWidth > 0)
		{
			painter->fillRect(QRect(rect.left(), rect.top(), filledWidth, rect.height()), fillColor);
		}

		painter->setPen(Qt::black);
		painter->drawText(rect, Qt::AlignCenter, QString("%1%").arg(pct));
		painter->restore();
		return;
	}

	const QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
	if (!icon.isNull())
	{
		const int iconY = rect.top() + (rect.height() - kIconSize) / 2;
		painter->drawPixmap(rect.left() + kMarginH, iconY, icon.pixmap(kIconSize, kIconSize));
	}

	const int textX = rect.left() + kMarginH + kIconSize + kMarginH;
	const int textW = rect.right() - textX - kMarginH;

	const bool selected = option.state & QStyle::State_Selected;
	const QColor primaryColor = option.palette.color(QPalette::WindowText);
	QColor secondaryColor = primaryColor;
	secondaryColor.setAlpha(selected ? 180 : 130);

	if (isFolder)
	{
		QFont folderFont = option.font;
		folderFont.setBold(true);
		if (selected)
		{
			folderFont.setPointSizeF(folderFont.pointSizeF() * kSelectedFontScale);
		}
		painter->setFont(folderFont);
		painter->setPen(primaryColor);
		const QString folderLabel = index.data(SessionsListModel::SessionNameRole).toString();
		painter->drawText(rect.adjusted(textX, 0, -kMarginH, 0),
			Qt::AlignVCenter | Qt::AlignLeft, folderLabel);
		painter->restore();
		return;
	}

	const bool isLive = index.data(SessionsListModel::IsLiveSessionRole).toBool();
	const int halfH = rect.height() / 2;

	QFont primaryFont = option.font;
	primaryFont.setBold(true);
	if (selected)
	{
		primaryFont.setPointSizeF(primaryFont.pointSizeF() * kSelectedFontScale);
	}
	painter->setFont(primaryFont);
	painter->setPen(primaryColor);

	QString primaryText;
	QString secondaryText;

	if (isLive)
	{
		primaryText = index.data(SessionsListModel::SessionNameRole).toString();
		const QString date = index.data(SessionsListModel::CreatedAtFormattedRole).toString();
		const int msgCnt = index.data(SessionsListModel::MessageCountRole).toInt();
		secondaryText = date.isEmpty()
			? QString("%1 msgs").arg(msgCnt)
			: QString("%1 | %2 msgs").arg(date).arg(msgCnt);
	}
	else
	{
		primaryText = index.data(SessionsListModel::CreatedAtFormattedRole).toString();
		const int msgCnt = index.data(SessionsListModel::MessageCountRole).toInt();
		secondaryText = QString("%1 msgs").arg(msgCnt);
	}

	painter->drawText(QRect(textX, rect.top(), textW, halfH),
		Qt::AlignVCenter | Qt::AlignLeft, primaryText);

	QFont subFont = option.font;
	subFont.setPointSizeF(subFont.pointSizeF() * 0.85);
	if (selected)
	{
		subFont.setPointSizeF(subFont.pointSizeF() * kSelectedFontScale);
	}
	painter->setFont(subFont);
	painter->setPen(secondaryColor);
	painter->drawText(QRect(textX, rect.top() + halfH, textW, halfH),
		Qt::AlignVCenter | Qt::AlignLeft, secondaryText);

	painter->restore();
}
