#include "PlayerPresenter.h"

#include "BaseView.h"

#include <QMouseEvent>
#include <QPainter>

namespace
{
const QColor kTrackFill(210, 210, 210);
const QColor kTrackBorder(150, 150, 150);
const QColor kProgress(100, 180, 230); // голубая заполненная часть (и слева, и справа на паузе)
const QColor kThumbFill(45, 125, 210);
const QColor kThumbBorder(30, 95, 170);
const QColor kThumbHoverFill(20, 20, 20);
const QColor kThumbHoverBorder(0, 0, 0);
const QColor kThumbLockedFill(140, 140, 140);
const QColor kThumbLockedBorder(100, 100, 100);
const double kSizeBoost = 1.25;
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

double PlayerPresenter::thumbHalf() const
{
	const auto* view = getView();
	if (!view)
	{
		return 2.0 * kSizeBoost * 1.5;
	}
	// Ширина ползунка ≈ в 3 раза меньше прежнего квадрата, затем ×1.5
	return qMin(view->height() * 0.14, 2.7) * kSizeBoost * 1.5;
}

double PlayerPresenter::thumbHalfHeight() const
{
	const auto* view = getView();
	if (!view)
	{
		return 8.0 * 1.5;
	}
	return qMin(view->height() * 0.45, 10.0) * 1.5;
}

double PlayerPresenter::timeToX(double seconds, double duration, double width) const
{
	const double half = thumbHalf();
	const double span = qMax(1.0, width - 2.0 * half);
	if (duration <= 0.0)
	{
		return half;
	}
	const double t = qBound(0.0, seconds / duration, 1.0);
	return half + t * span;
}

double PlayerPresenter::xToTime(double x, double duration, double width) const
{
	const double half = thumbHalf();
	const double span = qMax(1.0, width - 2.0 * half);
	if (duration <= 0.0)
	{
		return 0.0;
	}
	const double t = qBound(0.0, (x - half) / span, 1.0);
	return t * duration;
}

QRectF PlayerPresenter::thumbRect() const
{
	const auto* view = getView();
	const auto* doc = static_cast<const PlayerDocument*>(getDoc());
	if (!view || !doc)
	{
		return {};
	}

	const double halfW = thumbHalf();
	const double halfH = thumbHalfHeight();
	const double cursorX = timeToX(doc->cursorSeconds(), doc->durationSeconds(), view->width());
	const double cy = view->height() * 0.5;
	return QRectF(cursorX - halfW, cy - halfH, halfW * 2.0, halfH * 2.0);
}

bool PlayerPresenter::hitCursor(const QPointF& pos) const
{
	const QRectF thumb = thumbRect();
	if (thumb.isNull())
	{
		return false;
	}
	return thumb.adjusted(-3.0, -3.0, 3.0, 3.0).contains(pos);
}

void PlayerPresenter::updateHover(const QPointF& pos)
{
	auto* doc = playerDoc();
	if (doc && doc->isLiveMode() && doc->isPlaying())
	{
		if (m_hover)
		{
			m_hover = false;
			refreshView();
		}
		return;
	}

	const bool hover = hitCursor(pos);
	if (hover == m_hover)
	{
		return;
	}
	m_hover = hover;
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
	if (!doc || e->button() != Qt::LeftButton)
	{
		return;
	}

	// Live + проигрывание: ползунок серый и недоступен; на паузе — можно двигать
	if (doc->isLiveMode() && doc->isPlaying())
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
	const double duration = doc->durationSeconds();
	const double cursorX = timeToX(doc->cursorSeconds(), duration, w);
	const double playheadX = timeToX(doc->playheadSeconds(), duration, w);

	const double barH = qMax(3.0, h * 0.22);
	const double barY = (h - barH) * 0.5;
	const QRectF trackRect(0.0, barY, w, barH);

	painter.setRenderHint(QPainter::Antialiasing, true);

	// Тонкая серая дорожка с рамкой (как у обычного QSlider)
	painter.setPen(QPen(kTrackBorder, 1.0));
	painter.setBrush(kTrackFill);
	painter.drawRect(trackRect);

	painter.setPen(Qt::NoPen);
	painter.setBrush(kProgress);

	// timeToX(duration) = центр ползунка у конца (width - half), не правый край дорожки
	const bool fillToEnd = duration > 0.0
		&& doc->playheadSeconds() >= duration - 1e-9;
	const double progressRight = fillToEnd ? w : playheadX;

	if (fillToEnd)
	{
		painter.drawRect(trackRect);
	}
	else
	{
		// Слева от ползунка — голубая заполненная часть
		if (cursorX > 1.0)
		{
			painter.drawRect(QRectF(0.0, barY, cursorX, barH));
		}

		// На паузе live: справа от ползунка растёт та же голубая полоса (playhead)
		if (progressRight > cursorX + 0.5)
		{
			painter.drawRect(QRectF(cursorX, barY, progressRight - cursorX, barH));
		}
	}

	// Ползунок: live+play — серый и недоступен; иначе синий, при наведении — чёрный
	const QRectF thumb = thumbRect();
	const bool locked = doc->isLiveMode() && doc->isPlaying();
	const bool hot = !locked && (m_hover || m_dragging);
	if (locked)
	{
		painter.setPen(QPen(kThumbLockedBorder, 1.0));
		painter.setBrush(kThumbLockedFill);
	}
	else
	{
		painter.setPen(QPen(hot ? kThumbHoverBorder : kThumbBorder, 1.0));
		painter.setBrush(hot ? kThumbHoverFill : kThumbFill);
	}
	painter.drawRect(thumb);
}
