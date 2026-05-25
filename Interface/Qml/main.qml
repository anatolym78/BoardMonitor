import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.12
import QtCharts 2.12
import BoardStation 1.0

Item
{
    id: mainWindow
    width: 1400
    height: 960

    RowLayout
    {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // LeftPanel
        Rectangle
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 3
            color: "transparent"

            StackLayout
            {
                id: sessionsDocumentList
                anchors.fill: parent
                currentIndex: sessionsList.currentIndex

                Connections
                {
                    target: sessionsList

                    onCurrentIndexChanged:
                    {
                        sessionsListModel.selectSession(sessionsList.currentIndex)
                    }
                }

                Repeater
                {
                    model: sessionsListModel
                    delegate: Item
                    {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        ColumnLayout
                        {
                            anchors.fill: parent
                            RowLayout
                            {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                Layout.preferredHeight: 300

                                BoardParametersTree
                                {
                                    id: boardParametersList
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    Layout.preferredWidth: 1

                                    treeModel: parametersModel

                                    // onParameterSelected:
                                    // {
                                    //     chartModel.toggleParameter(label, color)
                                    // }
                                }

                                ChartsView
                                {
                                    id: chartsView
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    Layout.preferredWidth: 3
                                }
                            }

                            SessionPlayerPanel
                            {
                                id: sessionPlayerPanel
                                Layout.fillWidth: true
                                Layout.fillHeight: false
                                Layout.preferredHeight: implicitHeight
                                Layout.minimumHeight: 70
                            }
                        }

                    }
                }
            }
        }

        // Right panel
        Rectangle
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 1
            color: "transparent"

            ColumnLayout
            {
                anchors.fill: parent

                SessionListComboBox
                {
                    id: sessionsList
                    Layout.fillWidth: true
                }

                // Панель управления параметрами дрона
                DroneControlPanel
                {
                    id: droneControlPanel
                    Layout.preferredHeight: 10
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                }
            }
        }
    }
}

