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
        // Build the URL from width/height in a single binding: a separate
        // "size" property can still hold its old value while this binding is
        // re-evaluated, which produced requests like ".../0x0" that the image
        // provider renders at the element's natural size, only to discard them.
        source: prov == undefined || key == "" || Math.round(width) == 0 || Math.round(height) == 0
                ? ""
                : "image://" + provName + "/" + theme + "/" + key + "/" + Math.round(width) + "x" + Math.round(height)

        anchors.fill: parent
        smooth: parent.smooth
        cache: parent.cache
        asynchronous: true
        visible: false

        // Only set the visible image once the sprite has been fully rendered.
        // Null means that the source was cleared (no provider or key, or a
        // zero size); clear the frontend as well instead of keeping a stale
        // sprite.
        onStatusChanged: { // loads the sprite received from ImageProvider
            if (status == Image.Ready || status == Image.Null) parent.source = source;
        }
    }
}
