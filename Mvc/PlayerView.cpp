#include "PlayerView.h"

#include <QResizeEvent>
#include <QSizePolicy>

PlayerView::PlayerView(QWidget* parent)
	: BaseView(parent)
{
	setFixedHeight(25);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setMinimumWidth(50);
}

void PlayerView::resizeEvent(QResizeEvent* e)
{
	// Без zoomFit: шкала 1D, координаты считаем во float в Presenter
	QWidget::resizeEvent(e);
	update();
}
