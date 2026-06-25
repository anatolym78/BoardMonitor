#include "ChartsDashboardView.h"

#include <QScrollArea>
#include <QGridLayout>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLayoutItem>
#include <QDebug>
#include <algorithm>

#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>

#include <QToolButton>

#include <QToolButton>

#include "../../Model/Parameters/Tree/ParameterTreeHistoryItem.h"

ChartsDashboardView::ChartsDashboardView(QWidget *parent)
	: QFrame(parent)
{
	// Верхняя панель с элементами управления
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	QHBoxLayout* topBarLayout = new QHBoxLayout();
	topBarLayout->setContentsMargins(2, 2, 2, 0);

	// Default: 1-column mode, button shows "switch to 2 columns" icon
	m_toggleColumnButton = new QToolButton(this);
	m_toggleColumnButton->setIcon(QIcon(":/Resources/icons8-two_column-32.png"));
	m_toggleColumnButton->setIconSize(QSize(32, 32));
	m_toggleColumnButton->setToolTip(tr("Switch to two columns"));
	m_toggleColumnButton->setAutoRaise(true);
	connect(m_toggleColumnButton, &QToolButton::clicked, this, &ChartsDashboardView::onToggleColumnClicked);

	QToolButton* mergeButton = new QToolButton(this);
	mergeButton->setIcon(QIcon(":/Resources/icons8-merge-documents-32.png"));
	mergeButton->setIconSize(QSize(32, 32));
	mergeButton->setToolTip(tr("Merge selected charts"));
	mergeButton->setAutoRaise(true);
	connect(mergeButton, &QToolButton::clicked, this, &ChartsDashboardView::onMergeChartsClicked);

	topBarLayout->addWidget(m_toggleColumnButton, 0, Qt::AlignLeft);
	topBarLayout->addSpacing(2);
	topBarLayout->addWidget(mergeButton, 0, Qt::AlignLeft);
	topBarLayout->addStretch(1);

	mainLayout->addLayout(topBarLayout);

	m_scrollArea = new QScrollArea(this);
	m_scrollArea->setWidgetResizable(true);
	m_scrollArea->setFrameShape(QFrame::NoFrame);

	m_scrollContent = new QWidget();
	m_gridLayout = new QGridLayout(m_scrollContent);
	m_gridLayout->setSpacing(2);
	m_gridLayout->setContentsMargins(2, 2, 2, 2);

	m_scrollArea->setWidget(m_scrollContent);

	mainLayout->addWidget(m_scrollArea);

	// Следим за изменением ширины области просмотра, чтобы пересчитывать размеры ячеек (квадратов)
	m_scrollArea->viewport()->installEventFilter(this);
}

void ChartsDashboardView::setLayoutInteractionPaused(bool paused)
{
	if (m_layoutInteractionPaused == paused)
	{
		return;
	}

	m_layoutInteractionPaused = paused;

	if (!paused)
	{
		updateCellSizes();
	}
}

void ChartsDashboardView::setModel(ChatViewGridModel* chartsModel)
{
	m_chartsModel = chartsModel;

	connect(m_chartsModel, &ChatViewGridModel::parameterAdded, this, &ChartsDashboardView::onAddChart);
	connect(m_chartsModel, &ChatViewGridModel::parameterNeedToRemove, this, &ChartsDashboardView::onParameterRemoved);
	connect(m_chartsModel, &ChatViewGridModel::parametersNeedToMove, this, &ChartsDashboardView::onParameterMoved);
}

void ChartsDashboardView::onParameterItemHovered(ParameterTreeHistoryItem* treeItem)
{
	for (auto chartView : chartViewList())
	{
		for (auto series : chartView->chart()->series())
		{
			if (treeItem && (treeItem->fullName() == series->name()))
			{
				chartView->setHovered(true);
				hoverSeries(series);
			}
			else
			{
				chartView->setHovered(false); 
				restoreSeriesColor(series);
			}
		}
	}
}
void ChartsDashboardView::restoreSeriesColor(QtCharts::QAbstractSeries* series)
{
	if (series->type() != QtCharts::QAbstractSeries::SeriesTypeLine) return;

	auto lineSeries = (QtCharts::QLineSeries*)(series);

	if (m_seriesColors.contains(series))
	{
		lineSeries->setColor(m_seriesColors[series]);
	}

	auto seriesPen = lineSeries->pen();
	seriesPen.setStyle(Qt::PenStyle::SolidLine);

	lineSeries->setPen(seriesPen);
}

void ChartsDashboardView::hoverSeries(QtCharts::QAbstractSeries* series)
{
	if (series->type() != QtCharts::QAbstractSeries::SeriesTypeLine) return;
	
	auto lineSeries = (QtCharts::QLineSeries*)(series);

	if (!m_seriesColors.contains(series))
	{
		m_seriesColors.insert(series, lineSeries->color());
	}

	//lineSeries->setColor(QColor(225, 192, 0));

	auto seriesPen = lineSeries->pen();
	seriesPen.setStyle(Qt::PenStyle::DashLine);

	lineSeries->setPen(seriesPen);
}

void ChartsDashboardView::onAddChart(int chartIndex, ParameterTreeItem* parameter)
{
	auto parameterFullName = parameter->fullName();

	if (m_chartsModel->hasSeries(parameterFullName)) return;

	auto row = chartIndex / m_columnCount;
	auto column = chartIndex % m_columnCount;

	// create chart
	auto chartView = new ParametersChartView(chartIndex, row, column, this);
	chartView->setModel(m_chartsModel);
	chartView->setRenderHint(QPainter::Antialiasing, true);
	auto chart = new QtCharts::QChart();
	chart->setAnimationOptions(QtCharts::QChart::NoAnimation);
	chart->setBackgroundBrush(QBrush());
	chartView->setChart(chart);
	m_gridLayout->addWidget(chartView, row, column);
	chart->legend()->setVisible(false);

	// create timeaxis
	auto timeAxis = new QtCharts::QDateTimeAxis(chart);
	timeAxis->setFormat("hh:mm:ss");
	timeAxis->setTickCount(3);
	chart->addAxis(timeAxis, Qt::AlignBottom);

	//create value axis
	auto valueAxis = new QtCharts::QValueAxis(chart);
	valueAxis->setTickCount(3);
	valueAxis->setMin(0);
	valueAxis->setMax(1);
	chart->addAxis(valueAxis, Qt::AlignLeft);


	// create series
	if (parameter->type() == ParameterTreeItem::ItemType::History)
	{
		addSeriesToChart(chartIndex, chart, parameter);
	}

	if (parameter->type() == ParameterTreeItem::ItemType::Array
		|| parameter->type() == ParameterTreeItem::ItemType::Group)
	{
		chart->setTitle(parameterFullName);
		addHistoryDescendantsToChart(chartIndex, chart, parameter);
	}

	// После добавления - пересчитать размеры ячеек, чтобы они вписались по ширине и имели квадратную форму
	updateCellSizes();
}

void ChartsDashboardView::addHistoryDescendantsToChart(int chartIndex, QtCharts::QChart* chart, ParameterTreeItem* item)
{
	if (!item)
	{
		return;
	}

	if (item->type() == ParameterTreeItem::ItemType::History)
	{
		addSeriesToChart(chartIndex, chart, item);
		return;
	}

	for (ParameterTreeItem* child : item->children())
	{
		addHistoryDescendantsToChart(chartIndex, chart, child);
	}
}

void ChartsDashboardView::addSeriesToChart(int chartIndex, QtCharts::QChart* chart, ParameterTreeItem* parameter)
{
	if (!parameter || m_chartsModel->hasSeries(parameter->fullName()))
	{
		return;
	}

	auto timeAxis = (QtCharts::QDateTimeAxis*)chart->axisX();
	auto valueAxis = (QtCharts::QValueAxis*)chart->axisY();

	auto series = new QtCharts::QLineSeries();
	chart->addSeries(series);
	series->attachAxis(timeAxis);
	series->attachAxis(valueAxis);

	auto seriesPen = series->pen();
	seriesPen.setCapStyle(Qt::PenCapStyle::RoundCap);

	auto color = parameter->color();

	m_chartsModel->addSeriesToChart(
		chartIndex,
		parameter->fullName(),
		color,
		series,
		timeAxis,
		valueAxis);

	connect(series, &QtCharts::QLineSeries::hovered, this, [this, series](const QPointF& point, bool state)
		{
			if (state)
			{
				hoverSeries(series);
			}
			else
			{
				restoreSeriesColor(series);
			}
		});
}

void ChartsDashboardView::onParameterRemoved(int chartIndex, const QString& label)
{
	auto chartView = getChartView(chartIndex);
	if (!chartView)
	{
		return;
	}

	auto chart = chartView->chart();
	if (!chart)
	{
		return;
	}

	for (auto series : chart->series())
	{
		if (series->name() == label)
		{
			chart->removeSeries(series);
			series->deleteLater();
			break;
		}
	}

	if (chart->series().isEmpty())
	{
		m_gridLayout->removeWidget(chartView);
		chartView->deleteLater();
	}

	updateCellSizes();
}

void ChartsDashboardView::onParameterMoved(int targetChartIndex, const QStringList& labels)
{
	auto targetChartView = getChartView(targetChartIndex);
	if (!targetChartView) return;

	auto targetChart = targetChartView->chart();
	if (!targetChart) return;

	const QSet<QString> labelsSet = QSet<QString>(labels.begin(), labels.end());

	// Обходим все графики, кроме целевого, и переносим подходящие серии
	for (int i = 0; i < m_gridLayout->count(); ++i)
	{
		if (auto item = m_gridLayout->itemAt(i))
		{
			auto sourceView = qobject_cast<ParametersChartView*>(item->widget());
			if (!sourceView || sourceView == targetChartView) continue;

			auto sourceChart = sourceView->chart();
			if (!sourceChart) continue;

			// Копируем список, чтобы безопасно модифицировать исходный график
			const auto sourceSeriesList = sourceChart->series();
			bool anyMovedFromSource = false;
			for (auto series : sourceSeriesList)
			{
				if (!series) continue;
				if (!labelsSet.contains(series->name())) continue;

				// Сначала удаляем из исходного графика
				sourceChart->removeSeries(series);
				// Затем добавляем в целевой график
				targetChart->addSeries(series);
				// Подключаем оси целевого графика
				if (targetChart->axisX()) series->attachAxis(targetChart->axisX());
				if (targetChart->axisY()) series->attachAxis(targetChart->axisY());

				anyMovedFromSource = true;
			}

			// Если исходный график опустел — удаляем виджет
			if (anyMovedFromSource && sourceChart->series().isEmpty())
			{
				m_gridLayout->removeWidget(sourceView);
				sourceView->deleteLater();
			}
		}
	}

	targetChartView->setSelected(false);
	updateCellSizes();
}

ParametersChartView* ChartsDashboardView::getChartView(int chartIndex)
{
	for (int i = 0; i < m_gridLayout->count(); ++i)
	{
		if (auto item = m_gridLayout->itemAt(i))
		{
			auto view = qobject_cast<ParametersChartView*>(item->widget());
			if (view && view->chartIndex() == chartIndex)
			{
				return view;
			}
		}
	}
	return nullptr;
}

bool ChartsDashboardView::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == m_scrollArea->viewport() && event->type() == QEvent::Resize)
	{
		if (!m_layoutInteractionPaused)
		{
			updateCellSizes();
		}
	}
	return QFrame::eventFilter(watched, event);
}



void ChartsDashboardView::updateCellSizes()
{
	if (!m_gridLayout || !m_scrollArea) return;

	const int viewportWidth = m_scrollArea->viewport()->width();
	const QMargins margins = m_gridLayout->contentsMargins();
	const int hSpacing = m_gridLayout->horizontalSpacing();

	// Ширина контента с учётом внутренних отступов
	const int contentWidth = viewportWidth - margins.left() - margins.right();
	if (contentWidth <= 0 || m_columnCount <= 0) return;

	// Ширина одной ячейки с учётом горизонтальных промежутков
	const int totalSpacing = hSpacing * (m_columnCount - 1);
	const int cellWidth = (contentWidth - totalSpacing) / m_columnCount;
	if (cellWidth <= 0) return;

	// Пробегаем все элементы сетки и задаём им квадратный фиксированный размер
	const int itemCount = m_gridLayout->count();
	for (int i = 0; i < itemCount; ++i)
	{
		if (auto item = m_gridLayout->itemAt(i))
		{
			if (auto w = item->widget())
			{
				w->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
				w->setFixedSize(cellWidth, cellWidth);
			}
		}
	}

	// Обновление размеров контента для корректной вертикальной прокрутки
	m_scrollContent->adjustSize();
}

void ChartsDashboardView::onToggleColumnClicked()
{
	if (m_columnCount == 1)
	{
		m_columnCount = 2;
		m_toggleColumnButton->setIcon(QIcon(":/Resources/icons8-one_column-32.png"));
		m_toggleColumnButton->setToolTip(tr("Switch to one column"));
	}
	else
	{
		m_columnCount = 1;
		m_toggleColumnButton->setIcon(QIcon(":/Resources/icons8-two_column-32.png"));
		m_toggleColumnButton->setToolTip(tr("Switch to two columns"));
	}
	relayoutChartsGrid();
}

void ChartsDashboardView::onMergeChartsClicked()
{
	m_chartsModel->mergeCharts();
}

QList<ParametersChartView*> ChartsDashboardView::chartViewList() const
{
	QList<ParametersChartView*> chartViews;
	for (auto i = 0; i < m_gridLayout->count(); i++)
	{
		auto item = m_gridLayout->itemAt(i);
		auto chartView = (ParametersChartView*)(item->widget());

		if (chartView)
		{
			chartViews.append(chartView);
		}
	}

	return chartViews;
}



void ChartsDashboardView::relayoutChartsGrid()
{
	if (!m_gridLayout) return;

	// Собираем все текущие виды графиков
	QList<ParametersChartView*> views;
	for (int i = 0; i < m_gridLayout->count(); ++i)
	{
		if (auto item = m_gridLayout->itemAt(i))
		{
			if (auto v = qobject_cast<ParametersChartView*>(item->widget()))
			{
				views.append(v);
			}
		}
	}

	// Стабильный порядок по chartIndex
	std::sort(views.begin(), views.end(), [](ParametersChartView* a, ParametersChartView* b)
	{
		return a->chartIndex() < b->chartIndex();
	});

	// Убираем из layout, затем добавим заново согласно новым колонкам
	for (auto v : views)
	{
		m_gridLayout->removeWidget(v);
	}

	for (int idx = 0; idx < views.size(); ++idx)
	{
		int row = m_columnCount > 0 ? idx / m_columnCount : 0;
		int col = m_columnCount > 0 ? idx % m_columnCount : 0;
		m_gridLayout->addWidget(views.at(idx), row, col);
	}

	updateCellSizes();
}