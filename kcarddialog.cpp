/*
    This file is part of the KDE games library
    Copyright (C) 2000 Martin Heni (martin@heni-online.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
/*
    $Id$
*/

#include <stdio.h>
#include <assert.h>

#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qdir.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qpixmap.h>
#include <qmap.h>

#include <kapp.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kiconview.h>
#include <kimageio.h>
#include <ksimpleconfig.h>
#include <kdebug.h>

#include "kcarddialog.h"

#define KCARD_DEFAULTDECK QString::fromLatin1("deck0.png")
#define KCARD_DEFAULTCARD QString::fromLatin1("11.png")
#define KCARD_DEFAULTCARDDIR QString::fromLatin1("cards-default/")


class KCardDialogPrivate
{
public:
    KCardDialogPrivate()
    {
       deckLabel = 0;
       cardLabel = 0;
       deckIconView = 0;
       cardIconView = 0;
       randomDeck = 0;
       randomCardDir = 0;
    }

    QLabel* deckLabel;
    QLabel* cardLabel;
    KIconView* deckIconView;
    KIconView* cardIconView;
    QCheckBox* randomDeck;
    QCheckBox* randomCardDir;
    QMap<QIconViewItem*, QString> deckMap;
    QMap<QIconViewItem*, QString> cardMap;
    QMap<QString, QString> helpMap;

    //set query variables
    KCardDialog::CardFlags cFlags;
    QString cDeck;
    QString cCardDir;
};

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

const QString& KCardDialog::deck() const { return d->cDeck; }
void KCardDialog::setDeck(const QString& file) { d->cDeck=file; }
const QString& KCardDialog::cardDir() const { return d->cCardDir; }
void KCardDialog::setCardDir(const QString& dir) { d->cCardDir=dir; }
KCardDialog::CardFlags KCardDialog::flags() const { return d->cFlags; }
bool KCardDialog::isRandomDeck() const
{
  if (d->randomDeck) return d->randomDeck->isChecked();
  else return false;
}
bool KCardDialog::isRandomCardDir() const
{
  if (d->randomCardDir) return d->randomCardDir->isChecked();
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

    d->deckIconView = new KIconView(grp1,"decks");
    d->deckIconView->setSpacing(8);
    /*
    deckIconView->setGridX(-1);
    deckIconView->setGridY(50);
    */
    d->deckIconView->setGridX(82);
    d->deckIconView->setGridY(106);
    d->deckIconView->setSelectionMode(QIconView::Single);
    d->deckIconView->setResizeMode(QIconView::Adjust);
    d->deckIconView->setMinimumWidth(360);
    d->deckIconView->setMinimumHeight(170);
    d->deckIconView->setWordWrapIconText(false);
    d->deckIconView->showToolTips();

    // deck select
    QVBoxLayout* l = new QVBoxLayout(layout);
    QGroupBox* grp3 = new QGroupBox(i18n("Backside"), plainPage());
    grp3->setFixedSize(100, 130);
    l->addWidget(grp3, 0, AlignTop|AlignHCenter);
    d->deckLabel = new QLabel(grp3);
    d->deckLabel->setText(i18n("empty"));
    d->deckLabel->setAlignment(AlignHCenter|AlignVCenter);
    d->deckLabel->setGeometry(10, 20, 80, 90);

    d->randomDeck = new QCheckBox(plainPage());
    d->randomDeck->setChecked(false);
    connect(d->randomDeck, SIGNAL(toggled(bool)), this,
            SLOT(slotRandomDeckToggled(bool)));
    d->randomDeck->setText(i18n("Random backside"));
    l->addWidget(d->randomDeck, 0, AlignTop|AlignHCenter);

    connect(d->deckIconView,SIGNAL(clicked(QIconViewItem *)),
            this,SLOT(slotDeckClicked(QIconViewItem *)));
  }

  if (! (flags() & NoCards))
  {
    // Cards iconview
    QHBoxLayout* layout = new QHBoxLayout(topLayout);
    QGroupBox* grp2 = new QGroupBox(1, Horizontal, i18n("Choose frontside"), plainPage());
    layout->addWidget(grp2);

    d->cardIconView =new KIconView(grp2,"cards");
    /*
    cardIconView->setGridX(36);
    cardIconView->setGridY(50);
    */
    d->cardIconView->setGridX(82);
    d->cardIconView->setGridY(106);
    d->cardIconView->setResizeMode(QIconView::Adjust);
    d->cardIconView->setMinimumWidth(360);
    d->cardIconView->setMinimumHeight(170);
    d->cardIconView->setWordWrapIconText(false);
    d->cardIconView->showToolTips();

    // Card select
    QVBoxLayout* l = new QVBoxLayout(layout);
    QGroupBox* grp4 = new QGroupBox(i18n("Frontside"), plainPage());
    grp4->setFixedSize(100, 130);
    l->addWidget(grp4, 0, AlignTop|AlignHCenter);
    d->cardLabel = new QLabel(grp4);
    d->cardLabel->setText(i18n("empty"));
    d->cardLabel->setAlignment(AlignHCenter|AlignVCenter);
    d->cardLabel->setGeometry(10, 20, 80, 90 );

    d->randomCardDir = new QCheckBox(plainPage());
    d->randomCardDir->setChecked(false);
    connect(d->randomCardDir, SIGNAL(toggled(bool)), this,
            SLOT(slotRandomCardDirToggled(bool)));
    d->randomCardDir->setText(i18n("Random frontside"));
    l->addWidget(d->randomCardDir, 0, AlignTop|AlignHCenter);

    connect(d->cardIconView,SIGNAL(clicked(QIconViewItem *)),
            this,SLOT(slotCardClicked(QIconViewItem *)));
  }

  // Insert deck icons
  // First find the default or alternate path
  if (! (flags() & NoDeck))
  {
      insertDeckIcons();
      d->deckIconView->arrangeItemsInGrid();

      // Set default icons if given
      if (!deck().isNull())
      {
          file=deck();
          QPixmap pixmap(file);
          pixmap=pixmap.xForm(m);
          d->deckLabel->setPixmap(pixmap);
          QToolTip::add(d->deckLabel,d->helpMap[file]);
      }
  }

  // Insert card icons
  if (! (flags() & NoCards))
  {
      insertCardIcons();
      d->cardIconView->arrangeItemsInGrid();

    // Set default icons if given
    if (!cardDir().isNull())
    {
        file = cardDir() + KCARD_DEFAULTCARD;
        QPixmap pixmap(file);
        pixmap = pixmap.xForm(m);
        d->cardLabel->setPixmap(pixmap);
        QToolTip::add(d->cardLabel,d->helpMap[cardDir()]);
    }
  }

}

void KCardDialog::insertCardIcons()
{
    QStringList list = KGlobal::dirs()->findAllResources("cards", "card*/index.desktop", false, true);
    // kdDebug(11001) << "insert " << list.count() << endl;
    if (list.isEmpty())
        return;

    // We shrink the icons a little
    // 
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

        QString name=cfg.readEntry("Name", i18n("unnamed"));
        QIconViewItem *item= new QIconViewItem(d->cardIconView, name, pixmap);

        item->setDragEnabled(false);
        item->setDropEnabled(false);
        item->setRenameEnabled(false);
        item->setSelectable(true);

        d->cardMap[item] = path;
        d->helpMap[path] = cfg.readEntry("Comment",name);
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

        // pixmap=pixmap.xForm(m);

        cfg.setGroup(QString::fromLatin1("KDE Cards"));
        QString name=cfg.readEntry("Name", i18n("unnamed"));
        QIconViewItem *item= new QIconViewItem(d->deckIconView,name, pixmap);

        item->setDragEnabled(false);
        item->setDropEnabled(false);
        item->setRenameEnabled(false);

        d->deckMap[item] = getDeckName(*it);
        d->helpMap[d->deckMap[item]] = cfg.readEntry("Comment",name);
    }
}


KCardDialog::~KCardDialog()
{
 delete d;
}


// Create the dialog
KCardDialog::KCardDialog( QWidget *parent, const char *name, CardFlags mFlags)
    : KDialogBase( Plain, i18n("Carddeck selection"), Ok|Cancel, Ok, parent, name, true, true)
{
    KCardDialog::init();

    d = new KCardDialogPrivate;
    d->cFlags = mFlags;
}

void KCardDialog::slotDeckClicked(QIconViewItem *item)
{
    if (item && item->pixmap())
    {
        d->deckLabel->setPixmap(* (item->pixmap()));
        QToolTip::add(d->deckLabel,d->helpMap[d->deckMap[item]]);
        setDeck(d->deckMap[item]);
    }
}
void KCardDialog::slotCardClicked(QIconViewItem *item)
{
    if (item && item->pixmap())
    {
        d->cardLabel->setPixmap(* (item->pixmap()));
        QString path = d->cardMap[item];
        QToolTip::add(d->cardLabel,d->helpMap[path]);
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
    if (!d->randomDeck)
	return;

    if (s)
        d->randomDeck->show();
    else
        d->randomDeck->hide();
}

void KCardDialog::showRandomCardDirBox(bool s)
{
    if (!d->randomCardDir)
	return;

    if (s)
        d->randomCardDir->show();
    else
        d->randomCardDir->hide();
}

void KCardDialog::slotRandomDeckToggled(bool on)
{
  if (on) {
    d->deckLabel->setText("random");
    setDeck(getRandomDeck());
  } else {
    d->deckLabel->setText("empty");
    setDeck(0);
  }
}

void KCardDialog::slotRandomCardDirToggled(bool on)
{
  if (on) {
      d->cardLabel->setText("random");
      setCardDir(getRandomCardDir());
      if (cardDir().length()>0 && cardDir().right(1)!=QString::fromLatin1("/"))  {
          setCardDir(cardDir() + QString::fromLatin1("/"));
      }
  } else {
      d->cardLabel->setText("empty");
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
