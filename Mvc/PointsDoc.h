#pragma once

#include "./BaseDoc.h"

#include <QList>
#include <QPointF>

class PointsDoc : public BaseDoc
{
	Q_OBJECT

public:
	PointsDoc(QObject *parent);
	~PointsDoc() {}

	virtual void clear() override;

	void setPoints(const QList<QPointF>& points) { m_points = points; }

	QList<QPointF> points() const { return m_points; }

	void clearPoints();

protected:
	virtual void processCore() override {}

private:
	QList<QPointF> m_points;
};
