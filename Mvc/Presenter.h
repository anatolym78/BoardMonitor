#pragma once

#include <QColor>
#include <QPoint>
#include <QPointF>

class QKeyEvent;
class QMouseEvent;
class QPainter;
class QWheelEvent;

class BaseDoc;
class BaseView;

/**
 * @brief Presenter = бывшие InputController + DocRenderer.
 *
 * Один класс обрабатывает ввод и рисует документ во View
 * (как Nias.Mvp.Presenter: OnMouse* + Draw).
 */
class Presenter
{
public:
	Presenter();
	explicit Presenter(BaseDoc* pdoc);
	virtual ~Presenter() {}

	virtual void setView(BaseView* pview);
	virtual void setDocument(BaseDoc* pdoc);
	virtual Presenter* clone();
	virtual bool isValid();

	// --- input ---
	virtual void mousePress(QMouseEvent* e);
	virtual void mouseMove(QMouseEvent* e);
	virtual void mouseRelease(QMouseEvent* e);
	virtual void mouseWheel(QWheelEvent* e);
	virtual void mouseLeave();
	virtual void keyPress(QKeyEvent* e);

	// --- render ---
	virtual void draw();

protected:
	virtual void drawCore(QPainter& painter);
	void drawBackground(QPainter& painter, QColor color);
	void drawImage(QPainter& painter);

	virtual QPointF fromView(QPointF p);
	virtual QPointF toView(QPointF p);
	float getZoom();

	BaseView* getView() { return m_pView; }
	const BaseView* getView() const { return m_pView; }
	BaseDoc* getDoc() { return m_pDoc; }
	const BaseDoc* getDoc() const { return m_pDoc; }

protected:
	BaseView* m_pView = nullptr;
	BaseDoc* m_pDoc = nullptr;

	QPoint m_mouseStart;
	QPoint m_mousePrev;
	QPoint m_mouseCurr;
	bool m_bNeedUpdate = false;
};
