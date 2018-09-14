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
#ifndef K_GAME_POPUP_ITEM_H
#define K_GAME_POPUP_ITEM_H

#include <libkdegames_export.h>

#include <QGraphicsItem>
#include <QObject>

class KGamePopupItemPrivate;

/**
 * \class KGamePopupItem kgamepopupitem.h <KGamePopupItem>
 *
 * QGraphicsItem capable of showing short popup messages
 * which do not interrupt the gameplay.
 * Message can stay on screen for specified amount of time
 * and automatically hide after (unless user hovers it with mouse).
 *
 * Example of use:
 * \code
 * KGamePopupItem *messageItem = new KGamePopupItem();
 * myGraphicsScene->addItem(messageItem);
 * ...
 * messageItem->setMessageTimeout( 3000 ); // 3 sec
 * messageItem->showMessage("Hello, I'm a game message! How do you do?", BottomLeft);
 * \endcode
 */
class KDEGAMES_EXPORT KGamePopupItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    /**
     * Possible values for message showing mode in respect to a previous
     * message
     */
    enum ReplaceMode { LeavePrevious, ReplacePrevious };
    /**
     * Possible values for the popup angles sharpness
     */
    enum Sharpness { Square=0, Sharp=2, Soft=5, Softest=10 };
    /**
     * The possible places in the scene where a message can be shown
     */
    enum Position { TopLeft, TopRight, BottomLeft, BottomRight, Center };
    /**
     * Constructs a message item. It is hidden by default.
     */
    KGamePopupItem(QGraphicsItem * parent = 0);
    /**
     * Destructs a message item
     */
    ~KGamePopupItem();
    /**
     * Shows the message: item will appear at specified place
     * of the scene using simple animation
     * Item will be automatically hidden after timeout set in setMessageTimeOut() passes
     * If item is hovered with mouse it won't hide until user moves
     * the mouse away
     *
     * Note that if pos == Center, message animation will be of fade in/out type,
     * rather than slide in/out
     *
     * @param text holds the message to show
     * @param pos position on the scene where the message will appear
     * @param mode how to handle an already shown message by this item:
       either leave it and ignore the new one or replace it
     */
    void showMessage( const QString& text, Position pos, ReplaceMode mode = LeavePrevious);
    /**
     * Sets the amount of time the item will stay visible on screen
     * before it goes away.
     * By default item is shown for 2000 msec
     * If item is hovered with mouse it will hide only after
     * user moves the mouse away
     *
     * @param msec amount of time in milliseconds.
     * if msec is 0, then message will stay visible until it
     * gets explicitly hidden by forceHide()
     */
    void setMessageTimeout( int msec );
    /**
     * @return timeout that is currently set
     */
    int messageTimeout() const;
    /**
     * Sets the message opacity from 0 (fully transparent) to 1 (fully opaque)
     * For example 0.5 is half transparent
     * It defaults to 1.0
     */
    void setMessageOpacity( qreal opacity );
    /**
     * @return current message opacity
     */
    qreal messageOpacity() const;
    /**
     * Sets custom pixmap to show instead of default icon on the left
     */
    void setMessageIcon( const QPixmap& pix );
    /**
     * Sets whether to hide this popup item on mouse click.
     * By default a mouse click will cause an item to hide
     */
    void setHideOnMouseClick( bool hide );
    /**
     * @return whether this popup item hides on mouse click.
     */
    bool hidesOnMouseClick() const;
    /**
     * Used to specify how to hide in forceHide() - instantly or animatedly
     */
    enum HideType { InstantHide, AnimatedHide };
    /**
     * Requests the item to be hidden immediately.
     */
    void forceHide(HideType type=AnimatedHide);
    /**
     * Sets brush used to paint item background
     * By default system-default brush is used
     * @see KColorScheme
     */
    void setBackgroundBrush( const QBrush& brush );
    /**
     * Sets default color for unformatted text
     * By default system-default color is used
     * @see KColorScheme
     */
    void setTextColor( const QColor& color );
    /**
     * @return the bounding rect of this item. Reimplemented from QGraphicsItem
     */
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    /**
     * Paints item. Reimplemented from QGraphicsItem
     */
    void paint( QPainter* p, const QStyleOptionGraphicsItem *option, QWidget* widget ) Q_DECL_OVERRIDE;
    /**
     * Sets the popup angles sharpness
     */
    void setSharpness( Sharpness sharpness );
    /**
     * @return current popup angles sharpness
     */
    Sharpness sharpness() const;
Q_SIGNALS:
    /**
     * Emitted when user clicks on a link in item
     */
    void linkActivated( const QString& link );
    /**
     * Emitted when user hovers a link in item
     */
    void linkHovered( const QString& link );
    /**
     * Emitted when the popup finishes hiding. This includes hiding caused by
     * both timeouts and mouse clicks.
     */
    void hidden();
private Q_SLOTS:
    void animationFrame(int);
    void hideMe();
    void playHideAnimation();
    void onLinkHovered(const QString&);
    void onTextItemClicked();
private:
    void setupTimeline();
    void mousePressEvent( QGraphicsSceneMouseEvent* ) Q_DECL_OVERRIDE;
    void mouseReleaseEvent( QGraphicsSceneMouseEvent* ) Q_DECL_OVERRIDE;
    void hoverEnterEvent( QGraphicsSceneHoverEvent* ) Q_DECL_OVERRIDE;
    void hoverLeaveEvent( QGraphicsSceneHoverEvent* ) Q_DECL_OVERRIDE;

    KGamePopupItemPrivate * const d;
};

#endif
