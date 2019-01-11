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
#include <QIcon>

#include <KColorScheme>

// margin on the sides of message box
static const int MARGIN = 15;
// offset of message from start of the scene
static const int SHOW_OFFSET = 5;
// space between pixmap and text
static const int SOME_SPACE = 10;
// width of the border in pixels
static const qreal BORDER_PEN_WIDTH = 1.0;

class TextItemWithOpacity : public QGraphicsTextItem
{
    Q_OBJECT

public:
    TextItemWithOpacity( QGraphicsItem* parent = nullptr )
        :QGraphicsTextItem(parent), m_opacity(1.0) {}
    void setOpacity(qreal opa) { m_opacity = opa; }
    void setTextColor(const KStatefulBrush &brush) { m_brush = brush; }
    void paint( QPainter* p, const QStyleOptionGraphicsItem *option, QWidget* widget ) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void mouseClicked();

private:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*) Q_DECL_OVERRIDE;

private:
    qreal m_opacity;
    KStatefulBrush m_brush;
};

void TextItemWithOpacity::paint( QPainter* p, const QStyleOptionGraphicsItem *option, QWidget* widget )
{
    // hope that it is ok to call this function here - i.e. I hope it won't be too expensive :)
    // we call it here (and not in setTextColor), because KstatefulBrush
    // absolutely needs QWidget parameter :)
    //NOTE from majewsky: For some weird reason, setDefaultTextColor does on some systems not check
    //whether the given color is equal to the one already set. Just calling setDefaultTextColor without
    //this check may result in an infinite loop of paintEvent -> setDefaultTextColor -> update -> paintEvent...
    const QColor textColor = m_brush.brush(widget).color();
    if (textColor != defaultTextColor())
    {
        setDefaultTextColor(textColor);
    }
    //render contents
    p->save();
    p->setOpacity(m_opacity);
    QGraphicsTextItem::paint(p,option,widget);
    p->restore();
}

void TextItemWithOpacity::mouseReleaseEvent(QGraphicsSceneMouseEvent* ev)
{
    // NOTE: this item is QGraphicsTextItem which "eats" mouse events
    // because of interaction with links. Because of that let's make a
    // special signal to indicate mouse click
    emit mouseClicked();
    QGraphicsTextItem::mouseReleaseEvent(ev);
}

class KGamePopupItemPrivate
{
private:
    KGamePopupItemPrivate(const KGamePopupItemPrivate&);
    const KGamePopupItemPrivate& operator=(const KGamePopupItemPrivate&);
public:
    KGamePopupItemPrivate()
        : m_position( KGamePopupItem::BottomLeft ), m_timeout(2000),
          m_opacity(1.0), m_animOpacity(-1), m_hoveredByMouse(false),
          m_hideOnClick(true), m_textChildItem(0),
          m_sharpness(KGamePopupItem::Square), m_linkHovered(false) {}
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
     * Opacity used while animating appearing in center
     */
    qreal m_animOpacity;
    /**
     * Pixmap to display at the left of the text
     */
    QPixmap m_iconPix;
    /**
     * Set to true when mouse hovers the message
     */
    bool m_hoveredByMouse;
    /**
     * Set to true if this popup item hides on mouse click.
     */
    bool m_hideOnClick;
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
    KStatefulBrush m_brush;
    /**
     * popup angles sharpness
     */
    KGamePopupItem::Sharpness m_sharpness;
    /**
     * painter path to draw a frame
     */
    QPainterPath m_path;
    /**
     * Indicates if some link is hovered in text item
     */
    bool m_linkHovered;
};

KGamePopupItem::KGamePopupItem(QGraphicsItem * parent)
    : QGraphicsItem(parent), d(new KGamePopupItemPrivate)
{
    hide();
    d->m_textChildItem = new TextItemWithOpacity(this);
    d->m_textChildItem->setTextInteractionFlags( Qt::LinksAccessibleByMouse );
    // above call said to enable ItemIsFocusable which we don't need.
    // So disabling it
    d->m_textChildItem->setFlag( QGraphicsItem::ItemIsFocusable, false );

    connect(d->m_textChildItem, &TextItemWithOpacity::linkActivated, this, &KGamePopupItem::linkActivated);
    connect(d->m_textChildItem, &TextItemWithOpacity::linkHovered, this, &KGamePopupItem::onLinkHovered);
    connect(d->m_textChildItem, &TextItemWithOpacity::mouseClicked, this, &KGamePopupItem::onTextItemClicked);

    setZValue(100); // is 100 high enough???
    d->m_textChildItem->setZValue(100);

    QIcon infoIcon = QIcon::fromTheme( QStringLiteral( "dialog-information" ));
    // default size is 32
    setMessageIcon( infoIcon.pixmap(32, 32) );

    d->m_timer.setSingleShot(true);

    setAcceptHoverEvents(true);
    // ignore scene transformations
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

    // setup default colors
    d->m_brush = KStatefulBrush( KColorScheme::Tooltip, KColorScheme::NormalBackground );
    d->m_textChildItem->setTextColor( KStatefulBrush(KColorScheme::Tooltip, KColorScheme::NormalText) );

    connect(&d->m_timeLine, &QTimeLine::frameChanged, this, &KGamePopupItem::animationFrame);
    connect(&d->m_timeLine, &QTimeLine::finished, this, &KGamePopupItem::hideMe);
    connect(&d->m_timer, &QTimer::timeout, this, &KGamePopupItem::playHideAnimation);
}

void KGamePopupItem::paint( QPainter* p, const QStyleOptionGraphicsItem *option, QWidget* widget )
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    p->save();

    QPen pen = p->pen();
    pen.setWidthF( BORDER_PEN_WIDTH );
    p->setPen(pen);

    if( d->m_animOpacity != -1) // playing Center animation
    {
        p->setOpacity(d->m_animOpacity);
    }
    else
    {
        p->setOpacity(d->m_opacity);
    }
    p->setBrush(d->m_brush.brush(widget));
    p->drawPath(d->m_path);
    p->drawPixmap( MARGIN, static_cast<int>(d->m_boundRect.height()/2) - d->m_iconPix.height()/2,
                   d->m_iconPix );
    p->restore();
}

void KGamePopupItem::showMessage( const QString& text, Position pos, ReplaceMode mode )
{
    if(d->m_timeLine.state() == QTimeLine::Running || d->m_timer.isActive())
    {
        if (mode == ReplacePrevious)
        {
            forceHide(InstantHide);
        }
        else
        {
            return;// we're already showing a message
        }
    }

    // NOTE: we blindly take first visible view we found. I.e. we don't support
    // multiple views. If no visible scene is found, we simply pick the first one.
    QGraphicsView *sceneView = 0;
    foreach (QGraphicsView *view, scene()->views()) {
        if (view->isVisible()) {
            sceneView = view;
            break;
        }
    }
    if (!sceneView)
    {
        sceneView = scene()->views().at(0);
    }

    QPolygonF poly = sceneView->mapToScene( sceneView->viewport()->contentsRect() );
    d->m_visibleSceneRect = poly.boundingRect();

    d->m_textChildItem->setHtml(text);

    d->m_position = pos;

    // do as QGS docs say: notify the scene about rect change
    prepareGeometryChange();

    // recalculate bounding rect
    qreal w = d->m_textChildItem->boundingRect().width()+MARGIN*2+d->m_iconPix.width()+SOME_SPACE;
    qreal h = d->m_textChildItem->boundingRect().height()+MARGIN*2;
    if( d->m_iconPix.height() > h )
    {
        h = d->m_iconPix.height() + MARGIN*2;
	}
    d->m_boundRect = QRectF(0, 0, w, h);

    // adjust to take into account the width of the pen
    // used to draw the border
    const qreal borderRadius = BORDER_PEN_WIDTH / 2.0;
    d->m_boundRect.adjust( -borderRadius ,
                           -borderRadius ,
                            borderRadius ,
                            borderRadius );

    QPainterPath roundRectPath;
    roundRectPath.moveTo(w, d->m_sharpness);
    roundRectPath.arcTo(w-(2*d->m_sharpness), 0.0,(2*d->m_sharpness), (d->m_sharpness), 0.0, 90.0);
    roundRectPath.lineTo(d->m_sharpness, 0.0);
    roundRectPath.arcTo(0.0, 0.0, (2*d->m_sharpness), (2*d->m_sharpness), 90.0, 90.0);
    roundRectPath.lineTo(0.0, h-(d->m_sharpness));
    roundRectPath.arcTo(0.0, h-(2*d->m_sharpness), 2*d->m_sharpness, 2*d->m_sharpness, 180.0, 90.0);
    roundRectPath.lineTo(w-(d->m_sharpness), h);
    roundRectPath.arcTo(w-(2*d->m_sharpness), h-(2*d->m_sharpness), (2*d->m_sharpness), (2*d->m_sharpness), 270.0, 90.0);
    roundRectPath.closeSubpath();

    d->m_path = roundRectPath;

    // adjust y-pos of text item so it appears centered
    d->m_textChildItem->setPos( d->m_textChildItem->x(),
                                d->m_boundRect.height()/2 - d->m_textChildItem->boundingRect().height()/2);

    // setup animation
    setupTimeline();

    // move to the start position
    animationFrame(d->m_timeLine.startFrame());
    show();
    d->m_timeLine.start();

    if(d->m_timeout != 0)
    {
        // 300 msec to animate showing message + d->m_timeout to stay visible => then hide
        d->m_timer.start( 300+d->m_timeout );
    }
}

void KGamePopupItem::setupTimeline()
{
    d->m_timeLine.setDirection( QTimeLine::Forward );
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
    else if( d->m_position == Center )
    {
        d->m_timeLine.setFrameRange(0, d->m_timeLine.duration());
        setPos( d->m_visibleSceneRect.left() +
                d->m_visibleSceneRect.width()/2 - d->m_boundRect.width()/2,
                d->m_visibleSceneRect.top() +
                d->m_visibleSceneRect.height()/2 - d->m_boundRect.height()/2);
    }

}

void KGamePopupItem::animationFrame(int frame)
{
    if( d->m_position == TopLeft || d->m_position == BottomLeft )
    {
        setPos( d->m_visibleSceneRect.left()+SHOW_OFFSET, frame );
    }
    else if( d->m_position == TopRight || d->m_position == BottomRight )
    {
        setPos( d->m_visibleSceneRect.right()-d->m_boundRect.width()-SHOW_OFFSET, frame );
    }
    else if( d->m_position == Center )
    {
        d->m_animOpacity = frame*d->m_opacity/d->m_timeLine.duration();
        d->m_textChildItem->setOpacity( d->m_animOpacity );
        update();
    }
}

void KGamePopupItem::playHideAnimation()
{
    if( d->m_hoveredByMouse )
    {
        return;
    }
    // let's hide
    d->m_timeLine.setDirection( QTimeLine::Backward );
    d->m_timeLine.start();
}

void KGamePopupItem::setMessageTimeout( int msec )
{
    d->m_timeout = msec;
}

void KGamePopupItem::setHideOnMouseClick( bool hide )
{
    d->m_hideOnClick = hide;
}

bool KGamePopupItem::hidesOnMouseClick() const
{
    return d->m_hideOnClick;
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
    d->m_animOpacity = -1;
    // and restore child's opacity too
    d->m_textChildItem->setOpacity(d->m_opacity);

    // if we just got moved out of visibility, let's do more - let's hide :)
    if( d->m_timeLine.direction() == QTimeLine::Backward )
    {
        hide();
        emit hidden();
    }
}

void KGamePopupItem::hoverEnterEvent( QGraphicsSceneHoverEvent* )
{
    d->m_hoveredByMouse = true;
}

void KGamePopupItem::hoverLeaveEvent( QGraphicsSceneHoverEvent* )
{
    d->m_hoveredByMouse = false;

    if( d->m_timeout != 0 && !d->m_timer.isActive() && d->m_timeLine.state() != QTimeLine::Running )
    {
        playHideAnimation(); // let's hide
	}
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

void KGamePopupItem::forceHide(HideType howToHide)
{
    if(!isVisible())
    {
        return;
    }

    if(howToHide == InstantHide)
    {
        d->m_timeLine.stop();
        d->m_timer.stop();
        hide();
        emit hidden();
    }
    else if(howToHide == AnimatedHide)
    {
        // forcefully unset it even if it is set
        // so we'll hide in any event
        d->m_hoveredByMouse = false;
        d->m_timer.stop();
        playHideAnimation();
    }
}

qreal KGamePopupItem::messageOpacity() const
{
    return d->m_opacity;
}

void KGamePopupItem::setBackgroundBrush( const QBrush& brush )
{
    d->m_brush = KStatefulBrush(brush);
}

void KGamePopupItem::setTextColor( const QColor& color )
{
    KStatefulBrush brush(color, d->m_brush.brush(QPalette::Active));
    d->m_textChildItem->setTextColor(brush);
}

void KGamePopupItem::onLinkHovered(const QString& link)
{
    if(link.isEmpty())
    {
        d->m_textChildItem->setCursor( Qt::ArrowCursor );
    }
    else
    {
        d->m_textChildItem->setCursor( Qt::PointingHandCursor );
    }

    d->m_linkHovered = !link.isEmpty();
    emit linkHovered(link);
}

void KGamePopupItem::setSharpness( Sharpness sharpness )
{
    d->m_sharpness = sharpness;
}

KGamePopupItem::Sharpness KGamePopupItem::sharpness() const
{
    return d->m_sharpness;
}

void KGamePopupItem::mousePressEvent( QGraphicsSceneMouseEvent* )
{
    // it is needed to reimplement this function to receive future
    // mouse release events
}

void KGamePopupItem::mouseReleaseEvent( QGraphicsSceneMouseEvent* )
{
    // NOTE: text child item is QGraphicsTextItem which "eats" mouse events
    // because of interaction with links. Because of that TextItemWithOpacity has
    // special signal to indicate mouse click which we catch in a onTextItemClicked()
    // slot
    if (d->m_hideOnClick)
    {
        forceHide();
    }
}

void KGamePopupItem::onTextItemClicked()
{
    // if link is hovered we don't hide as click should go to the link
    if (d->m_hideOnClick && !d->m_linkHovered)
    {
        forceHide();
    }
}

#include "moc_kgamepopupitem.cpp" // For automocing KGamePopupItem
#include "kgamepopupitem.moc" // For automocing TextItemWithOpacity
