#include "kgamepopupitemtest.h"
#include <KApplication>
#include <KActionCollection>
#include <KCmdLineArgs>
#include <KLocale>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPushButton>
#include <KGamePopupItem>

KGpiMainWindow::KGpiMainWindow()
    : KXmlGuiWindow()
{
    QWidget *wid = new QWidget(this);
    m_mainWid.setupUi(wid);
    m_popupItem = new KGamePopupItem;

    actionCollection()->addAction( KStandardAction::Quit, this, SLOT(close()) );

    m_scene = new QGraphicsScene;
    m_scene->setSceneRect( -1000, 1000, 2000, 2000 );
    m_scene->addItem(m_popupItem);

    m_mainWid.graphicsView->setScene(m_scene);

    connect( m_mainWid.popupTL, SIGNAL(clicked()), SLOT( onPopupTL() ) );
    connect( m_mainWid.popupTR, SIGNAL(clicked()), SLOT( onPopupTR() ) );
    connect( m_mainWid.popupBL, SIGNAL(clicked()), SLOT( onPopupBL() ) );
    connect( m_mainWid.popupBR, SIGNAL(clicked()), SLOT( onPopupBR() ) );

    setCentralWidget(wid);

    setupGUI();
}

int main(int argc, char** argv)
{
    KCmdLineArgs::init(argc, argv, "kgamepopupitemtest", QByteArray(), ki18n("kgamepopupitemtest"), "0.1");
    KApplication a;
    KGpiMainWindow *win = new KGpiMainWindow;
    win->show();
    return a.exec();
}

void KGpiMainWindow::onPopupTL()
{
    m_popupItem->showMessage("Heya! Popping up!", KGamePopupItem::TopLeft);
}

void KGpiMainWindow::onPopupTR()
{
    m_popupItem->showMessage("Yippie! Popping up!", KGamePopupItem::TopRight);
}

void KGpiMainWindow::onPopupBL()
{
    m_popupItem->showMessage("Hoorray! Popping up!", KGamePopupItem::BottomLeft);
}

void KGpiMainWindow::onPopupBR()
{
    m_popupItem->showMessage("Popping up! I like it, yeah!", KGamePopupItem::BottomRight);
}

#include "kgamepopupitemtest.moc"
