/*
    SPDX-FileCopyrightText: 2010-2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KGAMEGRAPHICSVIEWRENDERER_H
#define KGAMEGRAPHICSVIEWRENDERER_H

// own
#include "kdegames_export.h"
#include "kgamerenderer.h"

class QGraphicsView;

class KGameGraphicsViewRendererPrivate;

/*!
 * \class KGameGraphicsViewRenderer
 * \inmodule LibKDEGames
 * \brief The KGameGraphicsViewRenderer class is a view renderer for a KGameRenderedGraphicsObject.
 */
class KDEGAMES_EXPORT KGameGraphicsViewRenderer : public KGameRenderer
{
    Q_OBJECT

public:
    /*!
     * Constructs a KGameGraphicsViewRenderer with \a prov, \a cacheSize.
     */
    explicit KGameGraphicsViewRenderer(KGameThemeProvider *prov, unsigned cacheSize = 0);
    /*!
     * \overload
     * Constructs a KGameGraphicsViewRenderer with \a theme, \a cacheSize.
     * Allows using KGameRenderer without a theme provider (useful when there is only one theme).
     * \note Takes ownership of \a theme.
     */
    explicit KGameGraphicsViewRenderer(KGameTheme *theme, unsigned cacheSize = 0);
    /*!
     * Deletes this KGameRenderer instance, as well as all clients using it.
     */
    ~KGameGraphicsViewRenderer() override;

public:
    /*!
     * Returns the primary view which is used by newly created
     * KGameRenderedItem instances associated with this renderer
     * \sa KGameRenderedGraphicsObject::setPrimaryView
     */
    QGraphicsView *defaultPrimaryView() const;

    /*!
     * Set the primary view which will be used by newly created
     * KGameRenderedItem instances associated with this renderer to \a view.
     * Calls to this method will have no effect on existing instances.
     * \sa KGameRenderedGraphicsObject::setPrimaryView
     */
    void setDefaultPrimaryView(QGraphicsView *view);

private:
    std::unique_ptr<KGameGraphicsViewRendererPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(KGameGraphicsViewRenderer)
};

#endif // KGAMEGRAPHICSVIEWRENDERER_H
