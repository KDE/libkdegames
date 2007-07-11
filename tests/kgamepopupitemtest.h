#ifndef KGPI_TEST_H
#define KGPI_TEST_H

#include <KXmlGuiWindow>

#include "ui_kgamepopupitemtest.h"

class QGraphicsScene;
class KGamePopupItem;
class QGraphicsSimpleTextItem;

class KGpiMainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    KGpiMainWindow();
private slots:
    void onPopupTL();
    void onPopupTR();
    void onPopupBL();
    void onPopupBR();
    void onLinkClicked(const QString&);
    void hideTextItem();
    void onTimeoutChanged(int);
    void changeIcon();
    void doInstantHide();
    void doAnimatedHide();
    void changeOpacity(int);
    void textColorChanged(const QColor& col);
    void bkgndColorChanged(const QColor& col);
private:
    Ui::KGpiMainWidget m_mainWid;
    QGraphicsScene *m_scene;
    KGamePopupItem *m_popupItem;
    QGraphicsSimpleTextItem *m_textItem;
};

#endif
