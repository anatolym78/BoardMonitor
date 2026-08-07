#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QColor>
#include <QBrush>

#include "qcustomplot.h"

#include "../../ViewModel/ChartsModel.h"

/**
 * @brief Виджет одной ячейки графика (обёртка над QCustomPlot).
 *
 * Живёт в сетке ChartsPanel. Отвечает за локальный UI: фон при выделении/hover,
 * клик (selection в ChartsModel), сигнал graphHovered. Точки серий пишет ChartsPanel.
 */
class ChartView : public QCustomPlot
{
	Q_OBJECT

public:
	explicit ChartView(int chartIndex, int row, int column, QWidget* parent = nullptr);
	void setModel(ChartsModel* model) { m_chartsModel = model; }
	void setSelected(bool selected);
	bool isSelected() const { return m_selected; }

	void setHovered(bool hover);
	int chartIndex() const { return m_chartIndex; }
	void setChartIndex(int index) { m_chartIndex = index; }

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

	ChartsModel* m_chartsModel = nullptr;
	QCPGraph* m_hoveredGraph = nullptr;
};

#endif // CHARTVIEW_H
