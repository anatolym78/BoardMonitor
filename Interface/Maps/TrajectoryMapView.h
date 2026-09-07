#ifndef TRAJECTORYMAPVIEW_H
#define TRAJECTORYMAPVIEW_H

#include <QWidget>

class QGVMap;

/**
 * @brief Вкладка «Траектория»: карта QGeoView (Google) с начальным видом на Москву.
 */
class TrajectoryMapView : public QWidget
{
	Q_OBJECT
public:
	explicit TrajectoryMapView(QWidget* parent = nullptr);

	QGVMap* map() const { return m_map; }

private:
	void setupNetworkAccess();
	void centerOnMoscow();

	QGVMap* m_map = nullptr;
};

#endif // TRAJECTORYMAPVIEW_H
