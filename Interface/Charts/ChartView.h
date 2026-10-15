#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QColor>
#include <QBrush>

#include "qcustomplot.h"

#include "../../ViewModel/ChartsModel.h"

class QToolButton;
class QResizeEvent;

/**
 * @brief Виджет одной ячейки графика (обёртка над QCustomPlot).
 *
 * Живёт в сетке ChartsPanel. Точки серий пишет ChartsPanel.
 * Колёсико — вертикальный масштаб 0.5…5.0 относительно курсора; ЛКМ сбрасывает в 1.0.
 * Кнопка в правом верхнем углу удаляет график из ChartsModel.
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

	/** Базовый диапазон Y от данных (без учёта zoom); применяет текущий m_valueZoom. */
	void setValueBaseRange(double lower, double upper);
	double valueZoom() const { return m_valueZoom; }
	void setValueZoom(double zoom, double anchorY);
	void resetValueZoom();

signals:
	/** Курсор вошёл в серию или покинул её: в QCustomPlot 1.x у серий нет своего сигнала hovered. */
	void graphHovered(QCPGraph* graph, bool hovered);
	/** Запрос удалить этот слот графика (индекс на момент клика). */
	void removeRequested(int chartIndex);

protected:
	void leaveEvent(QEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

private:
	void updateBackground();
	void setHoveredGraph(QCPGraph* graph);
	void applyZoomedRange(const double* anchorY);
	void updateZoomLabel();
	void positionOverlayControls();
	void onRemoveClicked();

	int m_row = 0;
	int m_column = 0;
	int m_chartIndex = 0;
	bool m_selected = false;

	ChartsModel* m_chartsModel = nullptr;
	QCPGraph* m_hoveredGraph = nullptr;
	QToolButton* m_removeButton = nullptr;

	double m_baseLower = 0.0;
	double m_baseUpper = 1.0;
	bool m_hasBaseRange = false;
	double m_valueZoom = 1.0;
	QCPItemText* m_zoomLabel = nullptr;

	static constexpr double kZoomMin = 0.5;
	static constexpr double kZoomMax = 5.0;
	static constexpr int kOverlayMargin = 4;
};

#endif // CHARTVIEW_H
