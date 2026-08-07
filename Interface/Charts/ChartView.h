#ifndef PARAMETERSCHARTVIEW_H
#define PARAMETERSCHARTVIEW_H

#include <QColor>
#include <QBrush>

#include "qcustomplot.h"

#include "../../ViewModel/ChatViewGridModel.h"

/**
 * @brief Виджет отдельного графика.
 * 
 * Отображает один график в сетке ChartsDashboardView. 
 * Может содержать одну или несколько серий данных (линий).
 * Поддерживает выделение и взаимодействие с мышью.
 */
class ParametersChartView : public QCustomPlot
{
	Q_OBJECT

public:
	explicit ParametersChartView(int chartIndex, int row, int column, QWidget* parent = nullptr);
	void setModel(ChatViewGridModel* model) { m_chartsModel = model; }
	void setSelected(bool selected);
	bool isSelected() const { return m_selected; }

	void setHovered(bool hover);
	int chartIndex() const { return m_chartIndex; }

signals:
	/** Курсор вошёл в серию или покинул её: в QCustomPlot 1.x у серий нет своего сигнала hovered. */
	void graphHovered(QCPGraph* graph, bool hovered);

protected:
	void enterEvent(QEvent* event) override;
	void leaveEvent(QEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
private:
	void updateBackground();
	void setHoveredGraph(QCPGraph* graph);

	int m_row = 0;
	int m_column = 0;
	int m_chartIndex = 0;
	bool m_selected = false;
	bool m_hovered = false;

	ChatViewGridModel* m_chartsModel = nullptr;
	QCPGraph* m_hoveredGraph = nullptr;
};

#endif // PARAMETERSCHARTVIEW_H
