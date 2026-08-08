#pragma once

#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QSize>

// Transforms coordinates between view and content (zoom / scroll).
class DocumentNavigator
{
public:
	QPoint GetNewPos();
	QSize GetNewSize();

	DocumentNavigator();
	DocumentNavigator(QSize isize, QSize wsize);
	void SetNewImage(QSize isize);
	void ResizeView(QSize wsize);
	void SetZoomRange(float zmin, float zmax);

	QPoint Move(QPoint dp);
	QPoint SetZoom(float zoom);
	QPoint SetZoomCenter(float zoom);
	QPoint SetZoom(float zoom, QPoint p);
	QPoint SetZoomNormal(QPoint p);
	QPoint SetZoomFit();
	QPoint SetZoomRect(QRect rect);

	QPoint GetPos();
	QSize GetSize();
	float GetZoom();
	float GetFitZoom();
	QSize GetViewSize();
	QRect GetViewRect();

	QSize GetNormalSize();
	QRect GetRect();
	QRect GetCrossRect();
	QSize GetCrossSize();
	float GetMinZoom();
	float GetMaxZoom();
	QPoint CorrectPos();

	QPointF ToImage(QPointF p);
	QPointF FromImage(QPointF p);

protected:
	QPoint m_ScrollPos;
	QSize m_ImgSize;
	QSize m_ViewSize;
	float m_Zoom;
	float m_MinZoom;
	float m_MaxZoom;

	float CorrectZoom(float zoom);

	QPointF cur;
	QPointF p;
	QPointF isize;
	QPointF wsize;
	double z;
};
