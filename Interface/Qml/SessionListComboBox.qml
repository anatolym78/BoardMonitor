import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "./MaterialDesign.js" as MD

ComboBox
{
    id: comboBox
    model: sessionsListModel

    background: null
    contentItem: null

    delegate: ItemDelegate
    {
        width: comboBox.width
        highlighted: index === comboBox.currentIndex
        height: 72
        spacing: 10
        Rectangle
        {
            anchors.fill: parent
            radius: 4
            color: "aliceblue"
            RowLayout
            {
                anchors.fill: parent
                spacing: 10
                // content
                ColumnLayout
                {
                    spacing: 10
                    // top row (name + live indicator)
                    RowLayout
                    {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        // live session indicator (hidded for recorded session)
                        Text
                        {
                            Layout.alignment: Qt.AlignHCenter
                            text: isLiveSession ? MD.icons.fiber_smart_record : MD.icons.bookmark
                            color: isLiveSession ? "red" : "steelblue"
                            font.pointSize: 20
                        }

                        // session name
                        Text
                        {
                            text: sessionName
                            font.pointSize: 12
                            font.bold: true
                            color: "dimgray"
                        }
                        Item
                        {
                            Layout.fillWidth: true
                        }

                    }
                    // bottom layout (creation date + messages count)
                    RowLayout
                    {
                        spacing: 10
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Text
                        {
                            text: createdAtFormatted
                            font.pointSize: 11
                            color: "dimgray"
                        }
                        Item
                        {
                            Layout.fillWidth: true
                        }
                        Text
                        {
                            text: qsTr("Count messages: ") + messageCount
                            font.pointSize: 11
                            color: "dimgray"

                        }
                    }
                }
            }
        }
    }

    indicator: Item
    {
        anchors.fill: parent
        implicitHeight: 72// parent.height

        Rectangle
        {
            anchors.fill: parent
            radius: 4
            color: "gainsboro"
            RowLayout
            {
                anchors.fill: parent
                spacing: 10
                // content
                ColumnLayout
                {
                    spacing: 10
                    // top row (name + live indicator)
                    RowLayout
                    {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        // live session indicator (hidded for recorded session)
                        Text
                        {
                            Layout.alignment: Qt.AlignHCenter
                            text: sessionsListModel.get(comboBox.currentIndex).isLiveSession ?
                                      MD.icons.fiber_smart_record : MD.icons.bookmark
                            color: sessionsListModel.get(comboBox.currentIndex).isLiveSession ? "red" : "steelblue"
                            font.pointSize: 20
                        }

                        // session name
                        Text
                        {
                            text: sessionsListModel.get(comboBox.currentIndex).sessionName
                            font.pointSize: 12
                            font.bold: true
                            color: "dimgray"
                        }
                        Item
                        {
                            Layout.fillWidth: true
                        }

                    }
                    // bottom layout (creation date + messages count)
                    RowLayout
                    {
                        spacing: 10
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Text
                        {
                            text: sessionsListModel.get(comboBox.currentIndex).createdAtFormatted
                            font.pointSize: 11
                            color: "dimgray"
                        }
                        Item
                        {
                            Layout.fillWidth: true
                        }
                        Text
                        {
                            text: qsTr("Count messages: ") + sessionsListModel.get(comboBox.currentIndex).messageCount
                            font.pointSize: 11
                            color: "dimgray"

                        }
                    }
                }

                // pupup button
                Text
                {
                    Layout.alignment: Qt.AlignRight
                    text: popup.opened ? MD.icons.expand_less : MD.icons.expand_more
                    font.pointSize: 24
                    color: "dimgray"
                }
            }
        }
    }
}
