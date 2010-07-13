/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>                     *
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

#ifndef KGAMERENDERER_H
#define KGAMERENDERER_H

#include <QtCore/QObject>
#include <QtGui/QPixmap>

#include <libkdegames_export.h>

class KGameRendererPrivate;
class KGameRendererClient;
class KGameRendererClientPrivate;
class KGameTheme;

/**
 * @class KGameRenderer
 * @since 4.6
 * @short Cache-enabled rendering of SVG themes.
 *
 * KGameRenderer is a light-weight rendering framework for the rendering of
 * SVG themes (as represented by KGameTheme) into pixmap caches.
 *
 * @section Terminology
 *
 * @li Themes, in the context of KGameRenderer, are the same as in the context
 *     of the KGameTheme class: a pair of a .desktop file and a .svg file. See
 *     the KGameTheme documentation for details.
 * @li A sprite is either a single pixmap ("non-animated sprites") or a sequence
 *     of pixmaps which are shown consecutively to produce an animation
 *     ("animated sprites"). Non-animated sprites correspond to a single element
 *     with the same key in the SVG theme file. The element keys for the pixmaps
 *     of an animated sprite are produced by appending the frameSuffix() to the
 *     sprite key.
 *
 * @section Access to the pixmaps
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
 * @section Rendering strategy
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
 * @section Support for legacy themes
 *
 * When porting applications to KGameRenderer, you probably have to support
 * the format of existing themes. KGameRenderer provides the frameBaseIndex()
 * and frameSuffix() properties for this purpose. It is recommended not to
 * change these properties in new applications.
 *
 */
class KDEGAMES_EXPORT KGameRenderer : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
	public:
		///Constructs a new KGameRenderer. The given @a defaultTheme will be
		///used as the initial theme, and will also act as a fallback when an
		///invalid theme is given.
		///@note If you load the theme name from a configuration file, give that
		///theme name as the first argument instead of calling setTheme() later.
		///setTheme() clears the cache, while this constructor tries to reuse it.
		///@param defaultTheme a theme name as used by KGameTheme::load
		///@param cacheSize the cache size in megabytes (if not given, a sane
		///default is used)
		explicit KGameRenderer(const QString& defaultTheme, unsigned cacheSize = 0);
		///Deletes this KGameRenderer instance, as well as all clients using it.
		virtual ~KGameRenderer();

		///@return the frame base index. @see setFrameBaseIndex()
		int frameBaseIndex() const;
		///Sets the frame base index, i.e. the lowest frame index. Usually,
		///frame numbering starts at zero, so the frame base index is zero.
		///
		///For example, if you set the frame base index to 42, and use the
		///default frame suffix, the 3 frames of an animated sprite "foo" are
		///provided by the SVG elements "foo_42", "foo_43" and "foo_44".
		///
		///It is recommended not to alter the frame base index unless you need
		///to support legacy themes.
		void setFrameBaseIndex(int frameBaseIndex);
		///@return the frame suffix. @see setFrameSuffix()
		QString frameSuffix() const;
		///Sets the frame suffix. This suffix will be added to a sprite key
		///to create the corresponding SVG element key, after any occurrence of
		///"%1" in the suffix has been replaced by the frame number.
		///@note Giving a suffix which does not include "%1" will reset to the
		///default suffix "_%1".
		///
		///For example, if the frame suffix is set to "_%1" (the default), the
		///SVG element key for the frame no. 23 of the sprite "foo" is "foo_23".
		///@note Frame numbering starts at zero unless you setFrameBaseIndex().
		void setFrameSuffix(const QString& suffix);
		///@return the theme currently in use (format as in KGameTheme::load())
		QString theme() const;

		///@return the KGameTheme instance used by this renderer
		const KGameTheme* gameTheme() const;

		///@return the bounding rectangle of the sprite with this @a key
		///This is equal to QSvgRenderer::boundsOnElement() of the corresponding
		///SVG element.
		QRectF boundsOnSprite(const QString& key, int frame = -1) const;
		///@return the count of frames available for the sprite with this @a key
		///If this sprite is not animated (i.e. there are no SVG elements for
		///any frames), this method returns 0. If the sprite does not exist at
		///all, -1 is returned.
		///
		///If the sprite is animated, the method counts frames starting at zero
		///(unless you change the frameBaseIndex()), and returns the number of
		///frames for which corresponding elements exist in the SVG file.
		///
		///For example, if the SVG contains the elements "foo_0", "foo_1" and
		///"foo_3", frameCount("foo") returns 2 for the default frame suffix.
		///(The element "foo_3" is ignored because "foo_2" is missing.)
		int frameCount(const QString& key) const;
		///@return if the sprite with the given @a key exists
		///This is the same as \code renderer.frameCount(key) >= 0 \endcode
		bool spriteExists(const QString& key) const;
		///@return a rendered pixmap
		///@param key the key of the sprite
		///@param size the size of the resulting pixmap
		///@param frame the number of the frame which you want
		///@note For non-animated frames, omit the @a frame parameter.
		QPixmap spritePixmap(const QString& key, const QSize& size, int frame = -1) const;
	public Q_SLOTS:
		///Load the given theme and update the pixmaps of all associated
		///KGameRendererClient instances.
		///@param theme a theme name as used by KGameTheme::load
		void setTheme(const QString& theme);
	Q_SIGNALS:
		///This signal is emitted when a new theme has been loaded. You usually
		///do not need to react on this signal if you use KGameRendererClient or
		///KGameRenderedItem, because these update their pixmaps automatically.
		void themeChanged(const QString& theme);
	private:
		friend class KGameRendererPrivate;
		friend class KGameRendererClient;
		friend class KGameRendererClientPrivate;
		KGameRendererPrivate* const d;
};

#endif // KGAMERENDERER_H
