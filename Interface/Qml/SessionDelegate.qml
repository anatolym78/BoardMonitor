import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15


ItemDelegate
{

    implicitHeight: 100
    width: 300
    Rectangle
    {
        id: listItemRect
        //height: 70
        radius: 4
        anchors.margins: 2
        color: "red"
        border.color: "blue"
        border.width: 1


        ColumnLayout
        {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 8

            // Название и состоянии сессии
            RowLayout
            {
                Layout.fillWidth: true

                // Красный кружок для живой сессии
                Rectangle
                {
                    id: recordingIndicator
                    width: 10
                    height:  10
                    radius: 5
                    color: "red"
                    visible: true
                }

                // Название сесии
                Text
                {
                    Layout.fillWidth: true
                    text: "Live session"// sessionName
                    font.pixelSize: 14
                    font.bold: true
                    color: isDisabled ? "#999999" : "#333333"
                    elide: Text.ElideRight
                }
            }

            // Информация о сессии
            RowLayout
            {
                Layout.fillWidth: true

                // Время создания сессии
                Text
                {
                    text: "13.09.2025"// qsTr("Created: ") + createdAtFormatted
                    font.pixelSize: 11
                    color: isDisabled ? "#bbbbbb" : "#666666"
                }

                Item
                {
                    Layout.fillWidth: true
                }

                // Количество сообщения в сессии
                Text
                {
                    text: "10" // messageCount + " " + qsTr("messages")
                    font.pixelSize: 11
                    color: isDisabled ? "#bbbbbb" : "#666666"
                }
            }
        }
    }
}
