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

#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qdir.h>
#include <stdio.h>
#include <klocale.h>
#include <kstddirs.h>
#include "kcarddialog.h"

int KCardDialog::getCardDeck(QString &mDeck,QString &mCarddir,QWidget *mParent,int mFlags,
                       QString mAlternateDeck,QString mAlternateCarddir,QString mMask)
{


  KCardDialog dlg(mParent);
  int result;

  // Default filemaks is a PNG icon
  if (mMask.isNull()) mMask=QString::fromLatin1(".png");
  // Remove mask from deck if given
  if (!mDeck.isNull() && !mMask.isNull())
  {
    if (mDeck.right(mMask.length())==mMask) mDeck=mDeck.left(mDeck.length()-mMask.length());
  }

  dlg.setDeck(mDeck);
  dlg.setCardDir(mCarddir);
  dlg.setFlags(mFlags);
  dlg.setFilemask(mMask);
  dlg.setAlternateDeck(mAlternateDeck);
  dlg.setAlternateCardDir(mAlternateCarddir);
  
  dlg.setupDialog();
  result=dlg.exec();
  if (result==QDialog::Accepted)
  {
    mDeck=dlg.deck();
    if (!mDeck.isNull())
    {
      mDeck+=dlg.filemask();
    }
    mCarddir=dlg.cardDir();
    if (!mCarddir.isNull() && mCarddir.right(1)!=QString::fromLatin1("/"))
    {
      mCarddir+=QString::fromLatin1("/");
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

QString KCardDialog::deck() { return cDeck; }
void KCardDialog::setDeck(QString file) { cDeck=file; }
QString KCardDialog::cardDir() { return cCardDir; }
void KCardDialog::setCardDir(QString dir) { cCardDir=dir; }
int KCardDialog::flags() { return cFlags; }
void KCardDialog::setFlags(int f) { cFlags=f; }
QString KCardDialog::filemask() { return cMask; }
void KCardDialog::setFilemask(QString mask) { cMask=mask; }
QString KCardDialog::alternateDeck() { return cAltDeck; }
void KCardDialog::setAlternateDeck(QString file) { cAltDeck=file; }
QString KCardDialog::alternateCardDir() { return cAltCarddir; }
void KCardDialog::setAlternateCardDir(QString dir) { cAltCarddir=dir; }

// protected
QString KCardDialog::deckPath() { return cDeckpath; }
void KCardDialog::setDeckPath(QString path) { cDeckpath=path; }
QString KCardDialog::cardPath() { return cCardpath; }
void KCardDialog::setCardPath(QString path) { cCardpath=path; }

void KCardDialog::setupDialog()
{
  QString file,path;
  int y_pos,y_pos2;
  QWMatrix m;
  m.scale(0.8,0.8);

  setCaption(i18n("Carddeck selection"));
  setMinimumSize(200,160);
  setMaximumSize(800,600);                                                        

  if (flags() & (NoDeck|NoCards))
  {
    resize(550,240);
    y_pos=200;
    y_pos2=0;
  }
  else
  {
    resize( 550, 480 );
    y_pos=440;
    y_pos2=220;
  }

  if (! (flags() & NoDeck))
  {
    // Deck iconview
    QGroupBox* grp1;
    grp1 = new QGroupBox(i18n("Choose backside"), this);
    grp1->resize(380,200);
    grp1->move(10,10);

    deckIconView =new KIconView(grp1,"decks");
    deckIconView->setGeometry( 10, 20, 360, 170 );
    deckIconView->setGridX(36);
    deckIconView->setGridY(50);
    deckIconView->setSelectionMode(QIconView::Single);

    // deck select
    QGroupBox* grp3;
    grp3 = new QGroupBox(i18n("Back"), this);
    grp3->resize(100,130);
    grp3->move(400,10);

    deckLabel = new QLabel(grp3);
    deckLabel->setGeometry(10, 20, 80, 90 );
    deckLabel->setText(i18n("empty"));
    deckLabel->setAlignment(AlignHCenter|AlignVCenter);

    connect(deckIconView,SIGNAL(clicked(QIconViewItem *)),
            this,SLOT(slotDeckClicked(QIconViewItem *)));
  }

  if (! (flags() & NoCards))
  {
    // Cards iconview
    QGroupBox* grp2;
    grp2 = new QGroupBox(i18n("Choose frontside"), this);
    grp2->resize(380,200);
    grp2->move(10,10+y_pos2);

    cardIconView =new KIconView(grp2,"cards");
    cardIconView->setGeometry( 10, 20, 360, 170 );
    cardIconView->setGridX(36);
    cardIconView->setGridY(50);

    // Card select
    QGroupBox* grp4;
    grp4 = new QGroupBox(i18n("Front"), this);
    grp4->resize(100,130);
    grp4->move(400,10+y_pos2);

    cardLabel = new QLabel(grp4);
    cardLabel->setGeometry(10, 20, 80, 90 );
    cardLabel->setText(i18n("empty"));
    cardLabel->setAlignment(AlignHCenter|AlignVCenter);

    connect(cardIconView,SIGNAL(clicked(QIconViewItem *)),
            this,SLOT(slotCardClicked(QIconViewItem *)));
  }



  // Buttons
  QPushButton *PushButton;
  PushButton = new QPushButton( this, "Button_1" );
  PushButton->setGeometry( 390, y_pos, 65, 30 );
  connect( PushButton, SIGNAL(clicked()), SLOT(accept()) );
  PushButton->setText( i18n("OK" ));
  PushButton->setAutoRepeat( FALSE );
  PushButton->setAutoResize( FALSE );

  PushButton = new QPushButton( this, "Button_2" );
  PushButton->setGeometry( 470, y_pos, 65, 30 );
  connect( PushButton, SIGNAL(clicked()), SLOT(reject()) );
  PushButton->setText( i18n("Cancel" ));
  PushButton->setAutoRepeat( FALSE );
  PushButton->setAutoResize( FALSE );


  // Insert Deckicons 
  // First find the default or alternate path
 
  if (! (flags() & NoDeck))
  {
    file=KCARD_DECKPATH+KCARD_DEFAULTDECK+filemask();
    path=kapp->dirs()->findResourceDir("data",file);
    if (!path.isNull())
    {
      path+=KCARD_DECKPATH;
    }


    // Set default pat if no alternate deck given or probe default deck given
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
      file=deck()+filemask();
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
    // Set default pat if no alternate deck given or probe default deck given
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
KCardDialog::KCardDialog( QWidget *parent, const char *name,bool modal , WFlags f )
    : KDialog( parent, name,modal,f )
{
  pixmapList.setAutoDelete(TRUE);
}

void KCardDialog::slotDeckClicked(QIconViewItem *item)
{
    if (item && item->pixmap()) 
    {
      deckLabel->setPixmap(* (item->pixmap()));
      setDeck(deckPath()+item->text());
    }
}
void KCardDialog::slotCardClicked(QIconViewItem *item)
{
    if (item && item->pixmap()) 
    {
      cardLabel->setPixmap(* (item->pixmap()));
      setCardDir(cardPath()+item->text());
    }

}

#include "kcarddialog.moc"
