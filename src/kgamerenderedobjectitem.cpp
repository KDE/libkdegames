/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgamerenderedobjectitem.h"

// own
#include "kgamerenderer.h"
// Qt
#include <QGraphicsView>
#include <QtMath>

class KGameRenderedObjectItemPrivate : public QGraphicsPixmapItem
{
public:
    KGameRenderedObjectItemPrivate(KGameRenderedObjectItem *parent);
    bool adjustRenderSize(); // returns whether an adjustment was made; WARNING: only call when m_primaryView != 0
    void adjustTransform();

    // QGraphicsItem reimplementations (see comment below for why we need all of this)
    bool contains(const QPointF &point) const override;
    bool isObscuredBy(const QGraphicsItem *item) const override;
    QPainterPath opaqueArea() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    QPainterPath shape() const override;

public:
    KGameRenderedObjectItem *m_parent;
    QGraphicsView *m_primaryView;
    QSize m_correctRenderSize;
    QSizeF m_fixedSize;
};

KGameRenderedObjectItemPrivate::KGameRenderedObjectItemPrivate(KGameRenderedObjectItem *parent)
    : QGraphicsPixmapItem(parent)
    , m_parent(parent)
    , m_primaryView(nullptr)
    , m_correctRenderSize(0, 0)
    , m_fixedSize(-1, -1)
{
}

static inline int vectorLength(QPointF point)
{
    return qSqrt(point.x() * point.x() + point.y() * point.y());
}

bool KGameRenderedObjectItemPrivate::adjustRenderSize()
{
    Q_ASSERT(m_primaryView);
    // create a polygon from the item's boundingRect
    const QRectF itemRect = m_parent->boundingRect();
    QPolygonF itemPolygon(3);
    itemPolygon[0] = itemRect.topLeft();
    itemPolygon[1] = itemRect.topRight();
    itemPolygon[2] = itemRect.bottomLeft();
    // determine correct render size
    const QPolygonF scenePolygon = m_parent->sceneTransform().map(itemPolygon);
    const QPolygon viewPolygon = m_primaryView->mapFromScene(scenePolygon);
    m_correctRenderSize.setWidth(qMax(vectorLength(viewPolygon[1] - viewPolygon[0]), 1));
    m_correctRenderSize.setHeight(qMax(vectorLength(viewPolygon[2] - viewPolygon[0]), 1));
    // ignore fluctuations in the render size which result from rounding errors
    const QSize diff = m_parent->renderSize() - m_correctRenderSize;
    if (qAbs(diff.width()) <= 1 && qAbs(diff.height()) <= 1) {
        return false;
    }
    m_parent->setRenderSize(m_correctRenderSize);
    adjustTransform();
    return true;
}

void KGameRenderedObjectItemPrivate::adjustTransform()
{
    // calculate new transform for this item
    QTransform t;
    t.scale(m_fixedSize.width() / m_correctRenderSize.width(), m_fixedSize.height() / m_correctRenderSize.height());
    // render item
    m_parent->prepareGeometryChange();
    setTransform(t);
    m_parent->update();
}

KGameRenderedObjectItem::KGameRenderedObjectItem(KGameRenderer *renderer, const QString &spriteKey, QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , KGameRendererClient(renderer, spriteKey)
    , d(new KGameRenderedObjectItemPrivate(this))
{
    setPrimaryView(renderer->defaultPrimaryView());
}

KGameRenderedObjectItem::~KGameRenderedObjectItem() = default;

QPointF KGameRenderedObjectItem::offset() const
{
    return d->pos();
}

void KGameRenderedObjectItem::setOffset(QPointF offset)
{
    if (d->pos() != offset) {
        prepareGeometryChange();
        d->setPos(offset);
        update();
    }
}

void KGameRenderedObjectItem::setOffset(qreal x, qreal y)
{
    setOffset(QPointF(x, y));
}

QSizeF KGameRenderedObjectItem::fixedSize() const
{
    return d->m_fixedSize;
}

void KGameRenderedObjectItem::setFixedSize(const QSizeF &fixedSize)
{
    if (d->m_primaryView) {
        d->m_fixedSize = fixedSize.expandedTo(QSize(1, 1));
        d->adjustTransform();
    }
}

QGraphicsView *KGameRenderedObjectItem::primaryView() const
{
    return d->m_primaryView;
}

void KGameRenderedObjectItem::setPrimaryView(QGraphicsView *view)
{
    if (d->m_primaryView != view) {
        d->m_primaryView = view;
        if (view) {
            if (!d->m_fixedSize.isValid()) {
                d->m_fixedSize = QSize(1, 1);
            }
            // determine render size and adjust coordinate system
            d->m_correctRenderSize = QSize(-10, -10); // force adjustment to be made
            d->adjustRenderSize();
        } else {
            d->m_fixedSize = QSize(-1, -1);
            // reset transform to make coordinate systems of this item and the private item equal
            prepareGeometryChange();
            d->setTransform(QTransform());
            update();
        }
    }
}

void KGameRenderedObjectItem::receivePixmap(const QPixmap &pixmap)
{
    prepareGeometryChange();
    d->setPixmap(pixmap);
    update();
}

// We want to make sure that all interactional events are sent ot this item, and
// not to the contained QGraphicsPixmapItem which provides the visual
// representation (and the metrics calculations).
// At the same time, we do not want the contained QGraphicsPixmapItem to slow
// down operations like QGraphicsScene::collidingItems().
// So the strategy is to use the QGraphicsPixmapItem implementation from
// KGameRenderedObjectItemPrivate for KGameRenderedObjectItem.
// Then the relevant methods in KGameRenderedObjectItemPrivate are reimplemented empty
// to effectively clear the item and hide it from any collision detection. This
// strategy allows us to use the nifty QGraphicsPixmapItem logic without exposing
// a QGraphicsPixmapItem subclass (which would conflict with QGraphicsObject).

// BEGIN QGraphicsItem reimplementation of KGameRenderedObjectItem

QRectF KGameRenderedObjectItem::boundingRect() const
{
    return d->mapRectToParent(d->QGraphicsPixmapItem::boundingRect());
}

bool KGameRenderedObjectItem::contains(const QPointF &point) const
{
    return d->QGraphicsPixmapItem::contains(d->mapFromParent(point));
}

bool KGameRenderedObjectItem::isObscuredBy(const QGraphicsItem *item) const
{
    return d->QGraphicsPixmapItem::isObscuredBy(item);
}

QPainterPath KGameRenderedObjectItem::opaqueArea() const
{
    return d->mapToParent(d->QGraphicsPixmapItem::opaqueArea());
}

void KGameRenderedObjectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)
}

QPainterPath KGameRenderedObjectItem::shape() const
{
    return d->mapToParent(d->QGraphicsPixmapItem::shape());
}

// END QGraphicsItem reimplementation of KGameRenderedObjectItem
// BEGIN QGraphicsItem reimplementation of KGameRenderedObjectItemPrivate

bool KGameRenderedObjectItemPrivate::contains(const QPointF &point) const
{
    Q_UNUSED(point)
    return false;
}

bool KGameRenderedObjectItemPrivate::isObscuredBy(const QGraphicsItem *item) const
{
    Q_UNUSED(item)
    return false;
}

QPainterPath KGameRenderedObjectItemPrivate::opaqueArea() const
{
    return QPainterPath();
}

void KGameRenderedObjectItemPrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Trivial stuff up to now. The fun stuff starts here. ;-)
    // There is no way to get informed when the viewport's coordinate system
    //(relative to this item's coordinate system) has changed, so we're checking
    // the renderSize in each paintEvent coming from the primary view.
    if (m_primaryView) {
        if (m_primaryView == widget || m_primaryView->isAncestorOf(widget)) {
            const bool isSimpleTransformation = !painter->transform().isRotating();
            // If an adjustment was made, do not paint now, but wait for the next
            // painting. However, paint directly if the transformation is
            // complex, in order to avoid flicker.
            if (adjustRenderSize()) {
                if (isSimpleTransformation) {
                    return;
                }
            }
            if (isSimpleTransformation) {
                // draw pixmap directly in physical coordinates
                const QPoint basePos = painter->transform().map(QPointF()).toPoint();
                painter->save();
                painter->setTransform(QTransform());
                painter->drawPixmap(basePos, pixmap());
                painter->restore();
                return;
            }
        }
    }
    QGraphicsPixmapItem::paint(painter, option, widget);
}

QPainterPath KGameRenderedObjectItemPrivate::shape() const
{
    return QPainterPath();
}

// END QGraphicsItem reimplementation of KGameRenderedObjectItemPrivate

#include "moc_kgamerenderedobjectitem.cpp"
