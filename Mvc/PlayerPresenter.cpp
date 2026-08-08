#include "PlayerPresenter.h"

#include "BaseView.h"

#include <QMouseEvent>
#include <QPainter>

namespace
{
const QColor kTrackColor(60, 60, 60);
const QColor kCursorBarColor(46, 160, 67);
const QColor kPlayheadBarColor(120, 200, 130);
const QColor kCursorColor(20, 90, 40);
const double kHoverScale = 1.2;
}

PlayerPresenter::PlayerPresenter()
{
}

PlayerPresenter::PlayerPresenter(PlayerDocument* doc)
	: Presenter(doc)
{
}

PlayerPresenter::~PlayerPresenter()
{
}

Presenter* PlayerPresenter::clone()
{
	auto* p = new PlayerPresenter();
	*p = *this;
	return p;
}

double PlayerPresenter::timeToX(double seconds, double duration, double width) const
{
	if (duration <= 0.0 || width <= 0.0)
	{
		return 0.0;
	}
	return (seconds / duration) * width;
}

double PlayerPresenter::xToTime(double x, double duration, double width) const
{
	if (duration <= 0.0 || width <= 0.0)
	{
		return 0.0;
	}
	return qBound(0.0, (x / width) * duration, duration);
}

double PlayerPresenter::baseRadius() const
{
	const auto* view = getView();
	if (!view)
	{
		return 6.0;
	}
	return qMin(view->height() * 0.35, 7.0);
}

double PlayerPresenter::drawRadius() const
{
	const double r = baseRadius();
	return (m_hover || m_dragging) ? r * kHoverScale : r;
}

bool PlayerPresenter::hitCursor(const QPointF& pos) const
{
	const auto* view = getView();
	const auto* doc = static_cast<const PlayerDocument*>(getDoc());
	if (!view || !doc)
	{
		return false;
	}

	const double cursorX = timeToX(doc->cursorSeconds(), doc->durationSeconds(), view->width());
	const QPointF center(cursorX, view->height() * 0.5);
	const double hitR = baseRadius() * kHoverScale + 4.0;
	const QPointF d = pos - center;
	return (d.x() * d.x() + d.y() * d.y()) <= hitR * hitR;
}

void PlayerPresenter::updateHover(const QPointF& pos)
{
	const bool hover = hitCursor(pos);
	if (hover == m_hover)
	{
		return;
	}
	m_hover = hover;
	if (auto* view = getView())
	{
		auto* doc = playerDoc();
		const bool canDrag = doc && !doc->isPlaying();
		view->setCursor(hover && canDrag ? Qt::OpenHandCursor : Qt::ArrowCursor);
	}
	refreshView();
}

void PlayerPresenter::scrubToX(double x)
{
	auto* view = getView();
	auto* doc = playerDoc();
	if (!view || !doc)
	{
		return;
	}
	doc->seekCursor(xToTime(x, doc->durationSeconds(), view->width()));
}

void PlayerPresenter::refreshView()
{
	if (auto* view = getView())
	{
		view->update();
	}
}

void PlayerPresenter::mousePress(QMouseEvent* e)
{
	auto* doc = playerDoc();
	if (!doc || doc->isPlaying() || e->button() != Qt::LeftButton)
	{
		return;
	}

	if (!hitCursor(e->localPos()))
	{
		return;
	}

	m_dragging = true;
	m_hover = true;
	doc->beginCursorDrag();
	if (auto* view = getView())
	{
		view->setCursor(Qt::ClosedHandCursor);
		view->grabMouse();
	}
	scrubToX(e->localPos().x());
}

void PlayerPresenter::mouseMove(QMouseEvent* e)
{
	if (m_dragging)
	{
		scrubToX(e->localPos().x());
		return;
	}

	updateHover(e->localPos());
}

void PlayerPresenter::mouseRelease(QMouseEvent* e)
{
	if (!m_dragging || e->button() != Qt::LeftButton)
	{
		return;
	}

	m_dragging = false;
	if (auto* doc = playerDoc())
	{
		doc->endCursorDrag();
	}
	if (auto* view = getView())
	{
		view->releaseMouse();
		const bool canDrag = playerDoc() && !playerDoc()->isPlaying();
		view->setCursor(m_hover && canDrag ? Qt::OpenHandCursor : Qt::ArrowCursor);
	}
	refreshView();
}

void PlayerPresenter::mouseLeave()
{
	if (m_dragging)
	{
		return;
	}
	if (!m_hover)
	{
		return;
	}
	m_hover = false;
	if (auto* view = getView())
	{
		view->setCursor(Qt::ArrowCursor);
	}
	refreshView();
}

void PlayerPresenter::drawCore(QPainter& painter)
{
	auto* view = getView();
	auto* doc = playerDoc();
	if (!view || !doc)
	{
		return;
	}

	const double w = view->width();
	const double h = view->height();
	drawBackground(painter, kTrackColor);

	const double duration = doc->durationSeconds();
	const double cursorX = timeToX(doc->cursorSeconds(), duration, w);
	const double playheadX = timeToX(doc->playheadSeconds(), duration, w);

	const double barH = qMax(4.0, h * 0.35);
	const double barY = (h - barH) * 0.5;
	const double radius = drawRadius();

	if (cursorX > 0.0)
	{
		painter.setPen(Qt::NoPen);
		painter.setBrush(kCursorBarColor);
		painter.drawRoundedRect(QRectF(0.0, barY, cursorX, barH), 2.0, 2.0);
	}

	if (playheadX > cursorX + 0.5)
	{
		painter.setPen(Qt::NoPen);
		painter.setBrush(kPlayheadBarColor);
		painter.drawRoundedRect(QRectF(cursorX, barY, playheadX - cursorX, barH), 2.0, 2.0);
	}

	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setPen(QPen(QColor(10, 50, 20), 1.0));
	painter.setBrush(kCursorColor);
	painter.drawEllipse(QPointF(cursorX, h * 0.5), radius, radius);
}
