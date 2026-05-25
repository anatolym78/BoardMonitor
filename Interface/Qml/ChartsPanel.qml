import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtCharts 2.15

Item
{
    id: chartsPanel
    
    // Функция для добавления нового графика
    function addChart(parameterLabel) 
    {
        console.log("ChartsDashboardView: Adding chart for parameter:", parameterLabel)
        if (chartsListModel) 
        {
            chartsListModel.addSeries(parameterLabel)
            console.log("ChartsDashboardView: Chart added successfully for", parameterLabel)
        }
        else 
        {
            console.warn("ChartsDashboardView: chartsListModel is not available")
        }
    }

    // ScrollView
    // {
    //     Layout.fillWidth: true
    //     Layout.fillHeight: true
    //     //clip: true
    //     anchors.fill: parent

    ScrollView
    {
        anchors.fill: parent
        clip: true

        GridLayout
        {
            id: chartsLayout
            width: parent.width
            columns: Math.max(1, Math.floor(width / (320 + 10)))
            spacing: 10

            Repeater
            {
                model: chartsListModel
                id: repeater

                // График для серии
                delegate: ChartView
                {
                    Component.onCompleted:
                    {
                    }

                    id: chartView
                    property var seriesModel: model.seriesModel
                    property var chartIndex: model.index
                    width: 320
                    height: 240
                    //theme: ChartView.ChartThemeDark
                    //backgroundColor: "blue"
                    property bool highlight: false
                    opacity: highlight ? 0.8 : 1.0
                    //z: depth
                    
                    ValueAxis
                    {
                        id: parameterValueAxis
                        min: -750
                        max: 750
                    }

                    ValueAxis
                    {
                        id: timeAxis
                        min: 0
                        max: 50
                    }

                    axes: [parameterValueAxis, timeAxis]

                    antialiasing: true



                    // Карта для хранения серий по меткам параметров
                    property var seriesMap: ({})

                    Connections
                    {
                        target: seriesModel

                        function onParameterValueAdded(label)
                        {
                            var chartSeries
                            var pointsModel = seriesModel.getPointsModel(label)

                            // Проверяем, есть ли уже серия для данной метки
                            if (!seriesMap.hasOwnProperty(label))
                            {
                                // Создаем новую серию для этой метки
                                chartSeries = createSeries(ChartView.SeriesTypeLine, label, timeAxis, parameterValueAxis)
                                seriesMap[label] = chartSeries
                                
                                if (pointsModel)
                                {
                                    var color = pointsModel.pointsColor()
                                    chartSeries.color = color
                                }
                            }
                            else
                            {
                                // Используем существующую серию для этой метки
                                chartSeries = seriesMap[label]
                            }
                            
                            // Добавляем точку в соответствующую серию
                            if (pointsModel && chartSeries)
                            {
                                chartSeries.append(pointsModel.elapsedTime(), pointsModel.lastY())

                                //console.log(chartSeries)
                                //console.log(chartSeries.count)

                                if(chartSeries.count > 10)
                                {
                                    //chartSeries.clear();
                                    chartSeries.remove(0)
                                }

                                console.log(timeAxis)

                                timeAxis.min = pointsModel.elapsedTime()
                                console.log(timeAxis.min)
                                timeAxis.max = timeAxis.min + 50


                                //if(chartSeries)
                            }
                        }
                    }


                    DragHandler
                    {
                        id: dragHandler
                        onActiveChanged:
                        {
                            if(active)
                            {
                                // Начало перетаскивания
                                chartView.Drag.start()
                            }
                            else
                            {
                                // Завершение перетаскивания
                                chartView.Drag.drop()
                            }
                        }
                    }

                    Drag.active: dragHandler.active
                    Drag.hotSpot.x: width / 2
                    Drag.hotSpot.y: height / 2
                    Drag.mimeData:
                    {
                        "text/plain": "dragged-object",  // Указываем тип данных
                        "application/x-series-model": seriesModel, // Передаем модель
                        "model-index": index // Передаем индекс в repeater
                    }
                    Drag.dragType: Drag.Automatic

                    DropArea
                    {
                        id: dropArea
                        anchors.fill: parent
                        enabled: true

                        onEntered:
                        {
                            return
                        }

                        onExited:
                        {
                            return
                        }

                        onDropped:
                        {
                            if(false)
                            {
                                console.log("=== DROP COMPLETED ===")
                                console.log("Target chart index:", index)
                                console.log("Target chart chartIndex:", chartView.chartIndex)
                                console.log("Target chart z:", chartView.z)
                                console.log("Target chart depth:", depth)

                                // Проверяем, не является ли цель самим источником
                                if (drop.source && drop.source.chartIndex === chartView.chartIndex) {
                                    console.log("*** SELF-DROP DETECTED IN DROP - IGNORING ***")
                                    return
                                }

                                if(chartsListModel.rowCount() < 2) {
                                    console.log("Not enough charts to merge")
                                    return
                                }

                                console.log("Drop source:", drop.source)
                                if (drop.source) {
                                    console.log("Source chart index:", drop.source.chartIndex)
                                    console.log("Source chart z:", drop.source.z)
                                }

                                console.log("Drop mimeData keys:", Object.keys(drop.mimeData))
                                console.log("Drop mimeData model-index:", drop.mimeData["model-index"])
                                console.log("Drop mimeData text:", drop.mimeData["text/plain"])

                                // Альтернативный способ определения цели через координаты
                                var mousePos = drop.position
                                console.log("Drop position (local):", mousePos)

                                // Глобальные координаты мыши
                                var globalMousePos = Qt.point(mousePos.x + chartView.x, mousePos.y + chartView.y)
                                console.log("Drop position (global):", globalMousePos)

                                // Находим элемент под курсором мыши
                                var targetChart = chartsLayout.childAt(mousePos.x, mousePos.y)
                                console.log("Chart under mouse:", targetChart)

                                // Альтернативный способ - поиск через все дочерние элементы
                                var chartsUnderMouse = []
                                for (var i = 0; i < chartsLayout.children.length; i++) {
                                    var child = chartsLayout.children[i]
                                    if (child.chartIndex !== undefined) {
                                        var childRect = Qt.rect(child.x, child.y, child.width, child.height)
                                        if (childRect.contains(mousePos)) {
                                            chartsUnderMouse.push(child)
                                            console.log("Chart", child.chartIndex, "contains mouse position")
                                        }
                                    }
                                }
                                console.log("All charts under mouse:", chartsUnderMouse.length)

                                // Определяем индексы
                                var sourceIndex = drop.source.chartIndex
                                var targetIndex = chartView.chartIndex

                                // Если есть альтернативный способ определения цели, используем его
                                if (targetChart && targetChart.chartIndex !== undefined) {
                                    console.log("Using alternative target detection")
                                    console.log("Alternative target index:", targetChart.chartIndex)
                                    // Можно использовать targetChart.chartIndex вместо chartView.chartIndex
                                }

                            }

                            return

                            console.log("Final source index:", sourceIndex)
                            console.log("Final target index:", targetIndex)

                            //return

                            // Используем новый метод mergeSeries из ChartsListModel
                            chartsListModel.mergeSeries(0, 1)
                            
                            // Очищаем карту серий для пересоздания после слияния
                            chartView.seriesMap = ({})
                            
                            console.log("Series merged successfully")
                        }
                    }
                }
            }
        }
    }
    //}
}
