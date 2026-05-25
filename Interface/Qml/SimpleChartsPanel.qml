import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtCharts 2.15

Rectangle
{
    id: simpleChartsPanel
    color: "transparent"
    radius: 4
    clip: true
    
    function toggleParameter(label, color)
    {
        if(chartViewModel)
        {
            if(!chartViewModel.toggleParameter(label, color))
            {
                return

                for (let i = 0; i < repeater.count; i++)
                {
                    const chartView = repeater.itemAt(i);

                    if(chartView.seriesMap.hasOwnProperty(label))
                    {
                        var series = chartView.seriesMap[label]

                        chartView.removeSeries(series)

                        delete chartView.seriesMap.label
                    }
                }
            }
        }
    }

    function firstOf(series)
    {
        return series.count > 0 ? series.at(0) : null
    }

    function lastOf(series)
    {
        return series.count > 0 ? series.at(series.count - 1) : null
    }

    function initializeAxes(parameter, timeAxis, valueAxis)
    {
        var parameterValue = parameter.value
        var parameterDate = new Date(parameter.timestamp)

        timeAxis.min = new Date(parameterDate.getTime());
        timeAxis.max = new Date(parameterDate.getTime() + 60*1000)

        valueAxis.min = parameterValue - 1
        valueAxis.max = parameterValue + 1
    }

    function fitAxes(parameter, timeAxis, valueAxis, series)
    {
        // корректируем если более одной точки
        var countPoints = series.count
        if(countPoints < 2) return

        var parameterValue = parameter.value
        var parameterDate = new Date(parameter.timestamp)
        var parameterTime = parameterDate.getTime()

        // корректируем ось значений (сейчас логика такова, что диапазон может только расти, но не может сужаться)
        var valueMin = valueAxis.min
        var valueMax = valueAxis.max

        if(valueMax < parameterValue)
        {
            valueAxis.max = parameterValue + 1
        }

        if(valueMin > parameterValue)
        {
            valueAxis.min = parameterValue - 1
        }

        // корректируем временную шкалу
        var timeMin = timeAxis.min
        var timeMax = timeAxis.max
        var timeRange = timeMax - timeMin

        if(parameterDate > timeMax)
        {
            timeAxis.max = parameterDate
            timeAxis.min = new Date(parameterTime - timeRange)
        }

        if(parameterDate < timeMin)
        {
            timeAxis.min = parameterDate
            timeAxis.max = new Date(parameterTime + timeRange)
        }
    }

    function insertBackPlayedPoint(parameterTimeMsec, parameterValue, timeAxis, series)
    {
        var timeMin = timeAxis.min
        var timeMax = timeAxis.max

        var firstPoint = series.at(0)
        var firstPointTime = firstPoint.x
        if(parameterTimeMsec < firstPointTime)
        {
            series.insert(0, parameterTimeMsec, parameterValue)
        }

        var lastPoint = series.at(series.count - 1)
        if(lastPoint.x > timeMax)
        {
            series.remove(series.count - 1)
        }
    }

    function insertForwardPlayedPoint(parameterTimeMsec, parameterValue, timeAxis, series)
    {
        if(series.count < 1)
        {
            series.append(parameterTimeMsec, parameterValue)

            return
        }

        var timeMin = timeAxis.min.getTime()
        var timeMax = timeAxis.max.getTime()

        var lastPoint = series.at(series.count - 1)
        var lastPointTime = lastPoint.x
        if(parameterTimeMsec > lastPointTime)
        {
            series.append(parameterTimeMsec, parameterValue)
        }

        while (series.count > 0 && series.at(0).x < timeMin)
        {
            series.remove(0)
        }
    }

    function chartHasLabel(chartLabels, label)
    {
        return chartLabels.indexOf(label) >= 0
    }

    function isSeriesCreated(seriesMap, label)
    {
        return label in seriesMap
    }


    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Text
        {
            text: "Charts"
            font.pointSize: 12
            font.bold: true
            color: "dimgray"
            Layout.alignment: Qt.AlignHCenter
        }

        Flow
        {
            id: chartsFlow
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10
        
            // Принудительно обновляем layout при изменении размера
            onWidthChanged:
            {
                // Небольшая задержка для корректного пересчета
                Qt.callLater(function()
                {
                    chartsFlow.childrenChanged()
                })
            }

            Repeater
            {
                model: chartViewModel
                id: repeater

                delegate: Item
                {
                    id: chartContainer
                    width: 600
                    height: 480

                    // Карта для хранения серий по меткам параметров
                    property var seriesMap: ({})

                    // График
                    ChartView
                    {
                        id: chartView
                        property int chartIndex: index
                        width: 600
                        //width: Math.max(300, Math.min(400, chartsFlow.width / 3))
                        height: 480
                        backgroundColor: "white"
                        z: depth
                        theme: ChartView.ChartThemeLight

                        // Простая настройка осей
                        DateTimeAxis
                        {
                            id: timeAxis
                            format: "hh:mm:ss"
                            tickCount: 3
                        }

                        ValueAxis
                        {
                            id: valueAxis
                            tickCount: 3
                            visible: true
                        }

                        axes: [timeAxis, valueAxis]
                        antialiasing: true
                        animationOptions: ChartView.NoAnimation

                        // Создаем серии при инициализации
                        Component.onCompleted:
                        {

                        }

                        Connections
                        {
                            target: parametersPlayer

                            function onParameterPlayed(parameter)
                            {
                                var parameterLabel = parameter.label

                                if(!chartHasLabel(labels, parameterLabel)) return

                                var parameterValue = parameter.value
                                var parameterDate = new Date(parameter.timestamp)
                                var parameterTimeMsec = parameterDate.getTime()

                                if(!isSeriesCreated(seriesMap, parameterLabel))
                                {
                                    initializeAxes(parameter, timeAxis, valueAxis)

                                    seriesMap[parameterLabel] = chartView.createSeries(ChartView.SeriesTypeLine, parameterLabel, timeAxis, valueAxis)
                                    seriesMap[parameterLabel].useOpenGL = true
                                }

                                fitAxes(parameter, timeAxis, valueAxis, seriesMap[parameterLabel])

                                insertForwardPlayedPoint(parameterTimeMsec, parameterValue, timeAxis, seriesMap[parameterLabel])

                                updateTimeMarker(parameterTimeMsec)
                            }

                            function onParameterBackPlayed(parameter)
                            {
                                var parameterLabel = parameter.label

                                if(!chartHasLabel(labels, parameterLabel)) return

                                var parameterValue = parameter.value
                                var parameterDate = new Date(parameter.timestamp)
                                var parameterTimeMsec = parameterDate.getTime()

                                if(!isSeriesCreated(seriesMap, parameterLabel)) return

                                fitAxes(parameter, timeAxis, valueAxis, seriesMap[parameterLabel])

                                //
                                insertBackPlayedPoint(parameterTimeMsec, parameterValue, timeAxis, seriesMap[parameterLabel])

                                updateTimeMarker(parameterTimeMsec)
                            }
                        }

                        MouseArea
                        {
                            anchors.fill: parent
                            acceptedButtons: Qt.LeftButton | Qt.RightButton
                            onPressed:
                            {
                                if(mouse.button == Qt.RightButton)
                                {
                                }
                            }
                            onWheel:
                            {
                                if(wheel.angleDelta.y > 0)
                                {
                                    zoom(1.1)
                                }
                                else
                                {
                                    zoom(0.9)
                                }

                            }
                        }

                        Drag.active: dragHandler.active
                        Drag.hotSpot.x: width/2
                        Drag.hotSpot.y: height/2

                        DragHandler
                        {
                            id: dragHandler
                            onActiveChanged:
                            {
                                if(active)
                                {
                                    chartView.Drag.start()

                                    chartViewModel.reorderChartsBeforeDrag(chartIndex)
                                }
                                else
                                {
                                    chartView.Drag.drop()
                                }
                            }
                        }

                        DropArea
                        {
                            anchors.fill: parent
                            onDropped:
                            {
                                if(drag.source !== chartView)
                                {
                                    var droppedChartView = drop.source
                                    var targetChartView = chartView

                                    var droppedChartLabels = chartViewModel.getChartSeriesLabels(droppedChartView.chartIndex);
                                    var targetChartLabels = chartViewModel.getChartSeriesLabels(targetChartView.chartIndex)

                                    var droppedSeries = droppedChartView.seriesMap[droppedChartLabels[0]]
                                    var targetSeries = targetChartView.seriesMap[targetChartLabels[0]]

                                    moveSeries(droppedChartView, targetChartView)

                                    chartViewModel.mergeCharts(droppedChartView.chartIndex, targetChartView.chartIndex)

                                    targetChartView.backgroundColor = "white"
                                    targetChartView.opacity = 1

                                    for(let i=0;i<2;i++)
                                    {
                                        chartView.axes[i].visible = true
                                        chartView.axes[i].tickCount = 3
                                    }
                                }

                                //chartViewModel.resetDepths()
                            }

                            function moveSeries(droppedChartView, targetChartView)
                            {
                                var droppedChartLabels = chartViewModel.getChartSeriesLabels(droppedChartView.chartIndex);

                                for(const droppedLabel of droppedChartLabels)
                                {
                                    var droppedSeries = droppedChartView.seriesMap[droppedLabel]

                                    var targetSeries = targetChartView.createSeries(ChartView.SeriesTypeLine, droppedLabel, xAxis, yAxis)
                                    targetSeries.color = droppedSeries.color

                                    for(var i = 0; i < droppedSeries.count; i++)
                                    {
                                        var point = droppedSeries.at(i);
                                        var x = point.x;
                                        var y = point.y;

                                        targetSeries.append(x, y)
                                    }

                                    targetChartView.seriesMap[droppedLabel] = targetSeries
                                }
                            }

                            onEntered:
                            {
                                if(drag.source !== chartView)
                                {
                                    chartView.backgroundColor = "lightskyblue"
                                    chartView.opacity = 0.5
                                    for(let i=0;i<2;i++)
                                    {
                                        chartView.axes[i].visible = false
                                        chartView.axes[i].tickCount = 0
                                    }
                                }

                            }

                            onExited:
                            {
                                chartView.backgroundColor = "white"
                                chartView.opacity = 1

                                for(let i=0;i<2;i++)
                                {
                                    chartView.axes[i].visible = true
                                    chartView.axes[i].tickCount = 3
                                }
                            }
                        }
                    } //Item

                    Rectangle
                    {
                        id: timeMarker
                        width: 2
                        opacity: 0.5
                        z: 100
                        x: 0
                        y: 20
                        height: chartView.height - 40
                        color: "gray"
                    }

                    function updateTimeMarker(timeMs)
                    {
                        // mapToPosition: вычисляем X в координатах графика
                        var pos = chartView.mapToPosition(Qt.point(timeMs, 0))
                        timeMarker.x = pos.x
                    }
                }
            }
        }
    }
}
