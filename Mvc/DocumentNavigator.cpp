#include "DocumentNavigator.h"

#include <algorithm>

#undef max
#undef min

namespace
{
const float cMaxZoom = 100.0f;
const float cMinZoom = 0.01f;
}

QPoint DocumentNavigator::GetNewPos()
{
	const QPointF c = cur;
	QPoint pos(0, 0);
	if (isize.x() * z > wsize.x())
	{
		const double x1 = p.x() + c.x();
		const double x2 = x1 * z;
		pos.setX(int(x2 - c.x()));
	}
	if (isize.y() * z > wsize.y())
	{
		const double y1 = p.y() + c.y();
		const double y2 = y1 * z;
		pos.setY(int(y2 - c.y()));
	}
	if (pos.x() < 0) pos.setX(0);
	if (pos.y() < 0) pos.setY(0);

	return pos;
}

QSize DocumentNavigator::GetNewSize()
{
	return QSize(int(isize.x() * z), int(isize.y() * z));
}

DocumentNavigator::DocumentNavigator()
{
	m_ScrollPos = QPoint(0, 0);
	m_ImgSize = QSize(640, 480);
	m_ViewSize = QSize(640, 480);
	m_Zoom = 1.0f;
	m_MinZoom = 0.1f;
	m_MaxZoom = 100.0f;
}

DocumentNavigator::DocumentNavigator(QSize isize, QSize wsize)
{
	m_ScrollPos = QPoint(0, 0);
	m_ImgSize = isize;
	m_ViewSize = wsize;
	m_Zoom = 1.0f;
	m_MinZoom = 0.1f;
	m_MaxZoom = 100.0f;
}

void DocumentNavigator::SetNewImage(QSize isize)
{
	m_ScrollPos = QPoint(0, 0);
	m_ImgSize = isize;
	m_Zoom = 1.0f;
}

void DocumentNavigator::ResizeView(QSize wsize)
{
	m_ViewSize = wsize;
	CorrectPos();
}

void DocumentNavigator::SetZoomRange(float zmin, float zmax)
{
	m_MinZoom = std::max(cMinZoom, zmin);
	m_MaxZoom = std::min(cMaxZoom, zmax);
}

QPoint DocumentNavigator::Move(QPoint dp)
{
	m_ScrollPos -= dp;
	CorrectPos();
	return m_ScrollPos;
}

QPoint DocumentNavigator::SetZoom(float zoom)
{
	return SetZoom(zoom, QPoint(0, 0));
}

QPoint DocumentNavigator::SetZoomCenter(float zoom)
{
	return SetZoom(zoom, QPoint(m_ViewSize.width() / 2, m_ViewSize.height() / 2));
}

QPoint DocumentNavigator::SetZoom(float zoom, QPoint cp)
{
	zoom = CorrectZoom(zoom);
	if (zoom < GetMinZoom())
		zoom = GetMinZoom();
	if (zoom > GetMaxZoom())
		zoom = GetMaxZoom();

	QPoint pos(0, 0);
	const QPoint p = m_ScrollPos;
	const QSize isize = m_ImgSize;
	const QSize wsize = m_ViewSize;
	const float z = zoom;
	if (isize.width() * z > wsize.width())
	{
		pos.setX(int((cp.x() + p.x()) * zoom / m_Zoom - cp.x()));
	}
	if (isize.height() * z > wsize.height())
	{
		pos.setY(int((cp.y() + p.y()) * zoom / m_Zoom - cp.y()));
	}

	m_Zoom = zoom;
	m_ScrollPos = pos;

	return CorrectPos();
}

QPoint DocumentNavigator::SetZoomNormal(QPoint p)
{
	return SetZoom(1.0f, p);
}

QPoint DocumentNavigator::SetZoomFit()
{
	m_Zoom = GetFitZoom();
	const QSize isize = m_ImgSize;
	const QSize wsize = m_ViewSize;
	const float z = m_Zoom;
	QPoint pos(0, 0);
	if (isize.width() * z < wsize.width())
	{
		pos.setX(int((wsize.width() - isize.width() * z) / 2));
	}
	if (isize.height() * z < wsize.height())
	{
		pos.setY(int((wsize.height() - isize.height() * z) / 2));
	}

	m_ScrollPos = -pos;

	return m_ScrollPos;
}

float DocumentNavigator::GetFitZoom()
{
	const int px = GetNormalSize().width();
	const int py = GetNormalSize().height();
	const int cx = m_ViewSize.width();
	const int cy = m_ViewSize.height();
	const int dx = px - cx;
	const int dy = py - cy;
	float k = 1.0f;
	const float kx = float(cx) / px;
	const float ky = float(cy) / py;
	const bool grow = (dx <= 0) && (dy <= 0);
	if (grow)
	{
		k = std::min(kx, ky);
	}
	else
	{
		if (dx > 0 && dy > 0)
		{
			k = std::min(kx, ky);
		}
		else
		{
			if (dx > 0) k = kx;
			if (dy > 0) k = ky;
		}
	}

	return k;
}

QPoint DocumentNavigator::SetZoomRect(QRect rect)
{
	const QRect wrect = GetViewRect();
	const int w1 = wrect.width();
	const int h1 = wrect.height();
	const int w2 = rect.width();
	const int h2 = rect.height();
	const float z = std::min(float(w1) / w2, float(h1) / h2);
	const QPoint cursor(rect.center());
	SetZoom(GetZoom() * z, cursor);

	return m_ScrollPos;
}

QPoint DocumentNavigator::GetPos()
{
	return m_ScrollPos;
}

QSize DocumentNavigator::GetSize()
{
	return QSize(int(m_ImgSize.width() * m_Zoom), int(m_ImgSize.height() * m_Zoom));
}

QSize DocumentNavigator::GetNormalSize()
{
	return m_ImgSize;
}

QRect DocumentNavigator::GetRect()
{
	return QRect(-m_ScrollPos.x(), -m_ScrollPos.y(),
		int(m_ImgSize.width() * m_Zoom), int(m_ImgSize.height() * m_Zoom));
}

QRect DocumentNavigator::GetCrossRect()
{
	return QRect(0, 0, GetCrossSize().width(), GetCrossSize().height());
}

QSize DocumentNavigator::GetCrossSize()
{
	const int cx = std::min(m_ViewSize.width(), GetSize().width() - GetPos().x());
	const int cy = std::min(m_ViewSize.height(), GetSize().height() - GetPos().y());

	return QSize(cx, cy);
}

QSize DocumentNavigator::GetViewSize()
{
	return m_ViewSize;
}

QRect DocumentNavigator::GetViewRect()
{
	return QRect(0, 0, m_ViewSize.width(), m_ViewSize.height());
}

float DocumentNavigator::GetZoom()
{
	return m_Zoom;
}

float DocumentNavigator::GetMinZoom()
{
	return m_MinZoom;
}

float DocumentNavigator::GetMaxZoom()
{
	return m_MaxZoom;
}

QPointF DocumentNavigator::ToImage(QPointF p)
{
	const QPointF p2(p.x() + GetPos().x(), p.y() + GetPos().y());
	return QPointF(p2.x() / GetZoom(), p2.y() / GetZoom());
}

QPointF DocumentNavigator::FromImage(QPointF p)
{
	const QPointF p2(p.x() * GetZoom(), p.y() * GetZoom());
	return QPointF(p2.x() - GetPos().x(), p2.y() - GetPos().y());
}

QPoint DocumentNavigator::CorrectPos()
{
	const QSize is = GetSize();
	if (is.width() - m_ScrollPos.x() <= m_ViewSize.width())
	{
		m_ScrollPos.setX(is.width() - m_ViewSize.width());
	}
	if (is.height() - m_ScrollPos.y() <= m_ViewSize.height())
	{
		m_ScrollPos.setY(is.height() - m_ViewSize.height());
	}

	if (m_ScrollPos.x() < 0) m_ScrollPos.setX(0);
	if (m_ScrollPos.y() < 0) m_ScrollPos.setY(0);

	return m_ScrollPos;
}

float DocumentNavigator::CorrectZoom(float zoom)
{
	if (zoom < GetMinZoom())
		zoom = GetMinZoom();
	if (zoom > GetMaxZoom())
		zoom = GetMaxZoom();

	return zoom;
}
