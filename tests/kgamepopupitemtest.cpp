#include "kgamepopupitemtest.h"
#include <KApplication>
#include <KActionCollection>
#include <KCmdLineArgs>
#include <KLocale>
#include <QGraphicsScene>
#include <KGamePopupItem>
#include <QTimer>
#include <KDebug>

KGpiMainWindow::KGpiMainWindow()
    : KXmlGuiWindow()
{
    QWidget *wid = new QWidget(this);
    m_mainWid.setupUi(wid);
    m_popupItem = new KGamePopupItem;
    connect(m_popupItem, SIGNAL(linkActivated(const QString&)), SLOT(onLinkClicked(const QString&)) );
    m_textItem = new QGraphicsSimpleTextItem;

    actionCollection()->addAction( KStandardAction::Quit, this, SLOT(close()) );

    m_scene = new QGraphicsScene;
    m_scene->setSceneRect( -1000, -1000, 2000, 2000 );
    m_scene->addItem(m_popupItem);
    m_scene->addItem(m_textItem);

    m_mainWid.graphicsView->setScene(m_scene);

    connect( m_mainWid.popupTL, SIGNAL(clicked()), SLOT( onPopupTL() ) );
    connect( m_mainWid.popupTR, SIGNAL(clicked()), SLOT( onPopupTR() ) );
    connect( m_mainWid.popupBL, SIGNAL(clicked()), SLOT( onPopupBL() ) );
    connect( m_mainWid.popupBR, SIGNAL(clicked()), SLOT( onPopupBR() ) );

    connect( m_mainWid.mesTimeout, SIGNAL(valueChanged(int)), SLOT(onTimeoutChanged(int)) );
    m_mainWid.mesTimeout->setValue( m_popupItem->messageTimeout() );
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
    QString str = !m_mainWid.showRichText->isChecked() ? "Heya! Popping up!" : "<font color=\"red\">Heya!</font> Click the <a href=\"oh-oh-i-am-the-Link\">link</a><br> and <b>a</b> message should appear in the scene";
    m_popupItem->showMessage(str, KGamePopupItem::TopLeft);
}

void KGpiMainWindow::onPopupTR()
{
    QString str = !m_mainWid.showRichText->isChecked() ? "Yippie! Popping up!" : "<font color=\"green\">Yippie!</font> Click the <a href=\"oh-oh-i-am-the-Link\">link</a><br> and <b>a</b> message should appear in the scene";
    m_popupItem->showMessage(str, KGamePopupItem::TopRight);
}

void KGpiMainWindow::onPopupBL()
{
    QString str = !m_mainWid.showRichText->isChecked() ? "Horray! Popping up!" : "<font color=\"yellow\">Horray</font><i>,comrades</i>! Click the <a href=\"oh-oh-i-am-the-Link\">link</a><br> and <b>a</b> message should appear in the scene";
    m_popupItem->showMessage(str, KGamePopupItem::BottomLeft);
}

void KGpiMainWindow::onPopupBR()
{
    QString str = !m_mainWid.showRichText->isChecked() ? "Popping up! I like it, yeah!" : "<font color=\"blue\">Wow. Just blue WOW</font>! Click the <a href=\"oh-oh-i-am-the-Link\">link</a><br> and <b>a</b> message should appear in the scene";
    m_popupItem->showMessage(str, KGamePopupItem::BottomRight);
}

void KGpiMainWindow::onLinkClicked(const QString& link)
{
    QRectF visibleRect = m_mainWid.graphicsView->mapToScene( m_mainWid.graphicsView->contentsRect() ).boundingRect();
    m_textItem->setText( "Hi! I'm the message that should appear :-). You cliked link: "+link );
    m_textItem->setPos( visibleRect.left()+visibleRect.width()/2-m_textItem->boundingRect().width()/2,
                        visibleRect.top()+visibleRect.height()/2 );
    kDebug() << m_textItem->pos() << endl;
    m_textItem->show();
    QTimer::singleShot(2000, this, SLOT(hideTextItem()));
}

void KGpiMainWindow::hideTextItem()
{
    m_textItem->hide();
}

void KGpiMainWindow::onTimeoutChanged(int msec)
{
    m_popupItem->setMessageTimeout( msec );
}

#include "kgamepopupitemtest.moc"
