/*
    SPDX-FileCopyrightText: 2007 Dmitry Suzdalev <dimsuz@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef K_GAME_POPUP_ITEM_H
#define K_GAME_POPUP_ITEM_H

// own
#include "kdegames_export.h"
// Qt
#include <QGraphicsItem>
#include <QObject>
// Std
#include <memory>

class KGamePopupItemPrivate;

/*!
 * \class KGamePopupItem
 * \inmodule LibKDEGames
 * \brief QGraphicsItem to show short popup messages which do not interrupt the gameplay.
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
    /*!
     * \enum KGamePopupItem::ReplaceMode
     * Possible values for message showing mode in respect to a previous message.
     * \value LeavePrevious
     *        Leaves the previous message in place.
     * \value ReplacePrevious
     *        Replaces the previous message.
     */
    enum ReplaceMode {
        LeavePrevious,
        ReplacePrevious
    };
    /*!
     * \enum KGamePopupItem::Sharpness
     * Possible values for the popup angles sharpness.
     * \value Square
     * \value Sharp
     * \value Soft
     * \value Softest
     */
    enum Sharpness {
        Square = 0,
        Sharp = 2,
        Soft = 5,
        Softest = 10
    };
    /*!
     * \enum KGamePopupItem::Position
     * The possible places in the scene where a message can be shown.
     * \value TopLeft
     *        Top left of the scene
     * \value TopRight
     *        Top right of the scene
     * \value BottomLeft
     *        Bottom left of the scene
     * \value BottomRight
     *        Bottom right of the scene
     * \value Center
     *        Center of the scene
     */
    enum Position {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Center
    };
    /*!
     * Constructs a message item with parent \a parent. It is hidden by default.
     */
    explicit KGamePopupItem(QGraphicsItem *parent = nullptr);
    /*!
     * Destructs a message item
     */
    ~KGamePopupItem() override;
    /*!
     * Shows the message \a text. item will appear at specified \a pos
     * of the scene using simple animation.
     * Item will be automatically hidden after timeout set in setMessageTimeOut() passes.
     * If item is hovered with mouse it won't hide until user moves
     * the mouse away.
     *
     * Note that if \a pos is \l Center, message animation will be of fade in/out type,
     * rather than slide in/out.
     *
     * \a mode how to handle an already shown message by this item:
     * either leave it and ignore the new one or replace it
     */
    void showMessage(const QString &text, Position pos, ReplaceMode mode = LeavePrevious);
    /*!
     * Sets the amount of time the item will stay visible on screen
     * before it goes away to \a msec milliseconds.
     * If item is hovered with mouse it will hide only after
     * user moves the mouse away.
     *
     * If \a msec is 0, the message will stay visible until it
     * gets explicitly hidden by forceHide().
     */
    void setMessageTimeout(int msec);
    /*!
     * Returns the message timeout that is currently set.
     * Defaults to 2000ms.
     */
    int messageTimeout() const;
    /*!
     * Sets the message \a opacity from 0 (fully transparent) to 1 (fully opaque).
     * For example 0.5 is half transparent.
     * It defaults to 1.0.
     */
    void setMessageOpacity(qreal opacity);
    /*!
     * Returns current message opacity.
     */
    qreal messageOpacity() const;
    /*!
     * Sets custom pixmap \a pix to show instead of default icon on the left.
     */
    void setMessageIcon(const QPixmap &pix);
    /*!
     * Sets whether to hide this popup item on mouse click to \a hide.
     * By default a mouse click will cause an item to hide
     */
    void setHideOnMouseClick(bool hide);
    /*!
     * Returns whether this popup item hides on mouse click.
     */
    bool hidesOnMouseClick() const;
    /*!
     * \enum KGamePopupItem::HideType
     * Used to specify how to hide in forceHide() - instantly or animatedly
     * \value InstantHide
     *        Hide instantly
     * \value AnimatedHide
     *        Hide animatedly
     */
    enum HideType {
        InstantHide,
        AnimatedHide
    };
    /*!
     * Requests the item to be hidden immediately, with HideType \a type.
     */
    void forceHide(HideType type = AnimatedHide);
    /*!
     * Sets the brush used to paint item background to \a brush.
     * By default system-default brush is used.
     * \sa KColorScheme
     */
    void setBackgroundBrush(const QBrush &brush);
    /*!
     * Sets the default color for unformatted text to \a color.
     * By default system-default color is used.
     * \sa KColorScheme
     */
    void setTextColor(const QColor &color);
    /*!
     * Returns the outer bounds of this item.
     * \sa QGraphicsItem::boundingRect
     */
    QRectF boundingRect() const override;
    /*!
     * Paints the contents of this item, in local coordinates, using painter \a p
     * with \a option, optionally onto \a widget.
     * \sa QGraphicsItem::paint
     */
    void paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    /*!
     * Sets the popup angles sharpness to \a sharpness.
     */
    void setSharpness(Sharpness sharpness);
    /*!
     * Returns current popup angles sharpness.
     */
    Sharpness sharpness() const;
Q_SIGNALS:
    /*!
     * Emitted when user clicks on a link \a link in this item.
     */
    void linkActivated(const QString &link);
    /*!
     * Emitted when user hovers a link \a link in this item.
     */
    void linkHovered(const QString &link);
    /*!
     * Emitted when the popup finishes hiding. This includes hiding caused by
     * both timeouts and mouse clicks.
     */
    void hidden();
private Q_SLOTS:
    void animationFrame(int);
    void hideMe();
    void playHideAnimation();
    void onLinkHovered(const QString &);
    void onTextItemClicked();

private:
    void setupTimeline();
    void mousePressEvent(QGraphicsSceneMouseEvent *) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *) override;

private:
    std::unique_ptr<KGamePopupItemPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(KGamePopupItem)
};

#endif
