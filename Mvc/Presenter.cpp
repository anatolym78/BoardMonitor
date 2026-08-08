#include "Presenter.h"

#include "BaseDoc.h"
#include "BaseView.h"

#include <QPainter>

Presenter::Presenter()
{
}

Presenter::Presenter(BaseDoc* pdoc)
{
	setDocument(pdoc);
}

void Presenter::setView(BaseView* pview)
{
	m_pView = pview;
}

void Presenter::setDocument(BaseDoc* pdoc)
{
	m_pDoc = pdoc;
}

Presenter* Presenter::clone()
{
	auto* p = new Presenter();
	*p = *this;
	return p;
}

bool Presenter::isValid()
{
	return m_pView && m_pDoc;
}

void Presenter::mousePress(QMouseEvent*)
{
}

void Presenter::mouseMove(QMouseEvent*)
{
}

void Presenter::mouseRelease(QMouseEvent*)
{
}

void Presenter::mouseWheel(QWheelEvent*)
{
}

void Presenter::mouseLeave()
{
}

void Presenter::keyPress(QKeyEvent*)
{
}

void Presenter::draw()
{
	if (!isValid())
	{
		return;
	}

	QPainter painter;
	painter.begin(getView());
	painter.setRenderHint(QPainter::Antialiasing, true);
	drawCore(painter);
	painter.end();
}

void Presenter::drawCore(QPainter& painter)
{
	drawBackground(painter, QColor(215, 215, 215));
	drawImage(painter);
}

void Presenter::drawBackground(QPainter& painter, QColor color)
{
	painter.setBrush(QBrush(color));
	painter.setPen(color);
	painter.drawRect(QRect(0, 0, getView()->width(), getView()->height()));
}

void Presenter::drawImage(QPainter& painter)
{
	const QImage& img = getDoc()->getImage();
	const QRectF irect = getView()->getImageRect();
	painter.drawImage(irect, img, QRect(0, 0, img.width(), img.height()));
}

QPointF Presenter::fromView(QPointF p)
{
	if (m_pView)
	{
		return getView()->fromView(p);
	}
	return QPointF(0, 0);
}

QPointF Presenter::toView(QPointF p)
{
	if (m_pView)
	{
		return getView()->toView(p);
	}
	return QPointF();
}

float Presenter::getZoom()
{
	if (m_pView)
	{
		return static_cast<float>(getView()->getZoom());
	}
	return 1.0f;
}
