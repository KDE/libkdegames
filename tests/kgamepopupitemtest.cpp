/*
    SPDX-FileCopyrightText: 2007 Dmitry Suzdalev <dimsuz@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgamepopupitemtest.h"

// KF
#include <KAboutData>
#include <KActionCollection>
#include <KColorScheme>
#include <KFileDialog>
#include <KLocalizedString>
// Qt
#include <QApplication>
#include <QCommandLineParser>
#include <QGraphicsScene>
#include <QTimer>

KGpiMainWindow::KGpiMainWindow()
    : KXmlGuiWindow()
    , m_replaceMode(KGamePopupItem::LeavePrevious)
{
    QWidget *wid = new QWidget(this);
    m_mainWid.setupUi(wid);
    m_popupItem = new KGamePopupItem;
    connect(m_popupItem, &KGamePopupItem::linkActivated, this, &KGpiMainWindow::onLinkClicked);
    m_textItem = new QGraphicsSimpleTextItem;

    KStandardAction::quit(this, &KGpiMainWindow::close, actionCollection());

    m_scene = new QGraphicsScene;
    m_scene->setSceneRect(-1000, -1000, 2000, 2000);
    QLinearGradient gradient(QPointF(-1000, -1000), QPointF(2000, 2000));
    gradient.setColorAt(0, Qt::white);
    gradient.setColorAt(1, Qt::blue);

    m_scene->setBackgroundBrush(gradient);

    m_scene->addItem(m_popupItem);
    m_scene->addItem(m_textItem);

    m_mainWid.graphicsView->setScene(m_scene);

    KColorScheme kcs(QPalette::Active, KColorScheme::Tooltip);
    m_mainWid.textColor->setColor(kcs.foreground(KColorScheme::NormalText).color());
    m_mainWid.bkgndColor->setColor(kcs.background().color());

    connect(m_mainWid.popupTL, &QPushButton::clicked, this, &KGpiMainWindow::onPopupTL);
    connect(m_mainWid.popupTR, &QPushButton::clicked, this, &KGpiMainWindow::onPopupTR);
    connect(m_mainWid.popupBL, &QPushButton::clicked, this, &KGpiMainWindow::onPopupBL);
    connect(m_mainWid.popupBR, &QPushButton::clicked, this, &KGpiMainWindow::onPopupBR);
    connect(m_mainWid.popupCenter, &QPushButton::clicked, this, &KGpiMainWindow::onPopupCenter);
    connect(m_mainWid.forceInstantHide, &QPushButton::clicked, this, &KGpiMainWindow::doInstantHide);
    connect(m_mainWid.forceAnimatedHide, &QPushButton::clicked, this, &KGpiMainWindow::doAnimatedHide);
    connect(m_mainWid.changeIcon, &QPushButton::clicked, this, &KGpiMainWindow::changeIcon);
    connect(m_mainWid.opacity, &QSlider::valueChanged, this, &KGpiMainWindow::changeOpacity);
    connect(m_mainWid.textColor, &KColorButton::changed, this, &KGpiMainWindow::textColorChanged);
    connect(m_mainWid.bkgndColor, &KColorButton::changed, this, &KGpiMainWindow::bkgndColorChanged);
    connect(m_mainWid.leavePrevious, &QRadioButton::clicked, this, &KGpiMainWindow::replaceModeChanged);
    connect(m_mainWid.replacePrevious, &QRadioButton::clicked, this, &KGpiMainWindow::replaceModeChanged);
    connect(m_mainWid.cornersType, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &KGpiMainWindow::sharpnessChanged);

    connect(m_mainWid.mesTimeout, static_cast<void (KIntSpinBox::*)(int)>(&KIntSpinBox::valueChanged), this, &KGpiMainWindow::onTimeoutChanged);
    m_mainWid.mesTimeout->setValue(m_popupItem->messageTimeout());
    m_mainWid.mesTimeout->setSuffix(ki18np(" millisecond", " milliseconds"));
    m_mainWid.opacity->setValue(static_cast<int>(m_popupItem->messageOpacity() * 100));
    setCentralWidget(wid);

    setupGUI();
}

int main(int argc, char **argv)
{
    KAboutData aboutData(QLatin1String("kgamepopupitemtest"), i18n("kgamepopupitemtest"), QLatin1String("0.1"));
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);
    KGpiMainWindow *win = new KGpiMainWindow;
    win->show();
    return a.exec();
}

void KGpiMainWindow::onPopupTL()
{
    QString str = !m_mainWid.showRichText->isChecked()
        ? "Heya! Popping up!"
        : "<font color=\"red\">Heya!</font> Click <a href=\"oh-oh-i-am-the-Link\">the link</a><br> and <b>a</b> message should appear in the scene";
    m_popupItem->showMessage(str, KGamePopupItem::TopLeft, m_replaceMode);
}

void KGpiMainWindow::onPopupTR()
{
    QString str = !m_mainWid.showRichText->isChecked()
        ? "Yippie! Popping up!"
        : "<font color=\"green\">Yippie!</font> Click <a href=\"oh-oh-i-am-the-Link\">the link</a><br> and <b>a</b> message should appear in the scene";
    m_popupItem->showMessage(str, KGamePopupItem::TopRight, m_replaceMode);
}

void KGpiMainWindow::onPopupBL()
{
    QString str = !m_mainWid.showRichText->isChecked() ? "Horray! Popping up!"
                                                       : "<font color=\"yellow\">Horray</font><i>,comrades</i>! Click <a href=\"oh-oh-i-am-the-Link\">the "
                                                         "link</a><br> and <b>a</b> message should appear in the scene";
    m_popupItem->showMessage(str, KGamePopupItem::BottomLeft, m_replaceMode);
}

void KGpiMainWindow::onPopupBR()
{
    QString str = !m_mainWid.showRichText->isChecked() ? "Popping up! I like it, yeah!"
                                                       : "<font color=\"blue\">Wow. Just blue WOW</font>! Click <a href=\"oh-oh-i-am-the-Link\">the "
                                                         "link</a><br> and <b>a</b> message should appear in the scene";
    m_popupItem->showMessage(str, KGamePopupItem::BottomRight, m_replaceMode);
}

void KGpiMainWindow::onLinkClicked(const QString &link)
{
    QRectF visibleRect = m_mainWid.graphicsView->mapToScene(m_mainWid.graphicsView->contentsRect()).boundingRect();
    m_textItem->setText("Hi! I'm the message that should appear :-). You clicked link: " + link);
    m_textItem->setPos(visibleRect.left() + visibleRect.width() / 2 - m_textItem->boundingRect().width() / 2, visibleRect.top() + visibleRect.height() / 2);
    m_textItem->show();
    QTimer::singleShot(2000, this, &KGpiMainWindow::hideTextItem);
}

void KGpiMainWindow::hideTextItem()
{
    m_textItem->hide();
}

void KGpiMainWindow::onTimeoutChanged(int msec)
{
    m_popupItem->setMessageTimeout(msec);
}

void KGpiMainWindow::changeIcon()
{
    QString newPix = KFileDialog::getImageOpenUrl(KUrl(), this).path();
    if (newPix.isEmpty())
        return;
    QPixmap pix(newPix);
    m_popupItem->setMessageIcon(newPix);
}

void KGpiMainWindow::doInstantHide()
{
    m_popupItem->forceHide(KGamePopupItem::InstantHide);
}

void KGpiMainWindow::doAnimatedHide()
{
    m_popupItem->forceHide(KGamePopupItem::AnimatedHide);
}

void KGpiMainWindow::changeOpacity(int opa)
{
    m_popupItem->setMessageOpacity(opa / 100.0);
}

void KGpiMainWindow::textColorChanged(const QColor &col)
{
    m_popupItem->setTextColor(col);
}

void KGpiMainWindow::bkgndColorChanged(const QColor &col)
{
    m_popupItem->setBackgroundBrush(col);
}

void KGpiMainWindow::replaceModeChanged()
{
    if (m_mainWid.leavePrevious->isChecked())
        m_replaceMode = KGamePopupItem::LeavePrevious;
    else
        m_replaceMode = KGamePopupItem::ReplacePrevious;
}

void KGpiMainWindow::sharpnessChanged(int idx)
{
    if (idx == 0)
        m_popupItem->setSharpness(KGamePopupItem::Square);
    else if (idx == 1)
        m_popupItem->setSharpness(KGamePopupItem::Sharp);
    else if (idx == 2)
        m_popupItem->setSharpness(KGamePopupItem::Soft);
    else if (idx == 3)
        m_popupItem->setSharpness(KGamePopupItem::Softest);
}

void KGpiMainWindow::onPopupCenter()
{
    QString str = !m_mainWid.showRichText->isChecked()
        ? "Popping, popping, popping up!"
        : "<font color=\"red\">Heya!</font> Click <a href=\"oh-oh-i-am-the-Link\">the link</a><br> and <b>a</b> message should appear in the scene";
    m_popupItem->showMessage(str, KGamePopupItem::Center, m_replaceMode);
}

#include "kgamepopupitemtest.moc"
