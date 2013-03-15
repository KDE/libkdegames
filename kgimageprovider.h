/***************************************************************************
 *   Copyright 2012 Viranch Mehta <viranch.mehta@gmail.com>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License          *
 *   version 2 as published by the Free Software Foundation                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KGIMAGEPROVIDER_H
#define KGIMAGEPROVIDER_H

#include <QDeclarativeImageProvider>

class QSvgRenderer;
class KgThemeProvider;

/**
 * @class KgImageProvider
 * @short A QDeclarativeImageProvider that renders requested sprites and
 * returns corresponding pixmap to the QML view.
 *
 * This class is a QDeclarativeImageProvider that takes a KgThemeProvider
 * in its constructor and uses it to get full path to SVGs. These theme
 * SVGs are read and requested sprite pixmap is extracted and given to
 * the QML image element that requests it.
 *
 * For porting KDE games to QML, there is a KgItem QML component provided
 * by KgCore QML plugin which is a small wrapper to request pixmaps from
 * this KgImageProvider. See KgItem's documentation for details.
 */
class KgImageProvider : public QDeclarativeImageProvider
{
public:
    ///Construcs a new KgImageProvider with the supplied KgThemeProvider
    ///@param provider The KgThemeProvider used to discover the game's
    ///themes.
    KgImageProvider(KgThemeProvider* provider);
    ///Destructor.
    ~KgImageProvider();

    ///Reimplemented method that is called when a sprite pixmap is requested
    QImage requestImage(const QString& source, QSize *size, const QSize &requestedSize);

private:
    void reloadRenderer();

    QString m_themeName;
    KgThemeProvider* m_provider;
    QSvgRenderer* m_renderer;

};

#endif //KGIMAGEPROVIDER_H
