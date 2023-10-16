/*
    SPDX-FileCopyrightText: 2012 Viranch Mehta <viranch.mehta@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgameimageprovider_p.h"

// own
#include "kgamethemeprovider.h"
// Qt
#include <QGuiApplication>
#include <QPainter>

KGameImageProvider::KGameImageProvider(KGameThemeProvider *prov)
    : QQuickImageProvider(Image)
    , m_provider(prov)
{
    reloadRenderer();
}

void KGameImageProvider::reloadRenderer()
{
    m_renderer.load(m_provider->currentTheme()->graphicsPath());
    m_themeName = m_provider->currentThemeName();
}

QImage KGameImageProvider::requestImage(const QString &source, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize); // this is always QSize(-1,-1) for some reason

    QImage image;

    const QList<QStringView> tokens = QStringView(source).split(QLatin1Char('/'));
    if (tokens.size() > 2) {
        const QStringView &theme = tokens[0];
        const QString spriteKey = tokens[1].toString();
        const QList<QStringView> size = tokens[2].split(QLatin1Char('x'));
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
