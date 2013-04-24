/*
    Copyright 2012 Viranch Mehta <viranch.mehta@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 1.1

Rectangle {
    id: background

    // API
    property alias text: message.text
    property alias textColor: message.color
    property alias messageTimeout: hideTimer.interval
    property alias icon: icon.iconName
    property alias showIcon: icon.visible
    property alias hideOnMouseClick: mouseArea.enabled
    property alias sharpness: background.radius
    property alias backgroundColor: background.color
    property real messageOpacity: 1
    property bool animate: true
    signal linkActivated(string link)

    // constants
    property int margin: 15
    property int iconTextMargin: showIcon ? 10 : 0

    color: "#181513"
    opacity: 0
    width: icon.paintedWidth + iconTextMargin + message.paintedWidth + margin*2
    height: Math.max(icon.paintedHeight, message.paintedHeight) + margin*2

    Image {
        id: icon
        property string iconName: "dialog-information"
        source: "image://icon/"+iconName
        width: visible ? 32 : 0
        height: visible ? 32 : 0
        anchors {
            left: parent.left
            leftMargin: parent.margin
            verticalCenter: parent.verticalCenter
        }
        smooth: true
    }

    Text {
        id: message
        anchors {
            left: icon.right
            leftMargin: parent.iconTextMargin
            verticalCenter: parent.verticalCenter
        }
        color: "white"
        textFormat: Text.RichText
        smooth: true
        onLinkActivated: parent.linkActivated(link);
    }

    Behavior on opacity { NumberAnimation { duration: animate ? 300 : 0 } }

    Timer {
        id: hideTimer
        interval: 2000
        onTriggered: hide();
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: hide();
    }

    function show() {
        if (messageTimeout > 0)
            hideTimer.start();
        opacity = messageOpacity;
    }

    function hide() { opacity = 0; }
}
