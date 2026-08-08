#include "PointsDoc.h"

PointsDoc::PointsDoc(QObject *parent) : BaseDoc(parent)
{
}

void PointsDoc::clear()
{
	BaseDoc::clear();

	clearPoints();
}

void PointsDoc::clearPoints()
{
	m_points.clear();
}
