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

Image {
    // the cached sprite to show while new one is rendered
    id: fallback

    property variant provider
    property string spriteKey

    smooth: true

    Image {
        // the original sprite to be rendered

        property alias prov: fallback.provider
        property string provName: prov.name
        property string theme: prov.currentThemeName
        property alias key: fallback.spriteKey
        property string size: Math.round(width)+"x"+Math.round(height)
        property string sourceUrl: "image://"+provName+"/"+theme+"/"+key+"/"+size
        source: prov==undefined || key=="" || width*height==0 ? "" : sourceUrl

        anchors.fill: parent
        smooth: true
        asynchronous: true

        onStatusChanged: {
            if (status == Image.Ready) parent.source = source;
        }
    }
}
