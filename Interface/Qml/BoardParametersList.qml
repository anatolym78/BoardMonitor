import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import BoardStation 1.0

Rectangle
{
    id: boardParemetersList

    radius: 4

    Component.onCompleted:
    {
        console.log(parametersModel)
        console.log(parametersModel.getCountParameters())
    }

    color: "#f0f0f0"

    //property var parametersListModel: null

    signal parameterSelected(string label, variant color)

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Text
        {
            text: qsTr("Drone Parameters")
            font.pointSize: 12
            font.bold: true
            color: "gray"
            Layout.alignment: Qt.AlignHCenter

            Component.onCompleted:
            {
            }
        }

        ListView
        {
            id: parametersListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter
            clip: false

            model: parametersModel

            // Data delegate
            delegate: Rectangle
            {
                id: parameterCell
                radius: 2
                width: parent.width
                //implicitWidth: 165
                implicitHeight: 40
                //border.width: 2
                border.color: "transparent"
                color: "transparent"
                z: 10

                Component.onCompleted:
                {
                    console.log(parameterColor)
                }

                RowLayout
                {
                    anchors.fill: parent

                    spacing: 4
                    z: 1

                    Rectangle
                    {
                        id: labelCell
                        radius: 2
                        width: 100
                        anchors.left: parent.left
                        anchors.leftMargin: 4
                        anchors.rightMargin: 4
                        anchors.topMargin: 4
                        anchors.bottomMargin: 4
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        color: Qt.hsva(0.6, 0.3, 0.4, 0.7)
                        Text
                        {
                            anchors.centerIn: parent
                            text: label
                            font.pointSize: 11
                            color: "white"
                        }
                    }

                    Rectangle
                    {
                        id: valueCell
                        radius: 2
                        anchors.left: labelCell.right
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.leftMargin: 4
                        anchors.topMargin: 4
                        anchors.bottomMargin: 4
                        anchors.rightMargin: 4
                        color: "white"
                        Text
                        {
                            id: valueText
                            anchors.centerIn: parent
                            color: "dimgray"
                            text: value + " " + unit
                            font.pointSize: 11
                        }

                        Rectangle
                        {
                            id: statusRect
                            radius: 2
                            width: 4
                            anchors.right: parent.right
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.rightMargin: 4
                            anchors.topMargin: 8
                            anchors.bottomMargin: 8
                            opacity: 0.75
                            color:
                            {
                                if(chartVisibility)
                                {
                                    return parameterColor
                                }
                                else
                                {
                                    return "transparent"
                                }
                            }
                        }
                    }
                }

                states:
                [
                    State
                    {
                        name: "hovered"
                        PropertyChanges
                        {
                            target: parameterCell;
                            color: Qt.hsva(0.6, 0.99, 0.99, 0.2)
                        }
                        PropertyChanges
                        {
                            target: labelCell
                            color: Qt.hsva(0.6, 0.99, 0.99, 1.0)
                        }
                        PropertyChanges
                        {
                            target: valueText
                            color: "midnightblue"
                        }
                    }
                ]

                transitions: Transition
                {
                    from: ""
                    to: "hovered"
                    ColorAnimation
                    {
                        duration: 300
                    }
                }

                MouseArea
                {
                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked:
                    {
                        parameterSelected(label, parameterColor)
                        chartVisibility = !chartVisibility
                    }

                    onEntered:
                    {
                        parameterCell.state = "hovered"
                    }

                    onExited:
                    {
                        parameterCell.state = ""

                    }
                }
            }
        }

    }


}
