#pragma once

#include "BaseDoc.h"

/**
 * @brief Документ временной шкалы плеера.
 *
 * cursorSeconds — позиция «ползунка» (кружок); при остановке UI может замирать.
 * playheadSeconds — реальное продолжающееся воспроизведение (полоса после кружка).
 */
class PlayerDocument : public BaseDoc
{
	Q_OBJECT

public:
	explicit PlayerDocument(QObject* parent = nullptr);

	double durationSeconds() const { return m_durationSeconds; }
	double cursorSeconds() const { return m_cursorSeconds; }
	double playheadSeconds() const { return m_playheadSeconds; }
	bool isPlaying() const { return m_isPlaying; }
	bool isLiveMode() const { return m_liveMode; }
	bool isDragging() const { return m_dragging; }

	void setPlaying(bool playing);
	void setLiveMode(bool live);
	void setTimeline(double durationSeconds, double cursorSeconds, double playheadSeconds);

	void beginCursorDrag();
	void endCursorDrag();
	/** Перемещение курсора пользователем (scrub); эмитит cursorSeeked. */
	void seekCursor(double cursorSeconds);

signals:
	void cursorSeeked(double seconds);

protected:
	void processCore() override {}

private:
	double m_durationSeconds = 0.0;
	double m_cursorSeconds = 0.0;
	double m_playheadSeconds = 0.0;
	bool m_isPlaying = false;
	bool m_liveMode = false;
	bool m_dragging = false;
};
