import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "./MaterialDesign.js" as MD

Rectangle 
{
    id: sessionPlayerPanel
    width: parent.width
    //height: 75
    implicitHeight: playPauseButton.implicitHeight
    color: "#f8f9fa"
    border.color: "transparent"
    //border.width: 1
    radius: 4
    
    property alias isPlaying: playPauseButton.isPlaying
    property alias currentPosition: positionSlider.value
    property alias maxPosition: positionSlider.to
    
    // Привязки к DriverDataPlayer
    Binding
    {
        target: playPauseButton
        property: "isPlaying"
        value:  playerModel ?  playerModel.isPlaying : false
        when:  playerModel !== null
    }
    
    signal playClicked()
    signal pauseClicked()
    signal stopClicked()
    signal positionChanged(real position)
    
    RowLayout 
    {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15
        
        // Кнопка воспроизведения/паузы
        Button 
        {
            id: playPauseButton
            property bool isPlaying: false
            
            Layout.preferredWidth: 80
            Layout.preferredHeight: 40
            
            text: isPlaying ? qsTr("Pause") : qsTr("Play")
            font.pixelSize: 14

            visible: true // playerModel ? playerModel.isPlayable() : false

            background: Rectangle 
            {
                color: parent.pressed ? 
                       (playPauseButton.isPlaying ? "#ffc107" : "#28a745") :
                       (playPauseButton.isPlaying ? "#ffc107" : "#28a745")
                radius: 4
            }
            
            contentItem: Text 
            {
                text: MD.icons.play_circle_outline
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                //font.pixelSize: 20// parent.font.pixelSize
                font.pointSize: 22
                font.bold: true
            }
            
            onClicked: 
            {
                if (isPlaying) 
                {
                    playerModel.pause()
                }
                else 
                {
                    playerModel.play()
                }
            }
        }
        
        // Кнопка остановки
        Button 
        {
            id: stopButton
            
            Layout.preferredWidth: 80
            Layout.preferredHeight: 40
            
            text: qsTr("Stop")
            font.pixelSize: 14

            visible: playerModel ? playerModel.isPlayable() : false

            background: Rectangle 
            {
                color: parent.pressed ? "steelblue" : "slategray"
                radius: 4
            }
            
            contentItem: Text 
            {
                text: MD.icons.stop
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 22
                font.bold: true
            }
            
            onClicked: 
            {
                sessionPlayerPanel.stopClicked()
            }
        }
        
        // Слайдер позиции
        Slider 
        {
            id: positionSlider
            Layout.fillWidth: true
            Layout.fillHeight: true
            //live: false
            
            from: 0
            to:  playerModel ?  playerModel.sessionDuration : 100
            value:  playerModel ?  playerModel.elapsedTime : 0
            stepSize: 0.1
            
            onValueChanged: 
            {
                //playerModel.setPosition(value)
                //sessionPlayerPanel.positionChanged(value)
            }

            onMoved:
            {
                playerModel.setElapsedTime(value)
                console.log(value)
                console.log(playerModel.elapsedTime)
            }
        }
        
        // Временные метки
        RowLayout 
        {
            Layout.fillWidth: true
            
            Text 
            {
                text: Qt.formatTime(new Date(positionSlider.value * 1000), "mm:ss") + "/"
                font.pointSize: 11
                color: "dimgray"
            }
            
            Text 
            {
                text: Qt.formatTime(new Date(positionSlider.to * 1000), "mm:ss")
                font.pointSize: 11
                color: "dimgray"
            }
        }
    }
}
