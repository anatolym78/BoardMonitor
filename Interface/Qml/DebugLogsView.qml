import QtQuick 2.12
import QtQuick.Controls 2.12

ScrollView
{
    id: debugScrollView
    anchors.fill: parent
    anchors.margins: 4
    
    property alias model: debugListView.model

    clip: true
    
    ListView
    {
        id: debugListView
        anchors.fill: parent
        spacing: 4
        
        // Автоматическая прокрутка к концу при добавлении новых элементов
        onCountChanged:
        {
            if (count > 0)
            {
                positionViewAtEnd()
            }
        }
        
        delegate: Rectangle
        {
            id: delegateRect
            width: debugListView.width
            height: messageText.implicitHeight + 8
            radius: 4
            color: "gainsboro"
            
            Text
            {
                id: messageText
                anchors.fill: parent
                anchors.margins: 4
                text: model.formattedMessage || ""
                font.pointSize: 9
                color: "dimgray"
                verticalAlignment: Text.AlignTop
                wrapMode: Text.Wrap
                clip: true
            }
        }
    }
}
