#ifndef KCANVASROOTPIXMAP_H
#define KCANVASROOTPIXMAP_H

#include <krootpixmap.h>

class QCanvasView;

/**
 * Implement KRootPixmap for a QCanvasView.
 *
 * The pixmap will be set as the background of the
 * QCanvas associated with the view :
 * <ul>
 * <li>for correct positioning of the background pixmap, the given
 * QCanvasView should be positioned at the origin of the canvas.</li>
 * <li>no other view of the same canvas should use KCanvasRootPixmap.</li>
 * <li>other views of the canvas will have the same background pixmap.</li>
 * </ul>
 */
class KCanvasRootPixmap : public KRootPixmap
{
 Q_OBJECT

 public:
    /**
     * Constructor.
     */
    KCanvasRootPixmap(QCanvasView *view, const char *name = 0);

 private slots:
    void backgroundUpdatedSlot(const QPixmap &);

 private:
    QCanvasView *_view;

    class KCanvasRootPixmapPrivate;
    KCanvasRootPixmapPrivate *d;
};

#endif

