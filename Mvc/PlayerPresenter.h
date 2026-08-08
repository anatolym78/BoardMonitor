#pragma once

#include "Presenter.h"
#include "PlayerDocument.h"

class PlayerPresenter : public Presenter
{
public:
	PlayerPresenter();
	explicit PlayerPresenter(PlayerDocument* doc);
	~PlayerPresenter() override;

	Presenter* clone() override;

	void mousePress(QMouseEvent* e) override;
	void mouseMove(QMouseEvent* e) override;
	void mouseRelease(QMouseEvent* e) override;
	void mouseLeave() override;

protected:
	void drawCore(QPainter& painter) override;

private:
	PlayerDocument* playerDoc() { return static_cast<PlayerDocument*>(getDoc()); }
	double timeToX(double seconds, double duration, double width) const;
	double xToTime(double x, double duration, double width) const;
	double baseRadius() const;
	double drawRadius() const;
	bool hitCursor(const QPointF& pos) const;
	void updateHover(const QPointF& pos);
	void scrubToX(double x);
	void refreshView();

	bool m_hover = false;
	bool m_dragging = false;
};
