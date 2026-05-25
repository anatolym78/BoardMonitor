#ifndef PARAMETERSCHARTVIEW_H
#define PARAMETERSCHARTVIEW_H

#include <QtCharts/QChartView>
#include <QColor>
#include <QBrush>

#include "../../ViewModel/ChatViewGridModel.h"

/**
 * @brief Виджет отдельного графика.
 * 
 * Отображает один график в сетке ChartsDashboardView. 
 * Может содержать одну или несколько серий данных (линий).
 * Поддерживает выделение и взаимодействие с мышью.
 */
class ParametersChartView : public QtCharts::QChartView
{
	Q_OBJECT

public:
	explicit ParametersChartView(int chartIndex, int row, int column, QWidget* parent = nullptr);
	void setModel(ChatViewGridModel* model) { m_chartsModel = model; }
	void setSelected(bool selected);
	bool isSelected() const { return m_selected; }

	void setHovered(bool hover);
	int chartIndex() const { return m_chartIndex; }

protected:
	void enterEvent(QEvent* event) override;
	void leaveEvent(QEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
private:
	void updateBackground();

	int m_row = 0;
	int m_column = 0;
	int m_chartIndex = 0;
	bool m_selected = false;
	bool m_hovered = false;
	QBrush m_originalChartBackground;

	ChatViewGridModel* m_chartsModel = nullptr;
};

#endif // PARAMETERSCHARTVIEW_H

