/***************************************************************************
                          KCardDialog  -  Change player names
                             -------------------
    begin                : 31 Oct 2000
    copyright            : (C) 2000 by Martin Heni
    email                : martin@heni-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*
    $Id$
*/

#include <stdio.h>

#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qdir.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <kapp.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kiconview.h>
#include <kimageio.h>
#include "kcarddialog.h"
#include <kdebug.h>
#include <ksimpleconfig.h>
#include <qpixmap.h>
#include <assert.h>

#define KCARD_DEFAULTDECK QString::fromLatin1("deck0.png")
#define KCARD_DEFAULTCARD QString::fromLatin1("11.png")
#define KCARD_DEFAULTCARDDIR QString::fromLatin1("cards-default/")

int KCardDialog::getCardDeck(QString &mDeck, QString &mCarddir, QWidget *mParent,
                             CardFlags mFlags, bool* mRandomDeck, bool* mRandomCardDir)
{
    KCardDialog dlg(mParent, "dlg", mFlags);

    dlg.setDeck(mDeck);
    dlg.setCardDir(mCarddir);

    dlg.setupDialog();
    dlg.showRandomDeckBox(mRandomDeck != 0);
    dlg.showRandomCardDirBox(mRandomCardDir != 0);
    int result=dlg.exec();
    if (result==QDialog::Accepted)
    {
        mDeck=dlg.deck();
        mCarddir=dlg.cardDir();
        if (!mCarddir.isNull() && mCarddir.right(1)!=QString::fromLatin1("/"))
        {
            mCarddir+=QString::fromLatin1("/");
        }
        if (mRandomDeck)
        {
            *mRandomDeck = dlg.isRandomDeck();
        }
        if (mRandomCardDir)
        {
            *mRandomCardDir = dlg.isRandomCardDir();
        }
    }
    return result;
}

QString KCardDialog::getDefaultDeck()
{
    KCardDialog::init();
    return locate("cards", QString::fromLatin1("decks/") + KCARD_DEFAULTDECK);
}

QString KCardDialog::getDefaultCardDir()
{
    KCardDialog::init();

    QString file = KCARD_DEFAULTCARDDIR + KCARD_DEFAULTCARD;
    return KGlobal::dirs()->findResourceDir("cards",file) + KCARD_DEFAULTCARDDIR;
}

QString KCardDialog::getCardPath(const QString &carddir, int index)
{
    KCardDialog::init();

    QString entry = carddir + QString::number(index);
    if (KStandardDirs::exists(entry + QString::fromLatin1(".png")))
        return entry + QString::fromLatin1(".png");

    // rather theoretical
    if (KStandardDirs::exists(entry + QString::fromLatin1(".xpm")))
        return entry + QString::fromLatin1(".xpm");

    return QString::null;
}

QString KCardDialog::deck() const { return cDeck; }
void KCardDialog::setDeck(const QString& file) { cDeck=file; }
QString KCardDialog::cardDir() const { return cCardDir; }
void KCardDialog::setCardDir(const QString& dir) { cCardDir=dir; }
bool KCardDialog::isRandomDeck() const
{
  if (randomDeck) return randomDeck->isChecked();
  else return false;
}
bool KCardDialog::isRandomCardDir() const
{
  if (randomCardDir) return randomCardDir->isChecked();
  else return false;
}

void KCardDialog::setupDialog()
{
  QVBoxLayout* topLayout = new QVBoxLayout(plainPage(), spacingHint());
  QString path, file;
  QWMatrix m;
  m.scale(0.8,0.8);

  setInitialSize(QSize(600,400));

  if (! (flags() & NoDeck))
  {
    QHBoxLayout* layout = new QHBoxLayout(topLayout);

    // Deck iconview
    QGroupBox* grp1 = new QGroupBox(1, Horizontal, i18n("Choose backside"), plainPage());
    layout->addWidget(grp1);

    deckIconView = new KIconView(grp1,"decks");
    deckIconView->setSpacing(8);
    /*
    deckIconView->setGridX(-1);
    deckIconView->setGridY(50);
    */
    deckIconView->setGridX(82);
    deckIconView->setGridY(106);
    deckIconView->setSelectionMode(QIconView::Single);
    deckIconView->setResizeMode(QIconView::Adjust);
    deckIconView->setMinimumWidth(360);
    deckIconView->setMinimumHeight(170);
    deckIconView->setWordWrapIconText(false);
    deckIconView->showToolTips();

    // deck select
    QVBoxLayout* l = new QVBoxLayout(layout);
    QGroupBox* grp3 = new QGroupBox(i18n("Backside"), plainPage());
    grp3->setFixedSize(100, 130);
    l->addWidget(grp3, 0, AlignTop|AlignHCenter);
    deckLabel = new QLabel(grp3);
    deckLabel->setText(i18n("empty"));
    deckLabel->setAlignment(AlignHCenter|AlignVCenter);
    deckLabel->setGeometry(10, 20, 80, 90);

    randomDeck = new QCheckBox(plainPage());
    randomDeck->setChecked(false);
    connect(randomDeck, SIGNAL(toggled(bool)), this,
            SLOT(slotRandomDeckToggled(bool)));
    randomDeck->setText(i18n("Random backside"));
    l->addWidget(randomDeck, 0, AlignTop|AlignHCenter);

    connect(deckIconView,SIGNAL(clicked(QIconViewItem *)),
            this,SLOT(slotDeckClicked(QIconViewItem *)));
  }

  if (! (flags() & NoCards))
  {
    // Cards iconview
    QHBoxLayout* layout = new QHBoxLayout(topLayout);
    QGroupBox* grp2 = new QGroupBox(1, Horizontal, i18n("Choose frontside"), plainPage());
    layout->addWidget(grp2);

    cardIconView =new KIconView(grp2,"cards");
    /*
    cardIconView->setGridX(36);
    cardIconView->setGridY(50);
    */
    cardIconView->setGridX(82);
    cardIconView->setGridY(106);
    cardIconView->setResizeMode(QIconView::Adjust);
    cardIconView->setMinimumWidth(360);
    cardIconView->setMinimumHeight(170);
    cardIconView->setWordWrapIconText(false);
    cardIconView->showToolTips();

    // Card select
    QVBoxLayout* l = new QVBoxLayout(layout);
    QGroupBox* grp4 = new QGroupBox(i18n("Frontside"), plainPage());
    grp4->setFixedSize(100, 130);
    l->addWidget(grp4, 0, AlignTop|AlignHCenter);
    cardLabel = new QLabel(grp4);
    cardLabel->setText(i18n("empty"));
    cardLabel->setAlignment(AlignHCenter|AlignVCenter);
    cardLabel->setGeometry(10, 20, 80, 90 );

    randomCardDir = new QCheckBox(plainPage());
    randomCardDir->setChecked(false);
    connect(randomCardDir, SIGNAL(toggled(bool)), this,
            SLOT(slotRandomCardDirToggled(bool)));
    randomCardDir->setText(i18n("Random frontside"));
    l->addWidget(randomCardDir, 0, AlignTop|AlignHCenter);

    connect(cardIconView,SIGNAL(clicked(QIconViewItem *)),
            this,SLOT(slotCardClicked(QIconViewItem *)));
  }

  // Insert deck icons
  // First find the default or alternate path
  if (! (flags() & NoDeck))
  {
      insertDeckIcons();
      deckIconView->arrangeItemsInGrid();

      // Set default icons if given
      if (!deck().isNull())
      {
          file=deck();
          QPixmap pixmap(file);
          pixmap=pixmap.xForm(m);
          deckLabel->setPixmap(pixmap);
      }
  }

  // Insert card icons
  if (! (flags() & NoCards))
  {
      insertCardIcons();
      cardIconView->arrangeItemsInGrid();

    // Set default icons if given
    if (!cardDir().isNull())
    {
        file = cardDir() + KCARD_DEFAULTCARD;
        QPixmap pixmap(file);
        pixmap = pixmap.xForm(m);
        cardLabel->setPixmap(pixmap);
    }
  }

}

void KCardDialog::insertCardIcons()
{
    QStringList list = KGlobal::dirs()->findAllResources("cards", "card*/index.desktop", false, true);
    kdDebug() << "insert " << list.count() << endl;
    if (list.isEmpty())
        return;

    // We shrink the icons a little
    QWMatrix m;
    m.scale(0.8,0.8);

    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        KSimpleConfig cfg(*it);

        cfg.setGroup(QString::fromLatin1("KDE Backdeck"));
        QString path = (*it).left((*it).findRev('/') + 1);
        assert(path[path.length() - 1] == '/');
        QPixmap pixmap(path + cfg.readEntry("Preview", "12c.png"));

        if (pixmap.isNull())
            continue;

        QIconViewItem *item= new QIconViewItem(cardIconView, cfg.readEntry("Name", i18n("unnamed")), pixmap);

        item->setDragEnabled(false);
        item->setDropEnabled(false);
        item->setRenameEnabled(false);
        item->setSelectable(true);

        cardMap[item] = path;
    }
}

void KCardDialog::insertDeckIcons()
{
    QStringList list = KGlobal::dirs()->findAllResources("cards", "decks/*.desktop", false, true);
    if (list.isEmpty())
        return;

    QString label;

    // We shrink the icons a little
    QWMatrix m;
    m.scale(0.8,0.8);

    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        KSimpleConfig cfg(*it);
        QPixmap pixmap(getDeckName(*it));
        if (pixmap.isNull())
            continue;

        pixmap=pixmap.xForm(m);

        cfg.setGroup(QString::fromLatin1("KDE Cards"));
        QIconViewItem *item= new QIconViewItem(deckIconView, cfg.readEntry("Name", i18n("unnamed")), pixmap);

        item->setDragEnabled(false);
        item->setDropEnabled(false);
        item->setRenameEnabled(false);

        deckMap[item] = getDeckName(*it);
    }
}


KCardDialog::~KCardDialog()
{
}


// Create the dialog
KCardDialog::KCardDialog( QWidget *parent, const char *name, CardFlags mFlags)
    : KDialogBase( Plain, i18n("Carddeck selection"), Ok|Cancel, Ok, parent, name, true, true)
{
    KCardDialog::init();

    randomDeck = 0;
    randomCardDir = 0;
    cFlags = mFlags;
}

void KCardDialog::slotDeckClicked(QIconViewItem *item)
{
    if (item && item->pixmap())
    {
        deckLabel->setPixmap(* (item->pixmap()));
        setDeck(deckMap[item]);
    }
}
void KCardDialog::slotCardClicked(QIconViewItem *item)
{
    if (item && item->pixmap())
    {
        cardLabel->setPixmap(* (item->pixmap()));
        QString path = cardMap[item];
        setCardDir(path);
    }
}

QString KCardDialog::getDeckName(const QString &desktop)
{
    QString entry = desktop.left(desktop.length() - strlen(".desktop"));
    if (KStandardDirs::exists(entry + QString::fromLatin1(".png")))
        return entry + QString::fromLatin1(".png");

    // rather theoretical
    if (KStandardDirs::exists(entry + QString::fromLatin1(".xpm")))
        return entry + QString::fromLatin1(".xpm");
    return QString::null;
}

QString KCardDialog::getRandomDeck()
{
    KCardDialog::init();

    QStringList list = KGlobal::dirs()->findAllResources("cards", "decks/*.desktop");
    if (list.isEmpty())
        return QString::null;

    int d = KApplication::random() % list.count();
    return getDeckName(*list.at(d));
}

QString KCardDialog::getRandomCardDir()
{
    KCardDialog::init();

    QStringList list = KGlobal::dirs()->findAllResources("cards", "card*/index.desktop");
    if (list.isEmpty())
        return QString::null;

    int d = KApplication::random() % list.count();
    QString entry = *list.at(d);
    return entry.left(entry.length() - strlen("index.desktop"));
}

void KCardDialog::showRandomDeckBox(bool s)
{
    if (!randomDeck)
	return;

    if (s)
        randomDeck->show();
    else
        randomDeck->hide();
}

void KCardDialog::showRandomCardDirBox(bool s)
{
    if (!randomCardDir)
	return;

    if (s)
        randomCardDir->show();
    else
        randomCardDir->hide();
}

void KCardDialog::slotRandomDeckToggled(bool on)
{
  if (on) {
    deckLabel->setText("random");
    setDeck(getRandomDeck());
  } else {
    deckLabel->setText("empty");
    setDeck(0);
  }
}

void KCardDialog::slotRandomCardDirToggled(bool on)
{
  if (on) {
      cardLabel->setText("random");
      setCardDir(getRandomCardDir());
      if (cardDir().length()>0 && cardDir().right(1)!=QString::fromLatin1("/"))  {
          setCardDir(cardDir() + QString::fromLatin1("/"));
      }
  } else {
      cardLabel->setText("empty");
      setCardDir(0);
  }
}

void KCardDialog::init()
{
    static bool _inited = false;
    if (_inited)
        return;
    KGlobal::dirs()->addResourceType("cards", KStandardDirs::kde_default("data") + QString::fromLatin1("carddecks/"));
#ifdef SRCDIR
    KGlobal::dirs()->addResourceDir("cards", SRCDIR + QString::fromLatin1("/carddecks/"));
#endif

    KGlobal::locale()->insertCatalogue("libkdegames");

}

#include "kcarddialog.moc"
