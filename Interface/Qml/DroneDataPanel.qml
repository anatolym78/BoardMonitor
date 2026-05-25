import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Rectangle
{
    id: droneDataPanel

    color: "#f0f0f0"

    property var parametersModel: null

    signal parameterSelected(string label)
    
    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        
        Text
        {
            text: "Drone Data"
            font.pointSize: 14
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }
           
        TableView
        {
            id: parametersTableView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: false
            
            model: parametersModel

            Component.onCompleted:
            {
                //console.log(parametersModel)
            }
            
            // Data delegate
            delegate: Rectangle 
            {
                id: parameterCell
                implicitWidth: 85
                implicitHeight: 40
                border.width: 2
                border.color: "#f4f4f0"
                //color: row % 2 === 0 ? "#ffffff" : "#f8f8f8"
                
                Text 
                {
                    anchors.centerIn: parent
                    font.pointSize: 11
                    text:
                    {
                        switch (column)
                        {
                            case 0:
                                return label
                            case 1:
                                return value
                            case 2:
                                return unit
                            case 3:
                                return timestamp
                            default:
                                return ""
                        }

                    }
                    //data(parametersModel.index(row, column))
                    elide: Text.ElideRight
                }

                Connections
                {
                    target: parametersModel
                    function onParameterUpdated(label)
                    {

                    }
                }

                Component.onCompleted:
                {
                }
                
                states:
                [
                    State
                    {
                        name: "hovered"
                        PropertyChanges
                        {
                            target: parameterCell;
                            color: "#77ddff"
                        }
                    }
                ]

                transitions: Transition
                {
                    from: ""
                    to: "hovered"
                    ColorAnimation
                    {
                        duration: 250
                    }
                }

                MouseArea 
                {
                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked: 
                    {
                        parameterSelected(label)
                    }

                    onEntered:
                    {
                        if(column == 0)
                        {
                            parameterCell.state = "hovered"
                        }
                    }

                    onExited:
                    {
                        if(column == 0)
                        {
                            parameterCell.state = ""
                        }
                    }
                }
            }
        }
    }
}
