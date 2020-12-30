/*
    SPDX-FileCopyrightText: 2012 Viranch Mehta <viranch.mehta@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.3

Image {
    // frontend sprite: shown after a rendered sprite is received
    id: frontend

    property variant provider
    property string spriteKey

    smooth: true

    Image {
        // backend sprite: triggers requests for new sprite

        property alias prov: frontend.provider
        property string provName: prov==undefined ? "" : prov.name
        property string theme: prov==undefined ? "" : prov.currentThemeName
        property alias key: frontend.spriteKey
        property string size: Math.round(width)+"x"+Math.round(height)
        property string sourceUrl: "image://"+provName+"/"+theme+"/"+key+"/"+size
        source: prov==undefined || key=="" || width*height==0 ? "" : sourceUrl

        anchors.fill: parent
        smooth: parent.smooth
        cache: parent.cache
        asynchronous: true
        visible: false

        onStatusChanged: { // loads the sprite received from ImageProvider
            if (status == Image.Ready) parent.source = source;
        }
        onSourceChanged: { // loads sprite from cache as status does not change in this case
            if (status == Image.Ready) parent.source = source;
        }
    }
}
