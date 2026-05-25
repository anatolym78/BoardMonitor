import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle 
{
    id: sessionsPanel
    width: parent.width
    height: 200
    color: "#f0f0f0"
    radius: 5
    
    property alias sessionsModel: sessionsList.model
    property alias currentIndex: sessionsList.currentIndex
    property int currentRecordingSessionId: -1
    
    signal sessionSelected(int sessionId, string sessionName)
    signal refreshRequested()
    
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
        //ListView ComboBox
        ListView
        {
            id: sessionsList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 4
            clip: true
            highlight: Rectangle
            {
                // width: 50
                // height: 25
                radius: 2
                color: Qt.hsva(0.6, 0.99, 0.99, 0.2)
            }

            onCurrentIndexChanged:
            {
                sessionsListModel.selectSession(currentIndex)
            }
            
            delegate: Rectangle 
            {
                id: listItemRect
                width: sessionsList.width
                height: 70
                radius: 4
                border.color: "transparent"
                border.width: 2
                anchors.margins: 2
                color: "transparent"
                
                // Определяем цвета в зависимости от состояния
                property bool isSelected: sessionsList.currentIndex === index
                
                // Отладочная информация
                Component.onCompleted:
                {
                    console.log(model)
                }
                
                states:
                [
                    State
                    {
                        name: "hovered"
                        PropertyChanges
                        {
                            target: listItemRect;
                            //border.color: Qt.hsva(0.6, 0.99, 0.99, 0.3)
                            color: Qt.hsva(0.15, 0.99, 0.99, 0.3)
                        }
                    }
                ]

                MouseArea
                {
                    anchors.fill: parent
                    hoverEnabled: true
                    enabled: !isDisabled
                    
                    onClicked:
                    {
                        sessionsList.currentIndex = index
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
                    
                    RowLayout
                    {
                        Layout.fillWidth: true

                        // Красный кружок для текущей записываемой сессии
                        Rectangle
                        {
                            id: recordingIndicator
                            width: 10
                            height: 10
                            radius: 5
                            color: "red"
                            visible: isLiveSession
                        }

                        Text
                        {
                            Layout.fillWidth: true
                            text: sessionName
                            font.pixelSize: 14
                            font.bold: true
                            color: isDisabled ? "#999999" : "#333333"
                            elide: Text.ElideRight
                        }

                        // // Кнопка Reset для живой сессии
                        // Button
                        // {
                        //     id: resetButton
                        //     Layout.alignment: Qt.AlignRight
                        //     visible: isLiveSession

                        //     background: Rectangle
                        //     {
                        //         id: resetButtonBackground
                        //         height: 24
                        //         color: Qt.hsva(0.1, 0.8, 0.8, 0.7)
                        //         opacity: 0.9
                        //         radius: 3
                        //         states:
                        //         [
                        //             State
                        //             {
                        //                 name: "resetButtonHovered"
                        //                 PropertyChanges
                        //                 {
                        //                     target: resetButtonBackground
                        //                     color: Qt.hsva(0.1, 0.8, 0.9, 0.9)
                        //                     opacity: 1
                        //                 }
                        //             },
                        //             State
                        //             {
                        //                 name: "resetButtonPressed"
                        //                 PropertyChanges
                        //                 {
                        //                     target: resetButtonBackground
                        //                     color: Qt.hsva(0.1, 0.9, 0.7, 0.9)
                        //                     opacity: 1
                        //                 }
                        //             }
                        //         ]

                        //         MouseArea
                        //         {
                        //             anchors.fill: parent
                        //             hoverEnabled: true
                        //             onEntered:
                        //             {
                        //                 resetButtonBackground.state = "resetButtonHovered"
                        //             }

                        //             onExited:
                        //             {
                        //                 resetButtonBackground.state = ""
                        //             }

                        //             onPressed:
                        //             {
                        //                 resetButtonBackground.state = "resetButtonPressed"
                        //             }

                        //             onReleased:
                        //             {
                        //                 resetButtonBackground.state = "resetButtonHovered"
                        //             }
                        //             onClicked:
                        //             {
                        //                 if(sessionsListModel)
                        //                 {
                        //                     sessionsListModel.resetLiveSession()
                        //                 }

                        //             }
                        //         }
                        //     }

                        //     contentItem: Text
                        //     {
                        //         text: qsTr("reset")
                        //         color: "white"
                        //         font.bold: true
                        //     }
                        // }

                        // // Кнопка Save/Delete
                        // Button
                        // {
                        //     id: actionButton
                        //     Layout.alignment: Qt.AlignRight
                        //     visible: true

                        //     background: Rectangle
                        //     {
                        //         id: actionButtonBackground
                        //         height: 24
                        //         color: Qt.hsva(0.6, 0.3, 0.4, 0.7)
                        //         opacity: 0.9
                        //         radius: 3
                        //         states:
                        //         [
                        //             State
                        //             {
                        //                 name: "actionButtonHovered"
                        //                 PropertyChanges
                        //                 {
                        //                     target: actionButtonBackground
                        //                     color: "dimgray"
                        //                     opacity: 1
                        //                 }
                        //             },
                        //             State
                        //             {
                        //                 name: "actionButtonPressed"
                        //                 PropertyChanges
                        //                 {
                        //                     target: actionButtonBackground
                        //                     color:
                        //                     {
                        //                         if(isLiveSession)
                        //                         {
                        //                             return Qt.hsva(0.35, 0.90, 0.80, 0.9)
                        //                         }
                        //                         else
                        //                         {
                        //                             return Qt.hsva(0.025, 0.90, 0.80, 0.9)
                        //                         }
                        //                     }
                        //                     opacity: 1
                        //                 }
                        //             }
                        //         ]

                        //         MouseArea
                        //         {
                        //             anchors.fill: parent
                        //             hoverEnabled: true
                        //             onEntered:
                        //             {
                        //                 actionButtonBackground.state = "actionButtonHovered"
                        //             }

                        //             onExited:
                        //             {
                        //                 actionButtonBackground.state = ""
                        //             }

                        //             onPressed:
                        //             {
                        //                 actionButtonBackground.state = "actionButtonPressed"
                        //             }

                        //             onReleased:
                        //             {
                        //                 actionButtonBackground.state = "actionButtonHovered"
                        //             }
                        //             onClicked:
                        //             {
                        //                 if(isLiveSession)
                        //                 {
                        //                     console.log(qmlMainWindow)
                        //                     qmlMainWindow.saveLiveData()
                        //                 }
                        //                 else
                        //                 {
                        //                     // Показываем диалог подтверждения
                        //                     deleteConfirmationDialog.sessionIndex = index
                        //                     deleteConfirmationDialog.sessionName = sessionName
                        //                     deleteConfirmationDialog.open()
                        //                 }
                        //             }
                        //         }
                        //     }

                        //     contentItem: Text
                        //     {
                        //         text:
                        //         {
                        //             return isLiveSession ? qsTr("save") : qsTr("delete")
                        //         }
                        //         color: "white"
                        //         font.bold: true
                        //     }
                        // }
                    }

                    RowLayout 
                    {
                        Layout.fillWidth: true
                        
                        Text 
                        {
                            text: qsTr("Created: ") + createdAtFormatted
                            font.pixelSize: 11
                            color: isDisabled ? "#bbbbbb" : "#666666"
                        }
                        
                        Item 
                        {
                            Layout.fillWidth: true
                        }
                        
                        Text 
                        {
                            text: messageCount + " " + qsTr("messages")
                            font.pixelSize: 11
                            color: isDisabled ? "#bbbbbb" : "#666666"
                        }
                    }
                }
            }
            
            ScrollBar.vertical: ScrollBar 
            {
                active: true
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
