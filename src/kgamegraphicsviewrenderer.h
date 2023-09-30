/*
    SPDX-FileCopyrightText: 2010-2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KGAMEGRAPHICSVIEWRENDERER_H
#define KGAMEGRAPHICSVIEWRENDERER_H

// own
#include "kdegameswidgets_export.h"
#include "kgamerenderer.h"

class QGraphicsView;

class KGameGraphicsViewRendererPrivate;

class KDEGAMESWIDGETS_EXPORT KGameGraphicsViewRenderer : public KGameRenderer
{
    Q_OBJECT

public:
    explicit KGameGraphicsViewRenderer(KGameThemeProvider *prov, unsigned cacheSize = 0);
    /// overload that allows to use KGameRenderer without a theme provider
    ///           (useful when there is only one theme)
    /// @note Takes ownership of @a theme.
    explicit KGameGraphicsViewRenderer(KGameTheme *theme, unsigned cacheSize = 0);
    /// Deletes this KGameRenderer instance, as well as all clients using it.
    ~KGameGraphicsViewRenderer() override;

public:
    /// @return the primary view which is used by newly created
    /// KGameRenderedItem instances associated with this renderer
    /// @see KGameRenderedItem::setPrimaryView
    QGraphicsView *defaultPrimaryView() const;

    /// Set the primary view which will be used by newly created
    /// KGameRenderedItem instances associated with this renderer.
    /// Calls to this method will have no effect on existing instances.
    /// @see KGameRenderedItem::setPrimaryView
    void setDefaultPrimaryView(QGraphicsView *view);

private:
    std::unique_ptr<KGameGraphicsViewRendererPrivate> const d;
};

#endif // KGAMEGRAPHICSVIEWRENDERER_H
