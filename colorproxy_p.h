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

#ifndef LIBKDEGAMES_COLORPROXY_P_H
#define LIBKDEGAMES_COLORPROXY_P_H

#include <QHash>
#include <QPaintDevice>
#include <QPaintEngine>

class QPaintEngineColorProxy;

#ifndef KDEGAMES_QCOLOR_QHASH
#	define KDEGAMES_QCOLOR_QHASH
	inline uint qHash(const QColor& color)
	{
		return color.rgba();
	}
#endif // KDEGAMES_QCOLOR_QHASH

/**
 * This QPaintDevice forwards all painting operations performed on it to the
 * contained QPaintDevice. The only modification is that certain colors are
 * replaced by others in all painting operations (except for drawImage and
 * drawPixmap).
 *
 * @note Using this class results in a non-negligible performance penalty
 * (~10-20% longer runtime), so use it only if the set of color replacements is
 * non-empty.
 */
class QPaintDeviceColorProxy : public QPaintDevice
{
	public:
		///@warning Replacement loops (e.g. color1 -> color2 -> color3 -> color1) lead to infinite loops.
		///@warning You should not interact with the @a proxiedDevice during the lifetime of this instance.
		QPaintDeviceColorProxy(QPaintDevice* proxiedDevice, const QHash<QColor, QColor>& replacements);
		~QPaintDeviceColorProxy();

		QPaintDevice* proxiedDevice() const;
		virtual QPaintEngine* paintEngine() const;

		QBrush map(const QBrush& brush) const;
		inline QColor map(const QColor& color) const;
		QPen map(const QPen& pen) const;
	protected:
		virtual int metric(PaintDeviceMetric metric) const;
	private:
		QPaintDevice* m_proxiedDevice;
		QPaintEngine* m_engine;
		QHash<QColor, QColor> m_replacements;
};

class QPaintEngineColorProxy : public QPaintEngine
{
	public:
		QPaintEngineColorProxy();
		~QPaintEngineColorProxy();

		virtual bool begin(QPaintDevice* device);
		virtual bool end();
		virtual void drawEllipse(const QRectF& rect);
		virtual void drawEllipse(const QRect& rect);
		virtual void drawImage(const QRectF& rectangle, const QImage& image, const QRectF& sr, Qt::ImageConversionFlags flags = Qt::AutoColor);
		virtual void drawLines(const QLineF* lines, int lineCount);
		virtual void drawLines(const QLine* lines, int lineCount);
		virtual void drawPath(const QPainterPath& path);
		virtual void drawPixmap(const QRectF& r, const QPixmap& pm, const QRectF& sr);
		virtual void drawPoints(const QPointF* points, int pointCount);
		virtual void drawPoints(const QPoint* points, int pointCount);
		virtual void drawPolygon(const QPointF* points, int pointCount, PolygonDrawMode mode);
		virtual void drawPolygon(const QPoint* points, int pointCount, PolygonDrawMode mode);
		virtual void drawRects(const QRectF* rects, int rectCount);
		virtual void drawRects(const QRect* rects, int rectCount);
		virtual void drawTextItem(const QPointF& p, const QTextItem& textItem);
		virtual void drawTiledPixmap(const QRectF& rect, const QPixmap& pixmap, const QPointF& p);
		virtual Type type() const;
		virtual void updateState(const QPaintEngineState& state);
	private:
		QPaintDeviceColorProxy* m_proxy;
		QPainter* m_painter;
};

#endif // LIBKDEGAMES_COLORPROXY_P_H
