import QtQuick 1.1

Image {
    property variant provider
    property string spriteKey

    source: provider==undefined || spriteKey=="" || width*height==0 ? "" : "image://"+provider.name+"/"+provider.currentThemeName+"/"+spriteKey+"/"+Math.round(width)+"x"+Math.round(height)
    smooth: true
    asynchronous: true
}
