#ifndef KGPI_TEST_H
#define KGPI_TEST_H

#include <KXmlGuiWindow>

#include "ui_kgamepopupitemtest.h"

class QGraphicsScene;
class KGamePopupItem;

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
private:
    Ui::KGpiMainWidget m_mainWid;
    QGraphicsScene *m_scene;
    KGamePopupItem *m_popupItem;
};

#endif
