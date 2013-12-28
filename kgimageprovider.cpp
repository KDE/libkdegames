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

#include "kgimageprovider_p.h"

#include <QPainter>
#include <KgThemeProvider>

KgImageProvider::KgImageProvider(KgThemeProvider* prov) :
    QQuickImageProvider(Image),
    m_provider(prov)
{
    reloadRenderer();
}

void KgImageProvider::reloadRenderer()
{
    m_renderer.load(m_provider->currentTheme()->graphicsPath());
    m_themeName = m_provider->currentThemeName();
}

QImage KgImageProvider::requestImage(const QString& source, QSize *size, const QSize& requestedSize)
{
    Q_UNUSED(requestedSize); // this is always QSize(-1,-1) for some reason

    QImage image;

    const QStringList tokens = source.split("/");
    if (tokens.size() > 2) {
        const QString theme = tokens[0];
        const QString spriteKey = tokens[1];
        const QStringList size = tokens[2].split("x");
        uint width = qRound(size[0].toDouble());
        uint height = qRound(size[1].toDouble());

        if (theme != m_themeName) {
            reloadRenderer();
        }

        if (m_renderer.isValid()) {
            if (width == 0 || height == 0) {
                image = QImage(m_renderer.boundsOnElement(spriteKey).size().toSize(), QImage::Format_ARGB32_Premultiplied);
            } else {
                image = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
            }
            image.fill(Qt::transparent);
            QPainter* painter = new QPainter(&image);
            m_renderer.render(painter, spriteKey);
            delete painter;
        }
    }

    if (size) *size = image.size();

    return image;
}

