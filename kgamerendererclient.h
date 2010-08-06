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

#ifndef KGAMERENDERERCLIENT_H
#define KGAMERENDERERCLIENT_H

#include <QtGui/QPixmap>

#include <libkdegames_export.h>

class KGameRendererClientPrivate;
class KGameRenderer;
class KGameRendererPrivate;

/**
 * @class KGameRendererClient kgamerendererclient.h <KGameRendererClient>
 * @since 4.6
 * @short An object that receives pixmaps from a KGameRenderer.
 *
 * This class abstracts a sprite rendered by KGameRenderer. Given a sprite key,
 * render size and possibly a frame index, it returns the QPixmap for this
 * sprite (frame) once it becomes available. See the KGameRenderer class
 * documentation for details.
 *
 * Subclasses have to reimplement the receivePixmap() method.
 */
class KDEGAMES_EXPORT KGameRendererClient
{
	public:
		///Creates a new client which receives pixmaps for the sprite with the 
		///given @a spriteKey as provided by the given @a renderer.
		KGameRendererClient(KGameRenderer* renderer, const QString& spriteKey);
		virtual ~KGameRendererClient();

		///@return the renderer used by this client
		KGameRenderer* renderer() const;
		///@return the frame count, or -1 for non-animated frames
		///@see KGameRenderer::frameCount()
		int frameCount() const;
		///@return the rendered pixmap (or an invalid pixmap if no pixmap has 
		///been rendered yet)
		QPixmap pixmap() const;

		///@return the key of the sprite currently rendered by this client
		QString spriteKey() const;
		///Defines the key of the sprite which is rendered by this client.
		void setSpriteKey(const QString& spriteKey);
		///@return the current frame number, or -1 for non-animated sprites
		int frame() const;
		///For animated sprites, render another frame. The given frame number is
		///normalized by taking the modulo of the frame count, so the following
		///code works fine:
		///@code
		///    class MyClient : public KGameRendererClient { ... }
		///    MyClient client;
		///    client.setFrame(client.frame() + 1); //cycle to next frame
		///    client.setFrame(KRandom::random());  //choose a random frame
		///@endcode
		void setFrame(int frame);
		///@return the size of the pixmap requested from KGameRenderer
		QSize renderSize() const;
		///Defines the size of the pixmap that will be requested from
		///KGameRenderer. For pixmaps rendered on the screen, you usually want 
		///to set this size such that the pixmap does not have to be scaled when 
		///it is rendered onto your primary view (for speed reasons).
		///
		///The default render size is very small (width = height = 3 pixels), so
		///that you notice when you forget to set this. ;-)
		void setRenderSize(const QSize& renderSize);
	protected:
		///This method is called when the KGameRenderer has provided a new
		///pixmap for this client (esp. after theme changes and after calls to 
		///setFrame(), setRenderSize() and setSpriteKey()).
		virtual void receivePixmap(const QPixmap& pixmap) = 0;
	private:
		friend class KGameRendererClientPrivate;
		friend class KGameRenderer;
		friend class KGameRendererPrivate;
		KGameRendererClientPrivate* const d;
};

#endif // KGAMERENDERERCLIENT_H
