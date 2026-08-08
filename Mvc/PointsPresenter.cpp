#include "PointsPresenter.h"

#include "BaseView.h"

#include <QPainter>

PointsPresenter::PointsPresenter()
{
}

PointsPresenter::PointsPresenter(PointsDoc* pdoc)
	: Presenter(pdoc)
{
}

PointsPresenter::~PointsPresenter()
{
}

Presenter* PointsPresenter::clone()
{
	auto* p = new PointsPresenter();
	*p = *this;
	return p;
}

void PointsPresenter::drawCore(QPainter& painter)
{
	Presenter::drawCore(painter);
	drawPoints(painter);
}

void PointsPresenter::drawPoints(QPainter& painter)
{
	Q_UNUSED(m_bShowPoints);
	drawCrosses(painter);
}

void PointsPresenter::drawCircles(QPainter& painter)
{
	painter.setBrush(QBrush(QColor(255, 16, 16)));
	painter.setPen(QPen(QColor(225, 225, 225), 1));

	const auto points = pointsDoc()->points();
	for (int i = 0; i < points.size(); ++i)
	{
		const auto p = getView()->toView(points[i]);
		painter.drawEllipse(p, 4, 4);
	}
}

void PointsPresenter::drawCrosses(QPainter& painter)
{
	painter.setPen(QPen(QColor(255, 0, 0), 2));

	const auto points = pointsDoc()->points();
	const int ds = 4;
	const auto dx = QPointF(ds, 0);
	const auto dy = QPointF(0, ds);
	for (int i = 0; i < points.size(); ++i)
	{
		const auto p = getView()->toView(points[i]);
		painter.drawLine(p - dx, p + dx);
		painter.drawLine(p - dy, p + dy);
	}
}
