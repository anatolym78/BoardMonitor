import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle
{
    id: sessionsComboBox
    width: parent.width
    height: 200
    color: "#f0f0f0"
    radius: 5

    property alias sessionsModel: sessionsList.model
    property alias currentIndex: sessionsList.currentIndex
    property int currentRecordingSessionId: -1

    signal sessionSelected(int sessionId, string sessionName)
    signal refreshRequested()
    signal sessionChanged(int index)

    // Диалог подтверждения удаления
    property alias deleteDialog: deleteConfirmationDialog

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Заголовок панели
        RowLayout
        {
            Layout.fillWidth: true

            Text
            {
                text: qsTr("Sessions")
                font.pixelSize: 16
                font.bold: true
                color: "dimgray"
            }
        }

        // Список сессий
        ComboBox
        {
            id: sessionsList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 4
            clip: true
            // indicator: null
            // popup: Popup
            // {
            //     height: 100
            // }

            // background: Rectangle
            // {
            //     color: "lightgray"
            //     radius: 4
            //     height: 100  // теперь можем задать высоту
            // }
            model: sessionsListModel

            onCurrentIndexChanged:
            {
                sessionsListModel.selectSession(currentIndex)
            }

            delegate: Rectangle
            {
                id: listItemRect
                //width: sessionsList.width
                height: 100
                radius: 16
                border.color: "transparent"
                border.width: 2
                anchors.margins: 2
                color: "red"

                // Определяем цвета в зависимости от состояния
                property bool isSelected: sessionsList.currentIndex === index

                states:
                [
                    State
                    {
                        name: "hovered"
                        PropertyChanges
                        {
                            target: listItemRect;
                            color: Qt.hsva(0.15, 0.99, 0.99, 0.3)
                        }
                    }
                ]

                MouseArea
                {
                    anchors.fill: parent
                    hoverEnabled: true
                    enabled: true

                    onClicked:
                    {
                        sessionsList.currentIndex = index

                        if(qmlMainWindow)
                        {
                            qmlMainWindow.changeSession()
                        }

                        sessionChanged(index)
                    }

                    onEntered:
                    {
                        listItemRect.state = "hovered"
                    }

                    onExited:
                    {
                        listItemRect.state = ""
                    }
                }

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
                            height: 10
                            radius: 5
                            color: "red"
                            visible: isLiveSession
                        }

                        // Название сесии
                        Text
                        {
                            Layout.fillWidth: true
                            text: sessionName
                            font.pixelSize: 14
                            font.bold: true
                            color: "gainsboro" // isDisabled ? "#999999" : "#333333"
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
                            text: qsTr("Created: ") + createdAtFormatted
                            font.pixelSize: 11
                            color: "gainsboro"// isDisabled ? "#bbbbbb" : "#666666"
                        }

                        Item
                        {
                            Layout.fillWidth: true
                        }

                        // Количество сообщения в сессии
                        Text
                        {
                            text: messageCount + " " + qsTr("messages")
                            font.pixelSize: 11
                            color:"gainsboro"// isDisabled ? "#bbbbbb" : "#666666"
                        }
                    }
                }
            }
        }
    }

    // Диалог подтверждения удаления
    Dialog
    {
        id: deleteConfirmationDialog
        title: qsTr("Подтверждение удаления")
        width: 400
        height: 200
        modal: true

        property int sessionIndex: -1
        property string sessionName: ""

        ColumnLayout
        {
            anchors.fill: parent
            spacing: 20

            Text
            {
                Layout.fillWidth: true
                text: qsTr("Вы уверены, что хотите удалить сессию \"%1\"?").arg(deleteConfirmationDialog.sessionName)
                wrapMode: Text.WordWrap
                font.pixelSize: 14
                color: "#333333"
            }

            Text
            {
                Layout.fillWidth: true
                text: qsTr("Это действие нельзя отменить. Все данные сессии будут удалены навсегда.")
                wrapMode: Text.WordWrap
                font.pixelSize: 12
                color: "#666666"
            }

            RowLayout
            {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
                spacing: 10

                Button
                {
                    text: qsTr("Отмена")
                    onClicked: deleteConfirmationDialog.close()

                    background: Rectangle
                    {
                        color: parent.pressed ? "#e0e0e0" : "#f0f0f0"
                        radius: 4
                        border.color: "#cccccc"
                        border.width: 1
                    }

                    contentItem: Text
                    {
                        text: parent.text
                        color: "#333333"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Button
                {
                    text: qsTr("Удалить")
                    onClicked:
                    {
                        if (deleteConfirmationDialog.sessionIndex >= 0)
                        {
                            sessionsListModel.removeSession(deleteConfirmationDialog.sessionIndex)
                        }
                        deleteConfirmationDialog.close()
                    }

                    background: Rectangle
                    {
                        color: parent.pressed ? "#d32f2f" : "#f44336"
                        radius: 4
                    }

                    contentItem: Text
                    {
                        text: parent.text
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.bold: true
                    }
                }
            }
        }
    }
}
