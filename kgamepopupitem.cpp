/*******************************************************************
    Copyright 2007 Dmitry Suzdalev <dimsuz@gmail.com>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ********************************************************************/
#include "kgamepopupitem.h"
#include <QPainter>
#include <QTimeLine>
#include <QTimer>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsTextItem>

#include <KIcon>
#include <KDebug>
#include <kcolorscheme.h>

// margin on the sides of message box
static const int MARGIN = 15;
// offset of message from start of the scene
static const int SHOW_OFFSET = 5;
// space between pixmap and text
static const int SOME_SPACE = 10;

class TextItemWithOpacity : public QGraphicsTextItem
{
public:
    TextItemWithOpacity( QGraphicsItem* parent = 0 )
        :QGraphicsTextItem(parent), m_opacity(1.0) {}
    void setOpacity(qreal opa) { m_opacity = opa; }
    virtual void paint( QPainter* p, const QStyleOptionGraphicsItem *option, QWidget* widget )
        {
            p->setOpacity(m_opacity);
            QGraphicsTextItem::paint(p,option,widget);
            p->setOpacity(1.0);
        }
private:
    qreal m_opacity;
};

class KGamePopupItemPrivate
{
private:
    KGamePopupItemPrivate(const KGamePopupItemPrivate&);
    const KGamePopupItemPrivate& operator=(const KGamePopupItemPrivate&);
public:
    KGamePopupItemPrivate()
        : m_position( KGamePopupItem::BottomLeft ), m_timeout(2000),
          m_opacity(1.0), m_hoveredByMouse(false), m_textChildItem(0) {}
    /**
     * Timeline for animations
     */
    QTimeLine m_timeLine;
    /**
     * Timer used to start hiding
     */
    QTimer m_timer;
    /**
     * Holds bounding rect of an item
     */
    QRectF m_boundRect;
    /**
     * Position where item will appear
     */
    KGamePopupItem::Position m_position;
    /**
     * Timeout to stay visible on screen
     */
    int m_timeout;
    /**
     * Item opacity
     */
    qreal m_opacity;
    /**
     * Pixmap to display at the left of the text
     */
    QPixmap m_iconPix;
    /**
     * Set to true when mouse hovers the message
     */
    bool m_hoveredByMouse;
    /**
     * Child of KGamePopupItem used to display text
     */
    TextItemWithOpacity* m_textChildItem;
    /**
     * Part of the scene that is actually visible in QGraphicsView
     * This is needed for item to work correctly when scene is larger than
     * the View
     */
    QRectF m_visibleSceneRect;
    /**
     * Background brush color
     */
    QBrush m_brush;
};

KGamePopupItem::KGamePopupItem()
    : d(new KGamePopupItemPrivate)
{
    hide();
    d->m_textChildItem = new TextItemWithOpacity(this);
    d->m_textChildItem->setTextInteractionFlags( Qt::LinksAccessibleByMouse );
    // above call said to enable ItemIsFocusable which we don't need.
    // So disabling it
    d->m_textChildItem->setFlag( QGraphicsItem::ItemIsFocusable, false );

    connect( d->m_textChildItem, SIGNAL(linkActivated(const QString&)),
                                 SIGNAL(linkActivated(const QString&)));
    connect( d->m_textChildItem, SIGNAL(linkHovered(const QString&)),
                                 SIGNAL(linkHovered(const QString&)));

    setZValue(100); // is 100 high enough???
    d->m_textChildItem->setZValue(100);

    KIcon infoIcon("dialog-information");
    // default size is 32
    setMessageIcon( infoIcon.pixmap(32, 32) );

    d->m_timer.setSingleShot(true);

    setAcceptsHoverEvents(true);

    // setup default colors
    KColorScheme kcs( KColorScheme::Tooltip );
    d->m_brush = kcs.background();
    d->m_textChildItem->setDefaultTextColor( kcs.foreground(KColorScheme::NormalText).color() );

    connect( &d->m_timeLine, SIGNAL(frameChanged(int)), SLOT(animationFrame(int)) );
    connect( &d->m_timeLine, SIGNAL(finished()), SLOT(hideMe()));
    connect( &d->m_timer, SIGNAL(timeout()), SLOT(playHideAnimation()) );
}

void KGamePopupItem::paint( QPainter* p, const QStyleOptionGraphicsItem *option, QWidget* widget )
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    p->setBrush( d->m_brush );
    p->setOpacity(d->m_opacity);
    p->drawRect( d->m_boundRect );
    p->drawPixmap( MARGIN, static_cast<int>(d->m_boundRect.height()/2) - d->m_iconPix.height()/2,
                   d->m_iconPix );

    p->setOpacity(1.0);
}

void KGamePopupItem::showMessage( const QString& text, Position pos )
{
    if(d->m_timeLine.state() == QTimeLine::Running || d->m_timer.isActive())
        return;// we're already showing a message

    // NOTE: we blindly take first view we found. I.e. we don't support
    // multiple views
    QGraphicsView *sceneView = scene()->views().at(0);
    QPolygonF poly = sceneView->mapToScene( sceneView->contentsRect() );
    d->m_visibleSceneRect = poly.boundingRect();

    d->m_textChildItem->setHtml(text);

    d->m_position = pos;
    d->m_timeLine.setDirection( QTimeLine::Forward );

    // do as QGS docs say: notify the scene about rect change
    prepareGeometryChange();

    // recalculate bounding rect
    qreal w = d->m_textChildItem->boundingRect().width()+MARGIN*2+d->m_iconPix.width()+SOME_SPACE;
    qreal h = d->m_textChildItem->boundingRect().height()+MARGIN*2;
    if( d->m_iconPix.height() > h )
        h = d->m_iconPix.height() + MARGIN*2;
    d->m_boundRect = QRectF(0, 0, w, h);

    // adjust y-pos of text item so it appears centered
    d->m_textChildItem->setPos( d->m_textChildItem->x(),
                                d->m_boundRect.height()/2 - d->m_textChildItem->boundingRect().height()/2);

    // setup animation
    d->m_timeLine.setDuration(300);
    if( d->m_position == TopLeft || d->m_position == TopRight )
    {
        int start = static_cast<int>(d->m_visibleSceneRect.top() - d->m_boundRect.height() - SHOW_OFFSET);
        int end = static_cast<int>(d->m_visibleSceneRect.top() + SHOW_OFFSET);
        d->m_timeLine.setFrameRange( start, end );
    }
    else if( d->m_position == BottomLeft || d->m_position == BottomRight )
    {
        int start = static_cast<int>(d->m_visibleSceneRect.bottom()+SHOW_OFFSET);
        int end = static_cast<int>(d->m_visibleSceneRect.bottom() - d->m_boundRect.height() - SHOW_OFFSET);
        d->m_timeLine.setFrameRange( start, end );
    }

    // move to the start position
    animationFrame(d->m_timeLine.startFrame());
    show();
    d->m_timeLine.start();
    // 300 msec to animate showing message + d->m_timeout to stay visible => then hide
    d->m_timer.start( 300+d->m_timeout );
}

void KGamePopupItem::animationFrame(int frame)
{
    if( d->m_position == TopLeft || d->m_position == BottomLeft )
        setPos( d->m_visibleSceneRect.left()+SHOW_OFFSET, frame );
    else if( d->m_position == TopRight || d->m_position == BottomRight )
        setPos( d->m_visibleSceneRect.right()-d->m_boundRect.width()-SHOW_OFFSET, frame );
}

void KGamePopupItem::playHideAnimation()
{
    if( d->m_hoveredByMouse )
        return;

    // let's hide
    d->m_timeLine.setDirection( QTimeLine::Backward );
    d->m_timeLine.start();
}

void KGamePopupItem::setMessageTimeout( int msec )
{
    d->m_timeout = msec;
}

void KGamePopupItem::setMessageOpacity( qreal opacity )
{
    d->m_opacity = opacity;
    d->m_textChildItem->setOpacity(opacity);
}

QRectF KGamePopupItem::boundingRect() const
{
    return d->m_boundRect;
}

KGamePopupItem::~KGamePopupItem()
{
    delete d;
}

void KGamePopupItem::hideMe()
{
    // if we just got moved out of visibility, let's do more - let's hide :)
    if( d->m_timeLine.direction() == QTimeLine::Backward )
        hide();
}

void KGamePopupItem::hoverEnterEvent( QGraphicsSceneHoverEvent* )
{
    d->m_hoveredByMouse = true;
}

void KGamePopupItem::hoverLeaveEvent( QGraphicsSceneHoverEvent* )
{
    d->m_hoveredByMouse = false;

    if( !d->m_timer.isActive() && d->m_timeLine.state() != QTimeLine::Running )
        playHideAnimation(); // let's hide
}

void KGamePopupItem::setMessageIcon( const QPixmap& pix )
{
    d->m_iconPix = pix;
    d->m_textChildItem->setPos( MARGIN+pix.width()+SOME_SPACE, MARGIN );
    // bounding rect is updated in showMessage()
}

int KGamePopupItem::messageTimeout() const
{
    return d->m_timeout;
}

void KGamePopupItem::forceHide()
{
    d->m_timeLine.stop();
    d->m_timer.stop();
    hide();
}

qreal KGamePopupItem::messageOpacity() const
{
    return d->m_opacity;
}

void KGamePopupItem::setBackgroundBrush( const QBrush& brush )
{
    d->m_brush = brush;
}

void KGamePopupItem::setTextColor( const QColor& color )
{
    d->m_textChildItem->setDefaultTextColor(color);
}

#include "kgamepopupitem.moc"
