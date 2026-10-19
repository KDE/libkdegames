/*
    SPDX-FileCopyrightText: 2010-2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KGAMERENDERER_H
#define KGAMERENDERER_H

// own
#include "kdegames_export.h"
// Qt
#include <QHash>
#include <QObject>
#include <QPixmap>
// Std
#include <memory>

class KGameRendererPrivate;
class KGameRendererClient;
class KGameRendererClientPrivate;
class KGameTheme;
class KGameThemeProvider;

#ifndef KDEGAMES_QCOLOR_QHASH
#define KDEGAMES_QCOLOR_QHASH
inline uint qHash(const QColor &color)
{
    return color.rgba();
}
#endif // KDEGAMES_QCOLOR_QHASH

/*!
 * \class KGameRenderer  kgamerenderer.h <KGameRenderer>
 * \inmodule LibKDEGames
 * \brief Cache-enabled rendering of SVG themes.
 *
 * KGameRenderer is a light-weight rendering framework for the rendering of
 * SVG themes (as represented by KGameTheme) into pixmap caches.
 *
 * \section1 Terminology
 *
 * \list
 * \li Themes in the context of KGameRenderer are KGameTheme instances. The theme
 *     selection by a KGameRenderer can be managed by a KGameThemeProvider.
 * \li A sprite is either a single pixmap ("non-animated sprites") or a sequence
 *     of pixmaps which are shown consecutively to produce an animation
 *     ("animated sprites"). Non-animated sprites correspond to a single element
 *     with the same key in the SVG theme file. The element keys for the pixmaps
 *     of an animated sprite are produced by appending the frameSuffix() to the
 *     sprite key.
 * \endlist
 *
 * \section1 Access to the pixmaps
 *
 * Sprite pixmaps can be retrieved from KGameRenderer in the main thread using
 * the synchronous KGameRenderer::spritePixmap() method. However, it is highly
 * recommended to use the asynchronous interface provided by the interface class
 * KGameRendererClient. A client corresponds to one pixmap and registers itself
 * with the corresponding KGameRenderer instance to get notified when a new
 * pixmap is available.
 *
 * For QGraphicsView-based applications, the KGameRenderedItem class provides a
 * QGraphicsPixmapItem which is a KGameRendererClient and displays the pixmap
 * for a given sprite.
 *
 * \section1 Rendering strategy
 *
 * For each theme, KGameRenderer keeps two caches around: an in-process cache of
 * QPixmaps, and a disk cache containing QImages (powered by KImageCache). You
 * therefore will not need to implement any caching for the pixmaps provided by
 * KGameRenderer.
 *
 * When requests from a KGameRendererClient cannot be served immediately because
 * the requested sprite is not in the caches, a rendering request is sent to a
 * worker thread.
 *
 * \section1 Support for legacy themes
 *
 * When porting applications to KGameRenderer, you probably have to support
 * the format of existing themes. KGameRenderer provides the frameBaseIndex()
 * and frameSuffix() properties for this purpose. It is recommended not to
 * change these properties in new applications.
 * \since 4.6
 */
class KDEGAMES_EXPORT KGameRenderer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(const KGameTheme *theme READ theme NOTIFY themeChanged)
    Q_PROPERTY(KGameThemeProvider *themeProvider READ themeProvider NOTIFY readOnlyProperty)

public:
    /*!
     * \enum KGameRenderer::Strategy
     * Describes the various strategies which KGameRenderer can use to speed
     * up rendering.
     * \sa setStrategyEnabled
     * \value UseDiskCache
     *        If set, pixmaps will be cached in a shared disk cache (using
     *        KSharedDataCache). This is especially useful for complex SVG
     *        themes because KGameRenderer will not load the SVG if all needed
     *        pixmaps are available from the disk cache.
     * \value UseRenderingThreads
     *        If set, pixmap requests from KGameRendererClients will be
     *        handled asynchronously if possible. This is especially useful
     *        when many clients are requesting complex pixmaps at one time.
     */
    enum Strategy {
        UseDiskCache = 1 << 0,
        UseRenderingThreads = 1 << 1
    };
    /**
     * Stores a combination of Strategy values.
     */
    Q_DECLARE_FLAGS(Strategies, Strategy)

    /*!
     * Constructs a new KGameRenderer that renders \a {prov}->currentTheme(),
     * where \a prov is the theme provider.
     * \a cacheSize is the cache size in mebibytes. If not given, a sane
     * default of 3MiB is used.
     * \warning This constructor may only be called from the main thread.
     * \sa KGameThemeProvider::currentTheme
     */
    explicit KGameRenderer(KGameThemeProvider *prov, unsigned cacheSize = 0);
    /*!
     * \overload
     * Constructs KGameRenderer without a theme provider
     * (useful when there is only one theme).
     * \note Takes ownership of \a theme.
     */
    explicit KGameRenderer(KGameTheme *theme, unsigned cacheSize = 0);
    /*!
     * Deletes this KGameRenderer instance, as well as all clients using it.
     */
    ~KGameRenderer() override;

    /*!
     * Returns the frame base index.
     */
    int frameBaseIndex() const;
    /*!
     * Sets the frame base index, that is, the lowest frame index, to \a frameBaseIndex.
     * Usually, frame numbering starts at zero, so the frame base index is zero.
     *
     * For example, if you set the frame base index to 42, and use the
     * default frame suffix, the 3 frames of an animated sprite "foo" are
     * provided by the SVG elements "foo_42", "foo_43" and "foo_44".
     *
     * It is recommended not to alter the frame base index unless you need
     * to support legacy themes.
     */
    void setFrameBaseIndex(int frameBaseIndex);
    /*!
     * Returns the frame suffix.
     */
    QString frameSuffix() const;
    /*!
     * Sets the frame suffix to \a suffix. This suffix will be added to a sprite key
     * to create the corresponding SVG element key, after any occurrence of
     * "%1" in the suffix has been replaced by the frame number.
     * \note Giving a suffix which does not include "%1" will reset to the
     * default suffix "_%1".
     *
     * For example, if the frame suffix is set to "_%1" (the default), the
     * SVG element key for the frame no. 23 of the sprite "foo" is "foo_23".
     * \note Frame numbering starts at zero unless you setFrameBaseIndex().
     */
    void setFrameSuffix(const QString &suffix);
    /*!
     * Returns the optimization strategies used by this renderer
     * \sa setStrategyEnabled()
     */
    Strategies strategies() const;
    /*!
     * Enables/disables an optimization \a strategy for this renderer according to \a enabled.
     * By default, both the UseDiskCache and the UseRenderingThreads strategies
     * are enabled. This is a sane default for 99% of all games. You might
     * only want to disable optimizations if the graphics are so simple that
     * the optimizations create an overhead in your special case.
     *
     * If you disable UseDiskCache, you should do so before setTheme(),
     * because changes to UseDiskCache cause a full theme reload.
     */
    void setStrategyEnabled(Strategy strategy, bool enabled = true);

    /*!
     * \property KGameRenderer::theme
     * \brief The KGameTheme instance used by this renderer.
     */
    const KGameTheme *theme() const;
    /*!
     * \property KGameRenderer::themeProvider
     * \brief The KGameThemeProvider instance used by this renderer.
     *
     * If the renderer was created with a single static theme, this is 0.
     */
    KGameThemeProvider *themeProvider() const;

    /*!
     * Returns the bounding rectangle of the sprite with this \a key for \a frame.
     * This is equal to QSvgRenderer::boundsOnElement() of the corresponding
     * SVG element.
     */
    QRectF boundsOnSprite(const QString &key, int frame = -1) const;
    /*!
     * Returns the count of frames available for the sprite with this \a key.
     * If this sprite is not animated (that is, there are no SVG elements for
     * any frames), this method returns 0. If the sprite does not exist at
     * all, -1 is returned.
     *
     * If the sprite is animated, the method counts frames starting at zero
     * (unless you change the frameBaseIndex()), and returns the number of
     * frames for which corresponding elements exist in the SVG file.
     *
     * For example, if the SVG contains the elements "foo_0", "foo_1" and
     * "foo_3", frameCount("foo") returns 2 for the default frame suffix.
     * (The element "foo_3" is ignored because "foo_2" is missing.)
     */
    int frameCount(const QString &key) const;
    /*!
     * Returns whether the sprite with the given \a key exists.
     * This is the same as \c {renderer.frameCount(key) >= 0}
     */
    bool spriteExists(const QString &key) const;
    // The parentheses around QHash<QColor, QColor>() avoid compile
    // errors on platforms with older gcc versions, e.g. OS X 10.6.
    /*!
     * Returns a rendered pixmap from the \a key of the sprite
     * at the size \a size using frame number \a frame.
     * \a customColors is the custom color replacements for this client.
     *        That is, for each entry in this has, the key color will be
     *        replaced by its value if it is encountered in the sprite.
     * \note  For non-animated frames, set \a frame to -1 or omit it.
     * \note  Custom colors increase the rendering time considerably, so use
     *        this feature only if you really need its flexibility.
     */
    QPixmap spritePixmap(const QString &key, QSize size, int frame = -1, const QHash<QColor, QColor> &customColors = (QHash<QColor, QColor>())) const;

Q_SIGNALS:
    /*!
     * Emitted when the theme changes to \a theme.
     */
    void themeChanged(const KGameTheme *theme);
    /// This signal is never emitted. It is provided because QML likes to
    /// complain about properties without NOTIFY signals, even readonly ones.
    void readOnlyProperty();

private:
    friend class KGameRendererPrivate;
    friend class KGameRendererClient;
    friend class KGameRendererClientPrivate;
    std::unique_ptr<KGameRendererPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(KGameRenderer)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KGameRenderer::Strategies)

#endif // KGAMERENDERER_H
