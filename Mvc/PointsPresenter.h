#pragma once

#include "Presenter.h"
#include "PointsDoc.h"

/**
 * @brief Presenter для PointsDoc: бывшие PointsController + PointsRenderer.
 */
class PointsPresenter : public Presenter
{
public:
	PointsPresenter();
	explicit PointsPresenter(PointsDoc* pdoc);
	~PointsPresenter() override;

	Presenter* clone() override;

protected:
	void drawCore(QPainter& painter) override;

private:
	void drawPoints(QPainter& painter);
	void drawCircles(QPainter& painter);
	void drawCrosses(QPainter& painter);

	PointsDoc* pointsDoc() { return static_cast<PointsDoc*>(getDoc()); }

	bool m_bShowPoints = true;
};
