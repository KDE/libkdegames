/*
    SPDX-FileCopyrightText: 2012 Viranch Mehta <viranch.mehta@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgimageprovider_p.h"

// own
#include "kgthemeprovider.h"
// Qt
#include <QGuiApplication>
#include <QPainter>

KgImageProvider::KgImageProvider(KgThemeProvider *prov)
    : QQuickImageProvider(Image)
    , m_provider(prov)
{
    reloadRenderer();
}

void KgImageProvider::reloadRenderer()
{
    m_renderer.load(m_provider->currentTheme()->graphicsPath());
    m_themeName = m_provider->currentThemeName();
}

QImage KgImageProvider::requestImage(const QString &source, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize); // this is always QSize(-1,-1) for some reason

    QImage image;

    const QStringList tokens = source.split(QStringLiteral("/"));
    if (tokens.size() > 2) {
        const QString theme = tokens[0];
        const QString spriteKey = tokens[1];
        const QStringList size = tokens[2].split(QLatin1Char('x'));
        uint width = qRound(size[0].toDouble());
        uint height = qRound(size[1].toDouble());

        if (theme != m_themeName) {
            reloadRenderer();
        }

        if (m_renderer.isValid()) {
            if (width == 0 || height == 0) {
                image = QImage(m_renderer.boundsOnElement(spriteKey).size().toSize() * qApp->devicePixelRatio(), QImage::Format_ARGB32_Premultiplied);
            } else {
                image = QImage(QSize(width, height) * qApp->devicePixelRatio(), QImage::Format_ARGB32_Premultiplied);
            }
            image.fill(Qt::transparent);
            QPainter *painter = new QPainter(&image);
            m_renderer.render(painter, spriteKey);

            // this is deliberately set after .render as Qt <= 5.4 has a bug in QSVGRenderer which makes it not fill the image properly
            image.setDevicePixelRatio(qApp->devicePixelRatio());
            delete painter;
        }
    }

    if (size)
        *size = image.size();

    return image;
}
