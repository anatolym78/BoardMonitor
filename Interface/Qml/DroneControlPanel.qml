import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Rectangle
{
    id: droneControlPanel

    color: "#f0f0f0"  // Тот же фон, что и у левой панели
    radius: 4

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        Text
        {
            text: qsTr("Drone Control")
            font.pointSize: 12
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
            color: "gray"
        }
        
        TableView
        {
            id: outParametersTableView
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter
            //anchors.left: parent.left
            //anchors.right: parent.right
            clip: true
            
            model: uplinkParametersModel

            property int lastColumnWidth: width - 195

            columnWidthProvider: function(column)
            {
                switch (column)
                {
                case 0: return 120
                case 1: return 80
                case 2: return outParametersTableView.width - 195
                default: return 100
                }
            }
            delegate: Rectangle
            {
                implicitHeight: 45
                color: "transparent"
                Rectangle
                {
                    id: cell
                    anchors.fill: parent
                    anchors.centerIn: parent
                    anchors.margins: 4
                    border.color: "transparent"
                    border.width: 2
                    radius: 2

                    color:
                    {
                        switch(column)
                        {
                        case 0: return Qt.hsva(0.6, 0.3, 0.4, 0.7)// return Qt.hsva(0.50, 0.5, 0.5, 0.7)
                        case 1: return "white"
                        case 2: return "white"
                        default: return "white"
                        }
                    }

                    // Different controls based on column
                    Loader
                    {
                        anchors.fill: parent
                        anchors.margins: 5

                        sourceComponent:
                        {
                            switch(column)
                            {
                                case 0:
                                    // Label column - just text
                                    return labelComponent
                                case 1:
                                    // Value column - text
                                    return valueComponent
                                case 2:
                                    // Control column - dynamic control based on parameter type
                                    return dynamicControlComponent
                                default:
                                    // Other columns - text
                                    return textComponent
                            }
                        }
                    }

                    Component
                    {
                        id: labelComponent
                        Item
                        {
                            Text
                            {
                                anchors.centerIn: parent
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                text: parameterLabel
                                font.pointSize: 11
                                color:
                                {
                                    switch(column)
                                    {
                                    case 0:
                                        return "white";
                                    }

                                    return "dimgray";
                                }

                                elide: Text.ElideRight
                            }
                        }
                    }

                    Component
                    {
                        id: valueComponent
                        Text
                        {
                            anchors.centerIn: parent
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: "dimgray"
                            text: parameterValue
                            font.pointSize: 11
                            elide: Text.ElideRight
                        }
                    }

                    Component
                    {
                        id: textComponent
                        Text
                        {
                            anchors.centerIn: parent
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: "dimgray"
                            text: parameterLabel
                            font.pointSize: 11
                            elide: Text.ElideRight
                        }
                    }

                    Component
                    {
                        id: dynamicControlComponent
                        Item
                        {
                            Loader
                            {
                                anchors.fill: parent
                                sourceComponent:
                                {
                                    if(dataControl == null) return defaultComponent

                                    switch(dataControl.controlType)
                                    {
                                        case "SpinBox": return spinBoxComponent
                                        case "Slider": return sliderComponent
                                        case "ComboBox": return comboBoxComponent
                                        case "CheckBox": return checkBoxComponent
                                        case "Switch": return switchComponent
                                        case "TextEdit": return textEditComponent
                                    }

                                    return defaultComponent
                                }
                            }
                            Component
                            {
                                id: spinBoxComponent
                                SpinBox
                                {
                                    anchors.fill: parent
                                    font.pointSize: 11
                                    from: dataControl ? (dataControl.range && dataControl.range.length > 0 ? dataControl.range[0] : 0) : 0
                                    to: dataControl ? (dataControl.range && dataControl.range.length > 1 ? dataControl.range[1] : 100) : 100
                                    stepSize: dataControl ? (dataControl.step ? dataControl.step : 1) : 1  // Используем шаг из параметра
                                    value: dataControl ? dataControl.currentValue : 0
                                    onValueChanged:
                                    {
                                        parameterValue = value
                                    }
                                }
                            }

                            Component
                            {
                                id: sliderComponent
                                Slider
                                {
                                    anchors.fill: parent
                                    font.pointSize: 11
                                    from: dataControl ? (dataControl.range && dataControl.range.length > 0 ? dataControl.range[0] : 0) : 0
                                    to: dataControl ? (dataControl.range && dataControl.range.length > 1 ? dataControl.range[1] : 100) : 100
                                    stepSize: dataControl ? (dataControl.step ? dataControl.step : 1) : 1  // Используем шаг из параметра
                                    value: dataControl ? dataControl.currentValue : 0
                                    onValueChanged:
                                    {
                                        parameterValue = value
                                    }
                                }
                            }

                            Component
                            {
                                id: comboBoxComponent
                                ComboBox
                                {
                                    anchors.fill: parent
                                    font.pointSize: 11
                                    model: dataControl ? dataControl.values : []
                                    onCurrentValueChanged:
                                    {
                                        parameterValue = currentValue
                                    }
                                }
                            }
                            Component
                            {
                                id: checkBoxComponent
                                CheckBox
                                {
                                    anchors.centerIn: parent
                                    font.pointSize: 11
                                    checked: paramData.value// paramData ? paramData.currentValue : false
                                    onCheckedChanged:
                                    {
                                        parameterValue = checkState
                                    }
                                }
                            }

                            Component
                            {
                                id: switchComponent
                                Switch
                                {
                                    anchors.centerIn: parent
                                    font.pointSize: 11
                                    checked: dataControl ? dataControl.currentValue : false
                                    onCheckedChanged:
                                    {
                                        parameterValue = checked
                                    }
                                }
                            }

                            Component
                            {
                                id: textEditComponent
                                TextField
                                {
                                    anchors.fill: parent
                                    text: paramData ? paramData.currentValue : ""
                                    font.pointSize: 11
                                    onTextChanged:
                                    {
                                        parameterValue = text
                                    }
                                }
                            }

                            Component
                            {
                                id: defaultComponent
                                Text {
                                    anchors.centerIn: parent
                                    text: "No control"
                                    color: "#666666"
                                }
                            }
                        }
                    }
                }

            }
        }
        
        // Кнопки управления (перемещены под таблицу)
        RowLayout
        {
            Layout.fillWidth: true
            spacing: 10
            
            Button
            {
                text: qsTr("Send to drone")
                font.pointSize: 10
                Layout.preferredWidth: 120
                onClicked: 
                {
                    // Вызываем C++ метод через объект QmlMainWindow
                    if (qmlMainWindow) 
                    {
                        qmlMainWindow.sendParametersToBoard()
                    }
                }
            }
            
            CheckBox
            {
                text: qsTr("Send immediately")
                checked: false
                font.pointSize: 10
                onCheckedChanged: 
                {
                    // Здесь будет логика автоматической отправки
                }
            }
            
            Item
            {
                Layout.fillWidth: true
            }
        }
    }
}
