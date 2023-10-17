/*
    SPDX-FileCopyrightText: 2010-2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgamegraphicsviewrenderer.h"

class KGameGraphicsViewRendererPrivate
{
public:
    QGraphicsView *m_defaultPrimaryView = nullptr;
};

KGameGraphicsViewRenderer::KGameGraphicsViewRenderer(KGameThemeProvider *provider, unsigned cacheSize)
    : KGameRenderer(provider, cacheSize)
    , d_ptr(new KGameGraphicsViewRendererPrivate())
{
}

KGameGraphicsViewRenderer::KGameGraphicsViewRenderer(KGameTheme *theme, unsigned cacheSize)
    : KGameRenderer(theme, cacheSize)
    , d_ptr(new KGameGraphicsViewRendererPrivate())
{
}

KGameGraphicsViewRenderer::~KGameGraphicsViewRenderer() = default;

QGraphicsView *KGameGraphicsViewRenderer::defaultPrimaryView() const
{
    Q_D(const KGameGraphicsViewRenderer);

    return d->m_defaultPrimaryView;
}

void KGameGraphicsViewRenderer::setDefaultPrimaryView(QGraphicsView *view)
{
    Q_D(KGameGraphicsViewRenderer);

    d->m_defaultPrimaryView = view;
}

#include "moc_kgamegraphicsviewrenderer.cpp"
