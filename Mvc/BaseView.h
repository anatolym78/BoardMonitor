#pragma once

#include <QWidget>
#include "ui_BaseView.h"

#include "DocumentNavigator.h"

#include <QImage>
#include <QPointF>
#include <QRectF>

// View widget: zoom/scroll + forwards input/paint to Presenter (merged controller+renderer).

class Presenter;
class BaseView : public QWidget
{
	Q_OBJECT

public:
	BaseView(QWidget *parent = 0);
	virtual ~BaseView();

	const QImage& getImage() { return m_image; }

	void setPresenter(Presenter* p);
	Presenter* getPresenter();

	// zoom/pane
	void zoomNormal();
	void zoomFit();
	void zoomIn();
	void zoomOut();
	void zoomIn(QPointF p, double k);
	void zoomOut(QPointF p, double k);
	void moveImage(QPointF dp);
	void zoomNormal(QPointF p);
	void swapZoom(QPointF p);
	void setZoom(double zoom);
	void setZoom(double zoom, QPointF p);
	void setZoomRect(QRectF rect);
	bool isZoomFit();

	// events (redirect to presenter)
	virtual void paintEvent(QPaintEvent* pe);
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void wheelEvent(QWheelEvent* e);
	virtual void keyPressEvent (QKeyEvent* e);
	virtual void resizeEvent(QResizeEvent* e);
	virtual void leaveEvent(QEvent* e);

	void dragEnterEvent(QDragEnterEvent* e);
	void dragMoveEvent(QDragMoveEvent* e);
	void dropEvent(QDropEvent* e);

	double getZoom();
	QRectF getImageRect();
	void setImageRect(QRectF rect);
	QPointF getImageScrollPos();
	QSizeF getImageZoomedSize();

	void updateView();

signals:
	void dropFile(QStringList, QPoint);

public:
	QPointF _fromView(QPointF p) { return m_navigator.ToImage(p); }
	QPointF _toView(QPointF p) { return m_navigator.FromImage(p); }

	QPointF toView(QPointF p);
	QPointF fromView(QPointF p);
	QRectF fromView(QRectF rect);
	QRectF toView(QRectF rect);

signals:
	void currentImageCoords(const QPointF& p);

public slots:
	void onImageUpdated(const QImage&);

protected:
	QImage m_image;
	Presenter* m_presenter;
	DocumentNavigator m_navigator;
	bool m_bZoomFit;

	bool m_bStopRender;

private:
	Ui::BaseView ui;
};