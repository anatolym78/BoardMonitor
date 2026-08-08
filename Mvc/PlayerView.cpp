#include "PlayerView.h"

#include <QResizeEvent>
#include <QSizePolicy>

PlayerView::PlayerView(QWidget* parent)
	: BaseView(parent)
{
	setFixedHeight(25);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setMinimumWidth(50);
	setAutoFillBackground(false);
	setAttribute(Qt::WA_OpaquePaintEvent, false);
	setAttribute(Qt::WA_NoSystemBackground, true);
}

void PlayerView::resizeEvent(QResizeEvent* e)
{
	// Без zoomFit: шкала 1D, координаты считаем во float в Presenter
	QWidget::resizeEvent(e);
	update();
}
