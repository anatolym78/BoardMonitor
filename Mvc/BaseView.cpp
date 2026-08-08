#include "BaseView.h"

#include "./Presenter.h"

#include <QDrag>
#include <QDragEnterEvent>
#include <QEvent>

#include <iostream>

BaseView::BaseView(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);

	m_presenter = 0;

	m_navigator.SetZoom(1);
	m_navigator.SetZoomRange(0.1f, 20.0f);

	this->setMouseTracking(true);

	m_bZoomFit = false;

	setAcceptDrops(true);

	m_bStopRender = false;
}

BaseView::~BaseView()
{
	if (m_presenter)
		delete m_presenter;
}

void BaseView::dragEnterEvent( QDragEnterEvent* e )
{
	//if(e->mimeData()->hasUrls())
	//{
	//	e->acceptProposedAction();
	//}
}

void BaseView::dragMoveEvent( QDragMoveEvent* )
{
	
}

void BaseView::dropEvent( QDropEvent* e )
{
	//QList<QUrl> urls = e->mimeData()->urls();
	//if(!urls.isEmpty())
	//{
	//	QStringList slist;
	//	for(int i=0;i<urls.count();i++)
	//	{
	//		QString str = urls[i].toLocalFile();
	//		if(str.indexOf(".bmp") != -1 || str.indexOf(".jpg") != -1 || str.indexOf(".png") != -1)
	//		{
	//			slist<<str;
	//		}
	//	}

	//	emit dropFile(slist, e->pos());
	//}
}

void BaseView::onImageUpdated(const QImage& img)
{
	if (!img.isNull())
	{
		m_navigator.SetNewImage(QSize(img.width(), img.height()));

		zoomFit();
	}
}

void BaseView::setPresenter(Presenter* p)
{
	if (!p)
	{
		return;
	}

	if (m_presenter)
		delete m_presenter;

	// Берём владение переданным экземпляром (template создаёт один Presenter на View)
	m_presenter = p;
	m_presenter->setView(this);
}

Presenter* BaseView::getPresenter()
{
	return m_presenter;
}

// events
void BaseView::paintEvent(QPaintEvent* )
{
	if (m_presenter)
	{
		m_presenter->draw();
	}
}

void BaseView::resizeEvent(QResizeEvent* e)
{
	m_navigator.ResizeView(QSize(width(), height()));

	zoomFit();

	QWidget::resizeEvent(e);
}

void BaseView::mousePressEvent(QMouseEvent* e)
{
	if (m_presenter)
	{
		m_presenter->mousePress(e);
	}

	QWidget::mousePressEvent(e);
}

void BaseView::mouseMoveEvent(QMouseEvent* e)
{
	if (!hasFocus())
	{
		setFocus();
	}
	if (m_presenter)
	{
		m_presenter->mouseMove(e);
	}

	QPoint p = e->pos();
	emit currentImageCoords(_fromView(QPointF(p)));

	QWidget::mouseMoveEvent(e);
}

void BaseView::mouseReleaseEvent(QMouseEvent* e)
{
	if (m_presenter)
	{
		m_presenter->mouseRelease(e);
	}

	QWidget::mouseReleaseEvent(e);
}

void BaseView::leaveEvent(QEvent* e)
{
	if (m_presenter)
	{
		m_presenter->mouseLeave();
	}
	QWidget::leaveEvent(e);
}

void BaseView::wheelEvent(QWheelEvent* e)
{
	if (m_presenter)
	{
		m_presenter->mouseWheel(e);
	}

	QWidget::wheelEvent(e);

	QPoint p = e->pos();
	emit currentImageCoords(_fromView(QPointF(p)));
}

void BaseView::keyPressEvent (QKeyEvent* e)
{
	if (m_presenter)
	{
		m_presenter->keyPress(e);
	}

	QWidget::keyPressEvent(e);
}

// zoom
void BaseView::zoomFit()
{
	m_navigator.SetZoomFit();
	m_bZoomFit = true;

    updateView();
}

bool BaseView::isZoomFit()
{
	return m_bZoomFit;
}

void BaseView::zoomNormal()
{
	m_navigator.SetZoomCenter(1.0);
	m_bZoomFit = false;

    updateView();
}

void BaseView::zoomIn()
{
	m_navigator.SetZoomCenter(getZoom()*1.1);
	m_bZoomFit = false;

    updateView();
}

void BaseView::zoomOut()
{
	m_navigator.SetZoomCenter(getZoom()/1.1);
	m_bZoomFit = false;

    updateView();
}

void BaseView::zoomIn(QPointF p, double k)
{
	m_navigator.SetZoom(getZoom() * k, p.toPoint());
	m_bZoomFit = false;

	updateView();
}

void BaseView::zoomOut(QPointF p, double k)
{
	m_navigator.SetZoom(getZoom() / k, p.toPoint());
	m_bZoomFit = false;

	updateView();
}

void BaseView::zoomNormal(QPointF p)
{
	m_navigator.SetZoomNormal(p.toPoint());
	m_bZoomFit = false;

	updateView();
}

void BaseView::swapZoom(QPointF p)
{
	if(m_navigator.GetZoom() != 1.0)
	{
		setZoom(1.0, p);
	}
	else
	{
		zoomFit();
	}
}

void BaseView::setZoom(double zoom)
{
	m_navigator.SetZoom(zoom);
	m_bZoomFit = false;

    updateView();
}

void BaseView::setZoom(double zoom, QPointF p)
{
	m_navigator.SetZoom(zoom, p.toPoint());
	m_bZoomFit = false;

	updateView();
}

void BaseView::setZoomRect(QRectF rect)
{
	if (rect.isNull())
	{
		zoomFit();

		return;
	}

	m_navigator.SetZoomRect(rect.toRect());
	m_bZoomFit = false;

	updateView();
}

void BaseView::moveImage(QPointF dp)
{
	m_navigator.Move(dp.toPoint());

	updateView();
}

double BaseView::getZoom()
{
	return m_navigator.GetZoom();
}

QPointF BaseView::getImageScrollPos()
{
	return QPointF(-m_navigator.GetPos().x(), -m_navigator.GetPos().y());
}

QSizeF BaseView::getImageZoomedSize()
{
	return QSizeF(m_navigator.GetSize());
}

void BaseView::updateView()
{
	return;

	QWidget::repaint();
}

QRectF BaseView::getImageRect()
{
	return QRectF(getImageScrollPos(), getImageZoomedSize());
}

void BaseView::setImageRect(QRectF)
{
}

QPointF BaseView::fromView(QPointF p)
{
	return m_navigator.ToImage(p);
}

QPointF BaseView::toView(QPointF p)
{
	return m_navigator.FromImage(p);
}

QRectF BaseView::fromView(QRectF rect)
{
	const QPointF p1 = _fromView(rect.topLeft());
	const QPointF p2 = _fromView(rect.bottomRight());
	return QRectF(p1, p2).normalized();
}

QRectF BaseView::toView(QRectF rect)
{
	const QPointF p1 = _toView(rect.topLeft());
	const QPointF p2 = _toView(rect.bottomRight());
	return QRectF(p1, p2).normalized();
}
