/*  Originally created for KBoard
    Copyright 2006 Maurizio Monge <maurizio.monge@gmail.com>

BSD License
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// KGame namespace changes: mauricio@tabuleiro.com

#include "kgamecanvas.h"

#include <QPaintEvent>
#include <QPainter>
#include <QRegion>
#include <QApplication>
#include <QTimer>
#include <QTime>
#include <QElapsedTimer>

/*
  TODO:
    - (maybe) allow an item to be destroyed while calling KGameCanvasItem::advance.
    - When a group is hidden/destroyed should only update items (optimize for sparse groups)
*/

#define DEBUG_DONT_MERGE_UPDATES 0
#define DEBUG_CANVAS_PAINTS      0

/*
    KGameCanvasAbstract
*/
KGameCanvasAbstract::KGameCanvasAbstract() {

}

KGameCanvasAbstract::~KGameCanvasAbstract() {
   //Note: this does not delete the items, be sure not to leak memory!
   for(int i=0;i<m_items.size();i++)
     m_items[i]->m_canvas = NULL;
}

KGameCanvasItem* KGameCanvasAbstract::itemAt(const QPoint &pt) const {
  for(int i=m_items.size()-1;i>=0;i--) {
    KGameCanvasItem *el = m_items[i];
    if(el->m_visible && el->rect().contains(pt))
      return el;
  }
  return NULL;
}

QList<KGameCanvasItem*> KGameCanvasAbstract::itemsAt(const QPoint &pt) const {
  QList<KGameCanvasItem*> retv;

  for(int i=m_items.size()-1;i>=0;i--) {
    KGameCanvasItem *el = m_items[i];
    if(el->m_visible && el->rect().contains(pt))
      retv.append(el);
  }

  return retv;
}


/*
    KGameCanvasWidget
*/
class KGameCanvasWidgetPrivate {
public:
  QTimer m_anim_timer;
  QElapsedTimer m_anim_time;
  bool m_pending_update;
  QRegion m_pending_update_reg;

#if DEBUG_CANVAS_PAINTS
  bool debug_paints;
#endif //DEBUG_CANVAS_PAINTS

  KGameCanvasWidgetPrivate()
  : m_pending_update(false)
#if DEBUG_CANVAS_PAINTS
  , debug_paints(false)
#endif //DEBUG_CANVAS_PAINTS
  {}
};

KGameCanvasWidget::KGameCanvasWidget(QWidget* parent)
: QWidget(parent)
, priv(new KGameCanvasWidgetPrivate()) {
  priv->m_anim_time.start();
  connect(&priv->m_anim_timer, &QTimer::timeout, this, &KGameCanvasWidget::processAnimations);
}

KGameCanvasWidget::~KGameCanvasWidget() {
  delete priv;
}

void KGameCanvasWidget::ensureAnimating() {
  if(!priv->m_anim_timer.isActive() )
      priv->m_anim_timer.start();
}

void KGameCanvasWidget::ensurePendingUpdate() {
  if(priv->m_pending_update)
    return;
  priv->m_pending_update = true;

#if DEBUG_DONT_MERGE_UPDATES
  updateChanges();
#else //DEBUG_DONT_MERGE_UPDATES
  QTimer::singleShot( 0, this, &KGameCanvasWidget::updateChanges );
#endif //DEBUG_DONT_MERGE_UPDATES
}

void KGameCanvasWidget::updateChanges() {
  for(int i=0;i<m_items.size();i++) {
    KGameCanvasItem *el = m_items.at(i);

    if(el->m_changed)
      el->updateChanges();
  }
  priv->m_pending_update = false;

#if DEBUG_CANVAS_PAINTS
  repaint();
  priv->debug_paints = true;
  repaint( priv->m_pending_update_reg );
  QApplication::syncX();
  priv->debug_paints = false;
  usleep(100000);
  repaint( priv->m_pending_update_reg );
  QApplication::syncX();
  usleep(100000);
#else //DEBUG_CANVAS_PAINTS
  repaint( priv->m_pending_update_reg );
#endif //DEBUG_CANVAS_PAINTS

  priv->m_pending_update_reg = QRegion();
}

void KGameCanvasWidget::invalidate(const QRect& r, bool /*translate*/) {
  priv->m_pending_update_reg |= r;
  ensurePendingUpdate();
}

void KGameCanvasWidget::invalidate(const QRegion& r, bool /*translate*/) {
  priv->m_pending_update_reg |= r;
  ensurePendingUpdate();
}

void KGameCanvasWidget::paintEvent(QPaintEvent *event) {
#if DEBUG_CANVAS_PAINTS
  if(priv->debug_paints)
  {
    QPainter p(this);
    p.fillRect(event->rect(), Qt::magenta);
    return;
  }
#endif //DEBUG_CANVAS_PAINTS

  {QPainter p(this);
  QRect evr = event->rect();
  QRegion evreg = event->region();

  for(int i=0;i<m_items.size();i++) {
    KGameCanvasItem *el = m_items.at(i);
    if( el->m_visible && evr.intersects( el->rect() )
        && evreg.contains( el->rect() ) ) {
      el->m_last_rect = el->rect();
      el->paintInternal(&p, evr, evreg, QPoint(), 1.0 );
    }
  }}
}

void KGameCanvasWidget::processAnimations() {
  if(m_animated_items.empty() ) {
    priv->m_anim_timer.stop();
    return;
  }

  int tm = priv->m_anim_time.elapsed();

  // The list MUST be copied, because it could be modified calling advance.
  // After all since it is implicitly shared the copy will not happen unless
  // is it actually modified
  QList<KGameCanvasItem*> ait = m_animated_items;
  for(int i=0;i<ait.size();i++) {
    KGameCanvasItem *el = ait[i];
    el->advance(tm);
  }

  if(m_animated_items.empty() )
    priv->m_anim_timer.stop();
}

void KGameCanvasWidget::setAnimationDelay(int d) {
  priv->m_anim_timer.setInterval(d);
}

int KGameCanvasWidget::mSecs() {
  return priv->m_anim_time.elapsed();
}

KGameCanvasWidget* KGameCanvasWidget::topLevelCanvas() {
  return this;
}

QPoint KGameCanvasWidget::canvasPosition() const {
	return QPoint(0, 0);
}

/*
    KGameCanvasItem
*/
KGameCanvasItem::KGameCanvasItem(KGameCanvasAbstract* KGameCanvas)
: m_visible(false)
, m_animated(false)
, m_opacity(255)
, m_pos(0,0)
, m_canvas(KGameCanvas)
, m_changed(false) {
  if(m_canvas) m_canvas->m_items.append(this);
}

KGameCanvasItem::~KGameCanvasItem() {
  if(m_canvas) {
    m_canvas->m_items.removeAll(this);
    if(m_animated)
      m_canvas->m_animated_items.removeAll(this);
    if(m_visible)
      m_canvas->invalidate(m_last_rect, false);
  }
}

void KGameCanvasItem::changed() {
  m_changed = true;

  //even if m_changed was already true we cannot optimize away this call, because maybe the
  //item has been reparented, etc. It is a very quick call anyway.
  if(m_canvas)
    m_canvas->ensurePendingUpdate();
}

void KGameCanvasItem::updateChanges() {
  if(!m_changed)
    return;
  if(m_canvas) {
    m_canvas->invalidate(m_last_rect, false);
    if(m_visible)
      m_canvas->invalidate(rect());
  }
  m_changed = false;
}

QPixmap *KGameCanvasItem::transparence_pixmap_cache = NULL;

QPixmap* KGameCanvasItem::getTransparenceCache(const QSize &s) {
  if(!transparence_pixmap_cache)
    transparence_pixmap_cache = new QPixmap();
  if(s.width()>transparence_pixmap_cache->width() ||
    s.height()>transparence_pixmap_cache->height()) {

    /* Strange that a pixmap with alpha should be created this way, i think a qt bug */
    *transparence_pixmap_cache = QPixmap::fromImage( QImage(
      s.expandedTo(transparence_pixmap_cache->size()), QImage::Format_ARGB32 ) );
  }

  return transparence_pixmap_cache;
}

void KGameCanvasItem::paintInternal(QPainter* pp, const QRect& /*prect*/,
                    const QRegion& /*preg*/, const QPoint& /*delta*/, double cumulative_opacity) {
  int opacity = int(cumulative_opacity*m_opacity + 0.5);

  if(opacity <= 0)
    return;

  if(opacity >= 255) {
    paint(pp);
    return;
  }

  if(!layered()) {
    pp->setOpacity(opacity/255.0);
    paint(pp);
    pp->setOpacity(1.0);
    return;
  }

  QRect mr = rect();
  QPixmap* cache = getTransparenceCache(mr.size());

  {
    QPainter p(cache);

    /* clear */
    p.setBrush(QColor(255,255,255,0));
    p.setPen(Qt::NoPen);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.drawRect(QRect(QPoint(),mr.size()));

    /* paint on the item */
    p.translate(-mr.topLeft());
    paint(&p);
    p.translate(mr.topLeft());

    /* make the opacity */
    p.setBrush( QColor(255,255,255,255-opacity) );
    p.setPen( Qt::NoPen );
    p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
    p.drawRect( QRect(QPoint(),mr.size()) );
  }

  pp->drawPixmap(mr.topLeft(), *cache, QRect(QPoint(),mr.size()) );
}

void KGameCanvasItem::putInCanvas(KGameCanvasAbstract *c) {
  if(m_canvas == c)
      return;

  if(m_canvas) {
    if(m_visible)
      m_canvas->invalidate(m_last_rect, false); //invalidate the previously drawn rectangle
    m_canvas->m_items.removeAll(this);
    if(m_animated)
      m_canvas->m_animated_items.removeAll(this);
  }

  m_canvas = c;

  if(m_canvas) {
    m_canvas->m_items.append(this);
    if(m_animated) {
      m_canvas->m_animated_items.append(this);
      m_canvas->ensureAnimating();
    }
    if(m_visible)
      changed();
  }
}

void KGameCanvasItem::setVisible(bool v) {
  if(m_visible == v)
      return;

  m_visible = v;
  if(m_canvas) {
    if(!v)
      m_canvas->invalidate(m_last_rect, false);
    else
      changed();
  }
  if(!v)
    m_last_rect = QRect();
}

void KGameCanvasItem::setAnimated(bool a) {
  if(m_animated == a)
    return;

  m_animated = a;
  if(m_canvas) {
    if(a) {
      m_canvas->m_animated_items.append(this);
      m_canvas->ensureAnimating();
    }
    else
      m_canvas->m_animated_items.removeAll(this);
  }
}

void KGameCanvasItem::setOpacity(int o) {
  if (o<0) o=0;
  if (o>255) o = 255;
  m_opacity = o;

  if(m_canvas && m_visible)
    changed();
}

bool KGameCanvasItem::layered() const { return true; }

void KGameCanvasItem::advance(int /*msecs*/) { }

void KGameCanvasItem::updateAfterRestack(int from, int to)
{
    int inc = from>to ? -1 : 1;

    QRegion upd;
    for(int i=from; i!=to;i+=inc)
    {
        KGameCanvasItem *el = m_canvas->m_items.at(i);
        if(!el->m_visible)
            continue;

        QRect r = el->rect() & rect();
        if(!r.isEmpty())
            upd |= r;
    }

    if(!upd.isEmpty())
        m_canvas->invalidate(upd);
}

void KGameCanvasItem::raise()
{
    if(!m_canvas || m_canvas->m_items.last() == this)
        return;

    int old_pos = m_canvas->m_items.indexOf(this);
    m_canvas->m_items.removeAt(old_pos);
    m_canvas->m_items.append(this);
    if(m_visible)
        updateAfterRestack(old_pos, m_canvas->m_items.size()-1);
}

void KGameCanvasItem::lower()
{
    if(!m_canvas || m_canvas->m_items.first() == this)
        return;

    int old_pos = m_canvas->m_items.indexOf(this);
    m_canvas->m_items.removeAt(old_pos);
    m_canvas->m_items.prepend(this);

    if(m_visible)
        updateAfterRestack(old_pos, 0);
}

void KGameCanvasItem::stackOver(KGameCanvasItem* ref)
{
    if(!m_canvas)
        return;

    if(ref->m_canvas != m_canvas)
    {
        qCritical("KGameCanvasItem::stackOver: Argument must be a sibling item!\n");
        return;
    }

    int i = m_canvas->m_items.indexOf( ref );
    if(i < m_canvas->m_items.size()-2  &&  m_canvas->m_items[i+1] == this)
        return;

    int old_pos = m_canvas->m_items.indexOf(this);
    m_canvas->m_items.removeAt(old_pos);
    i = m_canvas->m_items.indexOf(ref);
    m_canvas->m_items.insert(i+1,this);

    if(m_visible)
        updateAfterRestack(old_pos, i+1);
}

void KGameCanvasItem::stackUnder(KGameCanvasItem* ref)
{
    if(!m_canvas)
        return;


    if(ref->m_canvas != m_canvas)
    {
        qCritical("KGameCanvasItem::stackUnder: Argument must be a sibling item!\n");
        return;
    }

    int i = m_canvas->m_items.indexOf( ref );
    if(i >= 1  &&  m_canvas->m_items[i-1] == this)
        return;

    int old_pos = m_canvas->m_items.indexOf(this);
    m_canvas->m_items.removeAt(old_pos);
    i = m_canvas->m_items.indexOf(ref);
    m_canvas->m_items.insert(i,this);

    if(m_visible)
        updateAfterRestack(old_pos, i);
}

void KGameCanvasItem::moveTo(const QPoint &newpos)
{
  if(m_pos == newpos)
    return;
  m_pos = newpos;
  if(m_visible && m_canvas)
    changed();
}

QPoint KGameCanvasItem::absolutePosition() const
{
	if (m_canvas) {
		return m_canvas->canvasPosition() + m_pos;
	}
	else {
		return m_pos;
	}
}

/*
    KGameCanvasGroup
*/
KGameCanvasGroup::KGameCanvasGroup(KGameCanvasAbstract* KGameCanvas)
: KGameCanvasItem(KGameCanvas)
, KGameCanvasAbstract()
, m_child_rect_changed(true) {

}

KGameCanvasGroup::~KGameCanvasGroup() {

}

void KGameCanvasGroup::ensureAnimating() {
  setAnimated(true);
}

void KGameCanvasGroup::ensurePendingUpdate() {
   if(!m_changed || !m_child_rect_changed) {
     m_child_rect_changed = true;
     KGameCanvasItem::changed();
   }
}

void KGameCanvasGroup::updateChanges() {
  if(!m_changed)
    return;
  for(int i=0;i<m_items.size();i++) {
    KGameCanvasItem *el = m_items.at(i);

    if(el->m_changed)
      el->updateChanges();
  }
  m_changed = false;
}

void KGameCanvasGroup::changed() {
  if(!m_changed) {
    KGameCanvasItem::changed();

    for(int i=0;i<m_items.size();i++)
      m_items[i]->changed();
  }
}

void KGameCanvasGroup::invalidate(const QRect& r, bool translate) {
  if(m_canvas)
    m_canvas->invalidate(translate ? r.translated(m_pos) : r, translate);
  if(!m_changed)
    ensurePendingUpdate();
}

void KGameCanvasGroup::invalidate(const QRegion& r, bool translate) {
  if(m_canvas)
    m_canvas->invalidate(translate ? r.translated(m_pos) : r, translate);
  if(!m_changed)
    ensurePendingUpdate();
}

void KGameCanvasGroup::advance(int msecs) {

  // The list MUST be copied, because it could be modified calling advance.
  // After all since it is implicitly shared the copy will not happen unless
  // is it actually modified
  QList<KGameCanvasItem*> ait = m_animated_items;
  for(int i=0;i<ait.size();i++)
  {
      KGameCanvasItem *el = ait[i];
      el->advance(msecs);
  }

  if(m_animated_items.empty())
      setAnimated(false);
}

void KGameCanvasGroup::paintInternal(QPainter* p, const QRect& prect,
          const QRegion& preg, const QPoint& delta, double cumulative_opacity) {
  cumulative_opacity *= (m_opacity/255.0);

  QPoint adelta = delta;
  adelta += m_pos;
  p->translate(m_pos);

  for(int i=0;i<m_items.size();i++) {
    KGameCanvasItem *el = m_items.at(i);
    QRect r = el->rect().translated(adelta);

    if( el->m_visible && prect.intersects( r ) && preg.contains( r ) ) {
      el->m_last_rect = r;
      el->paintInternal(p,prect,preg,adelta,cumulative_opacity);
    }
  }

  p->translate(-m_pos);
}

void KGameCanvasGroup::paint(QPainter* /*p*/) {
  Q_ASSERT(!"This function should never be called");
}

QRect KGameCanvasGroup::rect() const
{
  if(!m_child_rect_changed)
    return m_last_child_rect.translated(m_pos);

  m_child_rect_changed = false;
  m_last_child_rect = QRect();
  for(int i=0;i<m_items.size();i++)
  {
    KGameCanvasItem *el = m_items[i];
    if(el->m_visible)
      m_last_child_rect |= el->rect();
  }

  return m_last_child_rect.translated(m_pos);
}

KGameCanvasWidget* KGameCanvasGroup::topLevelCanvas()
{
    return m_canvas ? m_canvas->topLevelCanvas() : NULL;
}

QPoint KGameCanvasGroup::canvasPosition() const {
	return KGameCanvasItem::absolutePosition();
}

/*
    KGameCanvasDummy
*/
KGameCanvasDummy::KGameCanvasDummy(KGameCanvasAbstract* KGameCanvas)
    : KGameCanvasItem(KGameCanvas)
{

}

KGameCanvasDummy::~KGameCanvasDummy()
{

}

void KGameCanvasDummy::paint(QPainter* /*p*/) {
}

QRect KGameCanvasDummy::rect() const
{
    return QRect();
}


/*
    KGameCanvasPixmap
*/
KGameCanvasPixmap::KGameCanvasPixmap(const QPixmap& p, KGameCanvasAbstract* KGameCanvas)
    : KGameCanvasItem(KGameCanvas), m_pixmap(p) {

}

KGameCanvasPixmap::KGameCanvasPixmap(KGameCanvasAbstract* KGameCanvas)
    : KGameCanvasItem(KGameCanvas) {

}

KGameCanvasPixmap::~KGameCanvasPixmap() {

}

void KGameCanvasPixmap::setPixmap(const QPixmap& p) {
  m_pixmap = p;
  if(visible() && canvas() )
    changed();
}

void KGameCanvasPixmap::paint(QPainter* p) {
  p->drawPixmap(pos(), m_pixmap);
}

QRect KGameCanvasPixmap::rect() const {
    return QRect(pos(), m_pixmap.size());
}

KGameCanvasRenderedPixmap::KGameCanvasRenderedPixmap(KGameRenderer* renderer, const QString& spriteKey, KGameCanvasAbstract* canvas)
	: KGameCanvasPixmap(canvas)
	, KGameRendererClient(renderer, spriteKey)
{
}

void KGameCanvasRenderedPixmap::receivePixmap(const QPixmap& pixmap)
{
	KGameCanvasPixmap::setPixmap(pixmap);
}

/*
    KGameCanvasTiledPixmap
*/
KGameCanvasTiledPixmap::KGameCanvasTiledPixmap(const QPixmap& pixmap, const QSize &size, const QPoint &origin,
                        bool move_orig, KGameCanvasAbstract* KGameCanvas)
    : KGameCanvasItem(KGameCanvas)
    , m_pixmap(pixmap)
    , m_size(size)
    , m_origin(origin)
    , m_move_orig(move_orig) {

}

KGameCanvasTiledPixmap::KGameCanvasTiledPixmap(KGameCanvasAbstract* KGameCanvas)
    : KGameCanvasItem(KGameCanvas)
    , m_size(0,0)
    , m_origin(0,0)
    , m_move_orig(false) {

}

KGameCanvasTiledPixmap::~KGameCanvasTiledPixmap() {

}

void KGameCanvasTiledPixmap::setPixmap(const QPixmap& pixmap) {
    m_pixmap = pixmap;
    if(visible() && canvas() )
      changed();
}

void KGameCanvasTiledPixmap::setSize(const QSize &size) {
  m_size = size;
  if(visible() && canvas() )
    changed();
}

void KGameCanvasTiledPixmap::setOrigin(const QPoint &origin)
{
  m_origin = m_move_orig ? origin - pos() : origin;

  if(visible() && canvas() )
    changed();
}


void KGameCanvasTiledPixmap::setMoveOrigin(bool move_orig)
{
  if(move_orig && !m_move_orig)
      m_origin -= pos();
  if(move_orig && !m_move_orig)
      m_origin += pos();
  m_move_orig = move_orig;
}

void KGameCanvasTiledPixmap::paint(QPainter* p)
{
    if(m_move_orig)
        p->drawTiledPixmap( rect(), m_pixmap, m_origin);
    else
        p->drawTiledPixmap( rect(), m_pixmap, m_origin+pos() );
}

QRect KGameCanvasTiledPixmap::rect() const
{
    return QRect(pos(), m_size);
}


/*
    KGameCanvasRectangle
*/
KGameCanvasRectangle::KGameCanvasRectangle(const QColor& color, const QSize &size, KGameCanvasAbstract* KGameCanvas)
    : KGameCanvasItem(KGameCanvas)
    , m_color(color)
    , m_size(size)
{

}

KGameCanvasRectangle::KGameCanvasRectangle(KGameCanvasAbstract* KGameCanvas)
    : KGameCanvasItem(KGameCanvas)
    , m_size(0,0)
{

}

KGameCanvasRectangle::~KGameCanvasRectangle()
{

}

void KGameCanvasRectangle::setColor(const QColor& color)
{
  m_color = color;
  if(visible() && canvas() )
    changed();
}

void KGameCanvasRectangle::setSize(const QSize &size)
{
  m_size = size;
  if(visible() && canvas() )
    changed();
}

void KGameCanvasRectangle::paint(QPainter* p) {
  p->fillRect( rect(), m_color );
}

QRect KGameCanvasRectangle::rect() const {
    return QRect(pos(), m_size);
}


/*
    KGameCanvasText
*/
KGameCanvasText::KGameCanvasText(const QString& text, const QColor& color,
                        const QFont& font, HPos hp, VPos vp,
                        KGameCanvasAbstract* KGameCanvas)
    : KGameCanvasItem(KGameCanvas)
    , m_text(text)
    , m_color(color)
    , m_font(font)
    , m_hpos(hp)
    , m_vpos(vp) {
    calcBoundingRect();
}

KGameCanvasText::KGameCanvasText(KGameCanvasAbstract* KGameCanvas)
    : KGameCanvasItem(KGameCanvas)
    //, m_text("")
    , m_color(Qt::black)
    , m_font(QApplication::font())
    , m_hpos(HStart)
    , m_vpos(VBaseline) {

}

KGameCanvasText::~KGameCanvasText() {

}

void KGameCanvasText::calcBoundingRect() {
    m_bounding_rect = QFontMetrics(m_font).boundingRect(m_text);
    /*printf("b rect is %d %d %d %d\n",
        m_bounding_rect.x(),
        m_bounding_rect.y(),
        m_bounding_rect.width(),
        m_bounding_rect.height() );*/
}

void KGameCanvasText::setText(const QString& text) {
  if(m_text == text)
    return;
  m_text = text;
  calcBoundingRect();

  if(visible() && canvas() )
    changed();
}

void KGameCanvasText::setColor(const QColor& color) {
  m_color = color;
}

void KGameCanvasText::setFont(const QFont& font) {
  m_font = font;
  calcBoundingRect();

  if(visible() && canvas() )
    changed();
}

void KGameCanvasText::setPositioning(HPos hp, VPos vp) {
  pos() += offsetToDrawPos();
  m_hpos = hp;
  m_vpos = vp;
  pos() -= offsetToDrawPos();
}

QPoint KGameCanvasText::offsetToDrawPos() const {
    QPoint retv;

    switch(m_hpos) {
        case HStart:
            retv.setX(0);
            break;
        case HLeft:
            retv.setX(-m_bounding_rect.left());
            break;
        case HRight:
            retv.setX(-m_bounding_rect.right());
            break;
        case HCenter:
            retv.setX(-(m_bounding_rect.left()+m_bounding_rect.right())/2);
            break;
    }

    switch(m_vpos) {
        case VBaseline:
            retv.setY(0);
            break;
        case VTop:
            retv.setY(-m_bounding_rect.top());
            break;
        case VBottom:
            retv.setY(-m_bounding_rect.bottom());
            break;
        case VCenter:
            retv.setY(-(m_bounding_rect.top()+m_bounding_rect.bottom())/2);
            break;
    }

    return retv;
}

void KGameCanvasText::paint(QPainter* p) {
  p->setPen(m_color);
  p->setFont(m_font);
  p->drawText( pos() + offsetToDrawPos(), m_text);
}

QRect KGameCanvasText::rect() const {
    return m_bounding_rect.translated( pos() + offsetToDrawPos() );
}


/*
    KGameCanvasPicture
*/
KGameCanvasPicture::KGameCanvasPicture(const QPicture& p, KGameCanvasAbstract* KGameCanvas)
    : KGameCanvasItem(KGameCanvas), m_picture(p)
{

}

KGameCanvasPicture::KGameCanvasPicture(KGameCanvasAbstract* KGameCanvas)
    : KGameCanvasItem(KGameCanvas)
{

}

KGameCanvasPicture::~KGameCanvasPicture()
{

}

void KGameCanvasPicture::setPicture(const QPicture& p)
{
  m_picture = p;

  if(visible() && canvas() )
    changed();
}

void KGameCanvasPicture::paint(QPainter* p)
{
    p->drawPicture(pos(), m_picture);
}

QRect KGameCanvasPicture::rect() const
{
    return m_picture.boundingRect().translated( pos());
}

KGameCanvasAdapter::KGameCanvasAdapter()
: m_child_rect_valid(false)
{
}

QRect KGameCanvasAdapter::childRect()
{
    if (!m_child_rect_valid) {
        m_child_rect = QRect();
        foreach (KGameCanvasItem* el, m_items) {
            m_child_rect |= el->rect();
        }
        m_child_rect_valid = true;
    }

    return m_child_rect;
}

void KGameCanvasAdapter::render(QPainter *painter)
{
    foreach (KGameCanvasItem* el, m_items) {
        if (el->m_visible) {
            el->m_last_rect = el->rect();
            el->paintInternal(painter, childRect(), childRect(), QPoint(), 1.0);
        }
    }
}

void KGameCanvasAdapter::ensurePendingUpdate()
{
    m_child_rect_valid = false;

    foreach (KGameCanvasItem* el, m_items) {
        if (el->m_changed) {
            el->updateChanges();
        }
    }

    updateParent(m_invalidated_rect);
    m_invalidated_rect = QRect();
}

void KGameCanvasAdapter::invalidate(const QRegion& r, bool)
{
    invalidate(r.boundingRect());
}

void KGameCanvasAdapter::invalidate(const QRect& r, bool)
{
    m_invalidated_rect |= r;
}


