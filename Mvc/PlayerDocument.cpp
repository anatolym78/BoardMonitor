#include "PlayerDocument.h"

#include <QtGlobal>

PlayerDocument::PlayerDocument(QObject* parent)
	: BaseDoc(parent)
{
}

void PlayerDocument::setPlaying(bool playing)
{
	if (m_isPlaying == playing)
	{
		return;
	}
	m_isPlaying = playing;
	updateViews();
}

void PlayerDocument::setLiveMode(bool live)
{
	if (m_liveMode == live)
	{
		return;
	}
	m_liveMode = live;
	updateViews();
}

void PlayerDocument::setTimeline(double durationSeconds, double cursorSeconds, double playheadSeconds)
{
	m_durationSeconds = qMax(0.0, durationSeconds);
	m_playheadSeconds = qBound(0.0, playheadSeconds, m_durationSeconds);

	if (m_dragging)
	{
		// Во время drag курсор ведёт пользователь; обновляем только длительность/playhead
		updateViews();
		return;
	}

	m_cursorSeconds = qBound(0.0, cursorSeconds, m_durationSeconds);
	updateViews();
}

void PlayerDocument::beginCursorDrag()
{
	m_dragging = true;
}

void PlayerDocument::endCursorDrag()
{
	m_dragging = false;
}

void PlayerDocument::seekCursor(double cursorSeconds)
{
	m_cursorSeconds = qBound(0.0, cursorSeconds, m_durationSeconds);
	emit cursorSeeked(m_cursorSeconds);
	updateViews();
}
