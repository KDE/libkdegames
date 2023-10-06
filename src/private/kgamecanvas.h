/*  Originally created for KBoard
    SPDX-FileCopyrightText: 2006 Maurizio Monge <maurizio.monge@gmail.com>

    SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef __KGAMECANVAS_H__
#define __KGAMECANVAS_H__

/*
 *  Author: Maurizio Monge <maurizio.monge@gmail.com>
 */

// own
#include "kdegamesprivate_export.h"
#include "kgamerendererclient.h"
// Qt
#include <QList>
#include <QPainter>
#include <QPicture>
#include <QPixmap>
#include <QPoint>
#include <QRect>
#include <QRegion>
#include <QWidget>

class KGameCanvasItem;

/**
    \class KGameCanvasAbstract kgamecanvas.h <KGameCanvas>
    \brief Container class.

    A KGameCanvasAbstract is a set of canvas items.

    \deprecated For new applications, use Qt's Graphics View framework or Qt Quick.
*/
class KDEGAMESPRIVATE_EXPORT KGameCanvasAbstract
{
protected:
    friend class KGameCanvasItem;

    QList<KGameCanvasItem *> m_items;
    QList<KGameCanvasItem *> m_animated_items;

public:
    /** The constructor */
    KGameCanvasAbstract();

    virtual ~KGameCanvasAbstract();

    /** Returns a const pointer to the list holding all the items in the canvas */
    const QList<KGameCanvasItem *> *items() const
    {
        return &m_items;
    }

    /** Helper function to retrieve the topmost item at the given position */
    KGameCanvasItem *itemAt(QPoint pos) const;

    /** Overload, same as above */
    KGameCanvasItem *itemAt(int x, int y) const
    {
        return itemAt(QPoint(x, y));
    }

    /** Helper function to retrieve all the items at the given position,
        starting from the topmost one. */
    QList<KGameCanvasItem *> itemsAt(QPoint pos) const;

    /** Overload, same as above */
    QList<KGameCanvasItem *> itemsAt(int x, int y) const
    {
        return itemsAt(QPoint(x, y));
    }

    /** Virtual function to let know the canvas that it has animated items in it */
    virtual void ensureAnimating() = 0;

    /** Virtual function to ensure an update is pending, called from children */
    virtual void ensurePendingUpdate() = 0;

    /** Virtual function to update a rect */
    virtual void invalidate(QRect r, bool translate = true) = 0;

    /** Virtual function to update a region */
    virtual void invalidate(const QRegion &r, bool translate = true) = 0;

    /** Returns the toplevel non-group KGameCanvasWidget object */
    virtual class KGameCanvasWidget *topLevelCanvas() = 0;

    /** @return Position of the abstract canvas relative to the toplevel canvas. */
    virtual QPoint canvasPosition() const = 0;
};

/**
    \class KGameCanvasItem kgamecanvas.h <KGameCanvas>
    \brief An abstract item.

    A KGameCanvasItem is an abstract class to represent a generic item that can be
    put in a canvas.

    \deprecated For new applications, use Qt's Graphics View framework or Qt Quick.
*/
class KDEGAMESPRIVATE_EXPORT KGameCanvasItem
{
private:
    friend class KGameCanvasAbstract;
    friend class KGameCanvasWidget;
    friend class KGameCanvasGroup;
    friend class KGameCanvasAdapter;

    bool m_visible;
    bool m_animated;
    int m_opacity;
    QPoint m_pos;
    KGameCanvasAbstract *m_canvas;

    bool m_changed;
    QRect m_last_rect;

    static QPixmap *transparence_pixmap_cache;
    static QPixmap *getTransparenceCache(QSize s);
    virtual void paintInternal(QPainter *p, QRect prect, const QRegion &preg, QPoint delta, double cumulative_opacity);

    void updateAfterRestack(int from, int to);

    /* function to update pending changes, called from parent */
    virtual void updateChanges();

public:
    /** Constructor, it allows you to specify the reference canvas or to create
        an orphan item that will be put into a canvas in a second moment.
        The item is always hidden after being created.
        Note that the restacking functions are quite intelligent and will only
        repaint if there is an actual need of doing it. So if you call raise on
        an item that is already (locally) on the top, no redraw will take place */
    explicit KGameCanvasItem(KGameCanvasAbstract *canvas = nullptr);

    virtual ~KGameCanvasItem();

    /** schedule an update if the item */
    virtual void changed();

    /** Returns true if the item is visible */
    bool visible() const
    {
        return m_visible;
    }

    /** Set the item as visible or hidden */
    void setVisible(bool v);

    /** Returns true if the item is animated */
    bool animated() const
    {
        return m_animated;
    }

    /** Set the item as animated or not */
    void setAnimated(bool a);

    /** Returns the opacity of the item */
    int opacity() const
    {
        return m_opacity;
    }

    /** Set the item's opacity value (int the 0-255 range) */
    void setOpacity(int o);

    /** Hides the item */
    void hide()
    {
        setVisible(false);
    }

    /** Shows the item */
    void show()
    {
        setVisible(true);
    }

    /** Restacks the item on the top of the canvas */
    void raise();

    /** Restacks the item on the bottom of the canvas */
    void lower();

    /** Restacks the item immediately over ref */
    void stackOver(KGameCanvasItem *ref);

    /** Restacks the item immediately under ref */
    void stackUnder(KGameCanvasItem *ref);

    /** Returns the canvas that is actually "owning" the item */
    KGameCanvasAbstract *canvas() const
    {
        return m_canvas;
    }

    /** Returns the toplevel canvas widget, or NULL */
    KGameCanvasWidget *topLevelCanvas() const
    {
        return m_canvas ? m_canvas->topLevelCanvas() : nullptr;
    }

    /** Lets you specify the owning canvas. Call this function with canvas
        set to NULL to remove the item from the current canvas. */
    void putInCanvas(KGameCanvasAbstract *canvas);

    /** Returns the position of the item */
    QPoint pos() const
    {
        return m_pos;
    }

    /** @return Position of the item relative to the top level canvas. */
    QPoint absolutePosition() const;

    /** Sets a new position. Note that an update will be posted to the parent
        canvas, and if you move an item twice in very little time, a region
        bigger than needed will be updated, causing a possible inefficiency */
    void moveTo(QPoint newpos);

    /** Overload, same as above */
    void moveTo(int x, int y)
    {
        moveTo(QPoint(x, y));
    }

    /** Override this function to draw the item with the painter */
    virtual void paint(QPainter *p) = 0;

    /** Override this function to return the rect the item will be drawn into */
    virtual QRect rect() const = 0;

    /** Override this function to specify if the painting operations will paint over
        each other. If not, the item will be drawn more quickly when opacity is != 255,
        because it does not have to be painted onto a pixmap first. If you don't care
        about the item's opacity, don't care about this function as well. */
    virtual bool layered() const;

    /** Override this function to handle animations, the default function does nothing.
        The argument is the number of milliseconds from the creation of the canvas, so
        that you use it to handle the animation. */
    virtual void advance(int msecs);
};

/**
    \class KGameCanvasDummy kgamecanvas.h <KGameCanvas>
    \brief A dummy (empty) item.

    A KGameCanvasDummy is an empty, invisible item.
    You can use it as reference for stacking items in the canvas using the
    stackOver and stackUnder functions.

    \deprecated For new applications, use Qt's Graphics View framework or Qt Quick.
*/
class KDEGAMESPRIVATE_EXPORT KGameCanvasDummy : public KGameCanvasItem
{
public:
    /** Constructor */
    explicit KGameCanvasDummy(KGameCanvasAbstract *canvas = nullptr);

    ~KGameCanvasDummy() override;

    /** This function does nothing (of course) */
    void paint(QPainter *p) override;

    /** This returns an empty rectangle */
    QRect rect() const override;
};

/**
    \class KGameCanvasGroup kgamecanvas.h <KGameCanvas>
    \brief An item containing other items.

    A KGameCanvasGroup is an KGameCanvasItem, but it is also a KGameCanvasAbstract,
    so you can add children items to it. Just an inner canvas, if you prefer.

    \deprecated For new applications, use Qt's Graphics View framework or Qt Quick.
*/
class KDEGAMESPRIVATE_EXPORT KGameCanvasGroup : public KGameCanvasItem, public KGameCanvasAbstract
{
private:
    mutable bool m_child_rect_changed;
    mutable QRect m_last_child_rect;

    void paintInternal(QPainter *p, QRect prect, const QRegion &preg, QPoint delta, double cumulative_opacity) override;

    void ensureAnimating() override;
    void ensurePendingUpdate() override;
    void invalidate(QRect r, bool translate = true) override;
    void invalidate(const QRegion &r, bool translate = true) override;
    void updateChanges() override;
    void changed() override;

public:
    /** Constructor */
    explicit KGameCanvasGroup(KGameCanvasAbstract *canvas = nullptr);

    ~KGameCanvasGroup() override;

    /** This paints all the children */
    void paint(QPainter *p) override;

    /** This returns the bouding rect of all children */
    QRect rect() const override;

    /** Animations step, updates the animation for the children */
    void advance(int msecs) override;

    /** returns the toplevel canvas (or null if it is in an orphan tree) */
    KGameCanvasWidget *topLevelCanvas() override;

    QPoint canvasPosition() const override;
};

/**
    \class KGameCanvasPicture kgamecanvas.h <KGameCanvas>
    \brief A picture, ie a collection of paint operations.

    A KGameCanvasPicture is a picture that can be replayed on the canvas.

    \deprecated For new applications, use Qt's Graphics View framework or Qt Quick.
*/
class KDEGAMESPRIVATE_EXPORT KGameCanvasPicture : public KGameCanvasItem
{
private:
    QPicture m_picture;

public:
    /** Constructor, specifying the picture to use */
    explicit KGameCanvasPicture(const QPicture &picture, KGameCanvasAbstract *canvas = nullptr);

    /** Constructor, creating with an empty picture */
    explicit KGameCanvasPicture(KGameCanvasAbstract *canvas = nullptr);

    ~KGameCanvasPicture() override;

    /** Returns the picture */
    QPicture picture() const
    {
        return m_picture;
    }

    /** Sets the picture of the sprite */
    void setPicture(const QPicture &picture);

    void paint(QPainter *p) override;
    QRect rect() const override;
};

/**
    \class KGameCanvasPixmap kgamecanvas.h <KGameCanvas>
    \brief A pixmap (sprite).

    A KGameCanvasPixmap is a pixmap that can be put in the canvas.

    \deprecated For new applications, use Qt's Graphics View framework or Qt Quick.
*/
class KDEGAMESPRIVATE_EXPORT KGameCanvasPixmap : public KGameCanvasItem
{
private:
    QPixmap m_pixmap;

public:
    /** Constructor, specifying the pixmap to use */
    explicit KGameCanvasPixmap(const QPixmap &pixmap, KGameCanvasAbstract *canvas = nullptr);

    /** Constructor, creating with an empty pixmap */
    explicit KGameCanvasPixmap(KGameCanvasAbstract *canvas = nullptr);

    ~KGameCanvasPixmap() override;

    /** Returns the pixmap */
    QPixmap pixmap() const
    {
        return m_pixmap;
    }

    /** Sets the pixmap of the sprite */
    void setPixmap(const QPixmap &pixmap);

    void paint(QPainter *p) override;
    QRect rect() const override;
    bool layered() const override
    {
        return false;
    }
};

/**
    \class KGameCanvasRenderedPixmap kgamecanvas.h <KGameCanvas>
    \brief A sprite pixmap from KGameRenderer.

    This canvas item behaves like KGameCanvasPixmap, but the pixmaps are served
    from a KGameRenderer. This class exists solely for the purpose of porting
    existing applications to KGameRenderer.

    \deprecated For new applications, use Qt's Graphics View framework or Qt Quick.
 */
class KDEGAMESPRIVATE_EXPORT KGameCanvasRenderedPixmap : public KGameCanvasPixmap, public KGameRendererClient
{
public:
    KGameCanvasRenderedPixmap(KGameRenderer *renderer, const QString &spriteKey, KGameCanvasAbstract *canvas = nullptr);

protected:
    void receivePixmap(const QPixmap &pixmap) override;
};

/**
    \class KGameCanvasTiledPixmap kgamecanvas.h <KGameCanvas>
    \brief A tiled pixmap (brush).

    A KGameCanvasTiledPixmap is a pixmap that can be put in the canvas.

    \deprecated For new applications, use Qt's Graphics View framework or Qt Quick.
*/
class KDEGAMESPRIVATE_EXPORT KGameCanvasTiledPixmap : public KGameCanvasItem
{
private:
    QPixmap m_pixmap;
    QSize m_size;
    QPoint m_origin;
    bool m_move_orig;

public:
    /** Constructor, specifying the pixmap and the parameters to use */
    KGameCanvasTiledPixmap(const QPixmap &pixmap, QSize size, QPoint origin, bool move_orig, KGameCanvasAbstract *canvas = nullptr);

    /** Constructor, creating with an empty pixmap */
    explicit KGameCanvasTiledPixmap(KGameCanvasAbstract *canvas = nullptr);

    ~KGameCanvasTiledPixmap() override;

    /** Returns the pixmap */
    QPixmap pixmap() const
    {
        return m_pixmap;
    }

    /** Sets the pixmap of the tile */
    void setPixmap(const QPixmap &pixmap);

    /** Sets the size */
    void setSize(QSize size);

    /** The origin */
    QPoint origin() const
    {
        return m_move_orig ? m_origin + pos() : m_origin;
    }

    /** Sets the origin of the tiles */
    void setOrigin(QPoint size);

    /** If the origin is moved */
    bool moveOrigin()
    {
        return m_move_orig;
    }

    /** Sets if the origin of the brush will be moved with the pixmap */
    void setMoveOrigin(bool move_orig);

    void paint(QPainter *p) override;
    QRect rect() const override;
    bool layered() const override
    {
        return false;
    }
};

/**
    \class KGameCanvasRectangle kgamecanvas.h <KGameCanvas>
    \brief A solid rectangle.

    A KGameCanvasRectangle is a rectangle that can be put in the canvas.

    \deprecated For new applications, use Qt's Graphics View framework or Qt Quick.
*/
class KDEGAMESPRIVATE_EXPORT KGameCanvasRectangle : public KGameCanvasItem
{
private:
    QColor m_color;
    QSize m_size;

public:
    /** Constructor, specifying the pixmap and the parameters to use */
    KGameCanvasRectangle(const QColor &color, QSize size, KGameCanvasAbstract *canvas = nullptr);

    /** Constructor, creating with an empty pixmap */
    explicit KGameCanvasRectangle(KGameCanvasAbstract *canvas = nullptr);

    ~KGameCanvasRectangle() override;

    /** Returns the color */
    QColor color() const
    {
        return m_color;
    }

    /** Sets the color */
    void setColor(const QColor &color);

    /** Sets the size */
    void setSize(QSize size);

    void paint(QPainter *p) override;
    QRect rect() const override;
    bool layered() const override
    {
        return false;
    }
};

/**
    \class KGameCanvasText kgamecanvas.h <KGameCanvas>
    \brief KGameCanvasText.

    A KGameCanvasText is a text that can be put in the canvas.

    \deprecated For new applications, use Qt's Graphics View framework or Qt Quick.
*/
class KDEGAMESPRIVATE_EXPORT KGameCanvasText : public KGameCanvasItem
{
public:
    /** Specifies the meaning of the x coordinate of the item. It can
        refer to the start of the text, of the left, center, right of
        the bounding rectangle. */
    enum HPos { HStart, HLeft, HRight, HCenter };

    /** Specifies the meaning of the y coordinate of the item. It can
        refer to the baseline of the text, of the top, center, bottom of
        the bounding rectangle. */
    enum VPos { VBaseline, VTop, VBottom, VCenter };

private:
    QString m_text;
    QColor m_color;
    QFont m_font;
    HPos m_hpos;
    VPos m_vpos;
    QRect m_bounding_rect;

    QPoint offsetToDrawPos() const;
    void calcBoundingRect();

public:
    /** Constructor, specifying the text and the parameters to use */
    KGameCanvasText(const QString &text, const QColor &color, const QFont &font, HPos hp, VPos vp, KGameCanvasAbstract *canvas = nullptr);

    /** Constructor, creating with an empty text */
    explicit KGameCanvasText(KGameCanvasAbstract *canvas = nullptr);

    ~KGameCanvasText() override;

    /** Returns the text */
    QString text() const
    {
        return m_text;
    }

    /** Sets the text */
    void setText(const QString &text);

    /** Returns the color */
    QColor color() const
    {
        return m_color;
    }

    /** Sets the color */
    void setColor(const QColor &color);

    /** Returns the font */
    QFont font() const
    {
        return m_font;
    }

    /** Sets the font */
    void setFont(const QFont &font);

    /** Returns the horizontal positioning style */
    HPos hPositioning() const
    {
        return m_hpos;
    }

    /** Returns the vertical positioning style */
    VPos vPositioning() const
    {
        return m_vpos;
    }

    /** Sets the positioning style */
    void setPositioning(HPos hp, VPos vp);

    void paint(QPainter *p) override;
    QRect rect() const override;
    bool layered() const override
    {
        return false;
    }
};

/**
    \class KGameCanvasWidget kgamecanvas.h <KGameCanvas>
    \brief Container widget.

    A KGameCanvasWidget is a widget that can contain many KGameCanvasItem (images, rectangles, lines, etc).
    Portions of the widget are automatically redrawn to update the changes made to the items.

    \deprecated For new applications, use Qt's Graphics View framework or Qt Quick.
*/
class KDEGAMESPRIVATE_EXPORT KGameCanvasWidget : public QWidget, public KGameCanvasAbstract
{
    Q_OBJECT

private:
    friend class KGameCanvasItem;
    friend class AnimationNotifier;

    class KGameCanvasWidgetPrivate *priv;

    void ensureAnimating() override;
    void ensurePendingUpdate() override;
    void invalidate(QRect r, bool translate = true) override;
    void invalidate(const QRegion &r, bool translate = true) override;

    void paintEvent(QPaintEvent *event) override;

private Q_SLOTS:
    void processAnimations();
    void updateChanges();

public:
    /** The constructor */
    explicit KGameCanvasWidget(QWidget *parent = nullptr);

    ~KGameCanvasWidget() override;

    /** Set the delay of the animation, in milliseconds */
    void setAnimationDelay(int d);

    /** Return the number of milliseconds from the creation of the canvas
        (see also KGameCanvasItem::advance)*/
    int mSecs();

    /** returns 'this' */
    KGameCanvasWidget *topLevelCanvas() override;

    /** @return 0 */
    QPoint canvasPosition() const override;
};

/**
    \class KGameCanvasAdapter kgamecanvas.h <KGameCanvas>

    \brief A generic adapter for KGameCanvas

    KGameCanvasAdapter can be used to draw content managed by KGameCanvas
    inside systems which do not use KGameCanvas internally for their
    rendering.

    For example, suppose you have a widget CustomDisplay which paints itself
    with direct QPainter calls in its paintEvent, and you want to add a
    complex element to its rendering, which is best implemented via the KGameCanvas
    abstractions. What you can do is to create a KGameCanvasAdapter subclass,
    use it just like a KGameCanvasGroup, then call its render member function
    to have a QPainter object draw it.

    A KGameCanvas adapter notifies its parent using the pure virtual function
    updateParent, which is called when the adapter content is invalidated.

    \todo Support animations

    \deprecated For new applications, use Qt's Graphics View framework or Qt Quick.
*/
class KDEGAMESPRIVATE_EXPORT KGameCanvasAdapter : public KGameCanvasAbstract
{
    QRect m_child_rect;
    bool m_child_rect_valid;
    QRect m_invalidated_rect;

    void ensureAnimating() override
    {
    }
    void ensurePendingUpdate() override;
    void invalidate(QRect r, bool translate = true) override;
    void invalidate(const QRegion &r, bool translate = true) override;

    QRect childRect();

public:
    /** Constructor */
    KGameCanvasAdapter();

    /**
     * An adapter is not associated to any canvas, so this function
     * simply returns 0.
     */
    class KGameCanvasWidget *topLevelCanvas() override
    {
        return nullptr;
    }

    /**
     * The absolute position of the rendered content is not well
     * defined for KGameCanvasAdapter. We assume that the adapter
     * will be rendered at (0,0), and leave it to the user to perform
     * the necessary adjustments, which will depend on the chosen
     * rendering method.
     *
     * @return The point (0, 0).
     */
    QPoint canvasPosition() const override
    {
        return QPoint(0, 0);
    }

    /**
     * Draw the items of the adapter using the specified painter.
     * \param p The QPainter object to be used for rendering.
     */
    virtual void render(QPainter *p);

    /**
     * Notify the parent that the adapter content inside \a rect needs
     * to be redrawn.
     *
     * \a rect The bounding rectangle of the region that needs repainting.
     */
    virtual void updateParent(QRect rect) = 0;
};

#endif //__KGRGAMECANVAS_H__
