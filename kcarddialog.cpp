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

/* TODO:
 *  - Check for tailing "/" in getDefaultXXX
 *  - check whether alternate dirs exists!
 *
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

#include "kcarddialog.h"

int KCardDialog::getCardDeck(QString &mDeck,QString &mCarddir,QWidget *mParent,
                  int mFlags, QString mAlternateDeck,QString mAlternateCarddir,
                  QString mMask, bool* mRandomDeck, bool* mRandomCardDir)
{
  KCardDialog dlg(mParent);
  int result;
  // Default filemaks is a PNG icon
  if (mMask.isNull()) mMask=QString::fromLatin1(".png");

  // Remove mask from deck if given // WHY?? removed - AB
//  if (!mDeck.isNull() && !mMask.isNull())
//  {
//    if (mDeck.right(mMask.length())==mMask) mDeck=mDeck.left(mDeck.length()-mMask.length());
//  }

  dlg.setDeck(mDeck);
  dlg.setCardDir(mCarddir);
  dlg.setFlags(mFlags);
  dlg.setFilemask(mMask);
  dlg.setAlternateDeck(mAlternateDeck);
  dlg.setAlternateCardDir(mAlternateCarddir);
  
  dlg.setupDialog();
  dlg.showRandomDeckBox(mRandomDeck ? true : false);
  dlg.showRandomCardDirBox(mRandomCardDir ? true : false);
  result=dlg.exec();
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

QString KCardDialog::getDefaultDeckPath(int mFlags,QString mAlternateDeck)
{
    QString file=KCARD_DECKPATH+KCARD_DEFAULTDECK+KCARD_DEFAULTFILEMASK;
    QString path=kapp->dirs()->findResourceDir("data",file);
    if (!path.isNull())
    {
      path+=KCARD_DECKPATH;
    }

    if (mAlternateDeck.isNull() || (!path.isNull() && (mFlags&ProbeDefaultDir)))
    {
      return path;
    }
    else
    {
      return mAlternateDeck;
    }
}

QString KCardDialog::getDefaultCardPath(int mFlags,QString mAlternateCarddir)
{
    QString file=KCARD_CARDPATH+KCARD_DEFAULTCARDDIR+KCARD_DEFAULTCARD+KCARD_DEFAULTFILEMASK;
    QString path=kapp->dirs()->findResourceDir("data",file);
    if (!path.isNull())
    {
      path+=KCARD_CARDPATH+KCARD_DEFAULTCARDDIR;
    }

    if (mAlternateCarddir.isNull() || (!path.isNull() && (mFlags&ProbeDefaultDir)))
    {
      return path;
    }
    else
    {
      return mAlternateCarddir;
    }
}

QString KCardDialog::deck() const { return cDeck; }
void KCardDialog::setDeck(QString file) { cDeck=file; }
QString KCardDialog::cardDir() const { return cCardDir; }
void KCardDialog::setCardDir(QString dir) { cCardDir=dir; }
int KCardDialog::flags() const { return cFlags; }
void KCardDialog::setFlags(int f) { cFlags=f; }
QString KCardDialog::filemask() const { return cMask; }
void KCardDialog::setFilemask(QString mask) { cMask=mask; }
QString KCardDialog::alternateDeck() const { return cAltDeck; }
void KCardDialog::setAlternateDeck(QString file) { cAltDeck=file; }
QString KCardDialog::alternateCardDir() const { return cAltCarddir; }
void KCardDialog::setAlternateCardDir(QString dir) { cAltCarddir=dir; }
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

// protected
QString KCardDialog::deckPath() const { return cDeckpath; }
void KCardDialog::setDeckPath(QString path) { cDeckpath=path; }
QString KCardDialog::cardPath() const { return cCardpath; }
void KCardDialog::setCardPath(QString path) { cCardpath=path; }

void KCardDialog::setupDialog()
{
  QVBoxLayout* topLayout = new QVBoxLayout(plainPage(), spacingHint());
  QString path, file;
  QWMatrix m;
  m.scale(0.8,0.8);

  if (! (flags() & NoDeck))
  {
    QHBoxLayout* layout = new QHBoxLayout(topLayout);

    // Deck iconview
    QGroupBox* grp1 = new QGroupBox(1, Horizontal, i18n("Choose backside"), plainPage());
    layout->addWidget(grp1);

    deckIconView = new KIconView(grp1,"decks");
    deckIconView->setGridX(36);
    deckIconView->setGridY(50);
    deckIconView->setSelectionMode(QIconView::Single);
    deckIconView->setResizeMode(QIconView::Adjust);
    deckIconView->setMinimumWidth(360);
    deckIconView->setMaximumHeight(170);

    // deck select
    QVBoxLayout* l = new QVBoxLayout(layout);
    QGroupBox* grp3 = new QGroupBox(i18n("Back"), plainPage());
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
    cardIconView->setGridX(36);
    cardIconView->setGridY(50);
    cardIconView->setResizeMode(QIconView::Adjust);
    cardIconView->setMinimumWidth(360);
    cardIconView->setMaximumHeight(170);

    // Card select
    QVBoxLayout* l = new QVBoxLayout(layout);
    QGroupBox* grp4 = new QGroupBox(i18n("Front"), plainPage());
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
    file=KCARD_DECKPATH+KCARD_DEFAULTDECK+filemask();
    path=kapp->dirs()->findResourceDir("data",file);
    if (!path.isNull())
    {
      path+=KCARD_DECKPATH;
    }

    // Set default path if no alternate deck given or probe default deck given
    // and default deck found
    if (alternateDeck().isNull() || (!path.isNull() && (flags()&ProbeDefaultDir)))
    {
      setDeckPath(path);
    }
    else
    {
      setDeckPath(alternateDeck());
    }


    insertDeckIcons(deckPath());
    deckIconView->arrangeItemsInGrid();
 
    // Set default icons if given
    if (!deck().isNull())
    {
      file=deck();
      QPixmap *pixmap=new QPixmap(file);
      QPixmap *pixmap2=new QPixmap;
      *pixmap2=pixmap->xForm(m);
      delete pixmap;
      deckLabel->setPixmap(*pixmap2);
      pixmapList.append(pixmap2);
    }
  }

  // Insert cardicons
  if (! (flags() & NoCards))
  {
    file=KCARD_CARDPATH+KCARD_DEFAULTCARDDIR+KCARD_DEFAULTCARD+filemask();
    path=kapp->dirs()->findResourceDir("data",file);
    if (!path.isNull())
    {
      path+=KCARD_CARDPATH;
    }
    // Set default path if no alternate deck given or probe default deck given
    // and default deck found
    if (alternateCardDir().isNull() || (!path.isNull() && (flags()&ProbeDefaultDir)))
    {
      setCardPath(path);
    }
    else
    {
      setCardPath(alternateCardDir());
    }

    insertCardIcons(cardPath());
    cardIconView->arrangeItemsInGrid();

    // Set default icons if given
    if (!cardDir().isNull())
    {
      file=cardDir()+KCARD_DEFAULTCARD+filemask();
      QPixmap *pixmap=new QPixmap(file);
      QPixmap *pixmap2=new QPixmap;
      *pixmap2=pixmap->xForm(m);
      delete pixmap;
      cardLabel->setPixmap(*pixmap2);
      pixmapList.append(pixmap2);
    }
  }

}

void KCardDialog::insertCardIcons(QString path)
{
  QDir dir;
  QString label;
  unsigned int i;
  // We shrink the icons a little
  QWMatrix m;
  m.scale(0.8,0.8);

  dir.cd(path);
  dir.setSorting(QDir::Name);
  dir.setNameFilter("card*");
  
  for (i=0;i<dir.count();i++)
  {
    QString card=QString::fromLatin1("/11")+filemask();
    label=dir[i]+card;

  
    QPixmap *pixmap=new QPixmap(path+label);
    if (pixmap->isNull())
    {
      delete pixmap;
      continue;
    }
   
    QPixmap *pixmap2=new QPixmap;
    *pixmap2=pixmap->xForm(m);
    delete pixmap;

    pixmapList.append(pixmap2);
  
    QIconViewItem *item= new QIconViewItem(cardIconView,dir[i],*pixmap2);
    item->setDragEnabled(false);
    item->setDropEnabled(false);
    item->setRenameEnabled(false);
    item->setSelectable(true);
    
  }
}
  
void KCardDialog::insertDeckIcons(QString path)
{
  QDir dir;
  QString label;
  unsigned int i;
  
  // We shrink the icons a little
  QWMatrix m;
  m.scale(0.8,0.8);

  // Read in directory sorted by name
  dir.cd(path);
  dir.setSorting(QDir::Name);
  dir.setNameFilter(QString::fromLatin1("*")+filemask());
  
  for (i=0;i<dir.count();i++)
  {
    label=dir[i];
    QPixmap *pixmap=new QPixmap(path+label);
    if (pixmap->isNull())
    {
      delete pixmap;
      continue;
    }
    // Transform bitmap (shrink)
    QPixmap *pixmap2=new QPixmap;
    *pixmap2=pixmap->xForm(m);
    delete pixmap;

    // To get rid of the pixmaps later on
    pixmapList.append(pixmap2);
  
    QIconViewItem *item= new QIconViewItem(deckIconView,label.left(label.length()-filemask().length()),
                                           *pixmap2);
    item->setDragEnabled(false);
    item->setDropEnabled(false);
    item->setRenameEnabled(false);
  }
}


KCardDialog::~KCardDialog()
{
}


// Create the dialog 
KCardDialog::KCardDialog( QWidget *parent, const char *name,bool modal)
    : KDialogBase( Plain, i18n("Carddeck selection"), Ok|Cancel, Ok, parent, name, modal, true)
{
  randomDeck = 0;
  randomCardDir = 0;
  pixmapList.setAutoDelete(TRUE);
}

void KCardDialog::slotDeckClicked(QIconViewItem *item)
{
    if (item && item->pixmap()) 
    {
      deckLabel->setPixmap(* (item->pixmap()));
      setDeck(deckPath()+item->text()+filemask());
    }
}
void KCardDialog::slotCardClicked(QIconViewItem *item)
{
    if (item && item->pixmap()) 
    {
      cardLabel->setPixmap(* (item->pixmap()));
      setCardDir(cardPath()+item->text());
      if (cardDir().length()>0 && cardDir().right(1)!=QString::fromLatin1("/")) 
      {
        setCardDir(cardDir() + QString::fromLatin1("/"));
      }
    }
}

QString KCardDialog::getRandomDeck(int mFlags, QString alternateDeck, QString mMask)
{
  if (mMask.isNull()) {
    mMask=QString::fromLatin1(".png");
  }

  QString file=KCARD_DECKPATH+KCARD_DEFAULTDECK+mMask;
  QString path=kapp->dirs()->findResourceDir("data", file);

  if (!path.isNull()) {
    path+=KCARD_DECKPATH;
  }
  if (!alternateDeck.isNull() && !(!path.isNull() && mFlags&ProbeDefaultDir)) {
    path=alternateDeck;
  }

  QDir dir;
  dir.cd(path);
  dir.setSorting(QDir::Name);
  dir.setNameFilter(QString::fromLatin1("*")+mMask);
  int d = kapp->random() % dir.count();
  return dir.absPath() + "/" + dir[d];
}

QString KCardDialog::getRandomCardDir(int mFlags, QString alternateCardDir, QString mMask)
{
  if (mMask.isNull()) {
    mMask=QString::fromLatin1(".png");
  }

  QString file=KCARD_CARDPATH+KCARD_DEFAULTCARDDIR+KCARD_DEFAULTCARD+mMask;
  QString path=kapp->dirs()->findResourceDir("data", file);

  if (!path.isNull()) {
    path+=KCARD_CARDPATH;
  }
  if (!alternateCardDir.isNull() && !(!path.isNull() && mFlags&ProbeDefaultDir)) {//uff what ugly...
    path=alternateCardDir;
  }

  QDir dir;
  dir.cd(path);
  dir.setSorting(QDir::Name);
  dir.setNameFilter("card*");
  int d = kapp->random() % dir.count();
  return dir.absPath() + "/" + dir[d];
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
    setDeck(getRandomDeck(0, deckPath()));
  } else {
    deckLabel->setText("empty");
    setDeck(0);
  }
}

void KCardDialog::slotRandomCardDirToggled(bool on)
{
  if (on) {
    cardLabel->setText("random");
    setCardDir(getRandomCardDir(0, cardPath()));
    if (cardDir().length()>0 && cardDir().right(1)!=QString::fromLatin1("/"))  {
      setCardDir(cardDir() + QString::fromLatin1("/"));
    }
  } else {
    cardLabel->setText("empty");
    setCardDir(0);
  }
}

#include "kcarddialog.moc"
