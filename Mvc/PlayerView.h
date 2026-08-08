#pragma once

#include "BaseView.h"

/** Узкая полоска шкалы плеера (25 px). */
class PlayerView : public BaseView
{
	Q_OBJECT

public:
	explicit PlayerView(QWidget* parent = nullptr);

protected:
	void resizeEvent(QResizeEvent* e) override;
};
