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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <stdio.h>
#include <assert.h>

#include <q3groupbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qslider.h>
#include <qmatrix.h>
//Added by qt3to4:
#include <QPixmap>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <kapplication.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <k3iconview.h>
#include <krandom.h>
#include <ksimpleconfig.h>

#include "kcarddialog.h"
#include <qpushbutton.h>
#include <kdebug.h>
#include <krandom.h>

#define KCARD_DEFAULTDECK QString::fromLatin1("deck0.png")
#define KCARD_DEFAULTCARD QString::fromLatin1("11.png")
#define KCARD_DEFAULTCARDDIR QString::fromLatin1("cards-default/")

// values for the resize slider
#define SLIDER_MIN 400
#define SLIDER_MAX 3000

// KConfig entries
#define CONF_GROUP "KCardDialog"
#define CONF_RANDOMDECK QString::fromLatin1("RandomDeck")
#define CONF_DECK "Deck"
#define CONF_CARDDIR QString::fromLatin1("CardDir")
#define CONF_RANDOMCARDDIR QString::fromLatin1("RandomCardDir")
#define CONF_USEGLOBALDECK QString::fromLatin1("GlobalDeck")
#define CONF_USEGLOBALCARDDIR QString::fromLatin1("GlobalCardDir")
#define CONF_SCALE QString::fromLatin1("Scale")

#define CONF_GLOBAL_GROUP QString::fromLatin1("KCardDialog Settings")
#define CONF_GLOBAL_DECK "GlobalDeck"
#define CONF_GLOBAL_CARDDIR QString::fromLatin1("GlobalCardDir")
#define CONF_GLOBAL_RANDOMDECK QString::fromLatin1("GlobalRandomDeck")
#define CONF_GLOBAL_RANDOMCARDDIR QString::fromLatin1("GlobalRandomCardDir")


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
       cPreview = 0;
       scaleSlider = 0;
       globalDeck = 0;
       globalCardDir = 0;

       cScale = 1;
    }

    QLabel* deckLabel;
    QLabel* cardLabel;
    K3IconView* deckIconView;
    K3IconView* cardIconView;
    QCheckBox* randomDeck;
    QCheckBox* randomCardDir;
    QCheckBox* globalDeck;
    QCheckBox* globalCardDir;

    QSlider* scaleSlider;
    QPixmap cPreviewPix;
    QLabel* cPreview;

    QMap<Q3IconViewItem*, QString> deckMap;
    QMap<Q3IconViewItem*, QString> cardMap;
    QMap<QString, QString> helpMap;

    //set query variables
    KCardDialog::CardFlags cFlags;
    QString cDeck;
    QString cCardDir;
    double cScale;
};

int KCardDialog::getCardDeck(QString &pDeck, QString &pCardDir, QWidget *pParent,
                             CardFlags pFlags, bool* pRandomDeck, bool* pRandomCardDir,
			     double* pScale, KConfig* pConf)
{
    KCardDialog dlg(pParent, "dlg", pFlags);

    dlg.setDeck(pDeck);
    dlg.setCardDir(pCardDir);

    dlg.setupDialog(pScale != 0);
    dlg.loadConfig(pConf);
    dlg.showRandomDeckBox(pRandomDeck != 0);
    dlg.showRandomCardDirBox(pRandomCardDir != 0);
    int result=dlg.exec();
    if (result==QDialog::Accepted)
    {
    // TODO check for global cards/decks!!!!
        pDeck=dlg.deck();
        pCardDir=dlg.cardDir();
        if (!pCardDir.isNull() && pCardDir.right(1)!=QString::fromLatin1("/"))
        {
            pCardDir+=QString::fromLatin1("/");
        }
        if (pRandomDeck)
        {
            *pRandomDeck = dlg.isRandomDeck();
        }
        if (pRandomCardDir)
        {
            *pRandomCardDir = dlg.isRandomCardDir();
        }
	if (pScale)
	{
            *pScale = dlg.cardScale();
	}

        if (dlg.isGlobalDeck())
	{
	    kDebug(11000) << "use global deck" << endl;
	    bool random;
	    getGlobalDeck(pDeck, random);
	    kDebug(11000) << "use: " << pDeck<< endl;
	    if (pRandomDeck)
	    {
	        *pRandomDeck=random;
		if (random)
	        kDebug(11000) << "use random deck" << endl;
	    }
	}
        if (dlg.isGlobalCardDir())
	{
	    kDebug(11000) << "use global carddir" << endl;
	    bool random;
	    getGlobalCardDir(pCardDir, random);
	    kDebug(11000) << "use: " << pCardDir << endl;
	    if (pRandomCardDir)
	    {
	        *pRandomCardDir=random;
		if (random)
	        kDebug(11000) << "use random carddir" << endl;
	    }
	}
    }
    dlg.saveConfig(pConf);
    return result;
}

void KCardDialog::getConfigCardDeck(KConfig* conf, QString &pDeck, QString &pCardDir, double& pScale)
{
// TODO check for global cards/decks!!!!
 if (!conf) {
	return;
 }
 QString origGroup = conf->group();

 conf->setGroup(CONF_GROUP);
 if (conf->readEntry(CONF_RANDOMDECK,false) || !conf->hasKey(CONF_DECK)) {
	pDeck = getRandomDeck();
 } else {
	pDeck = conf->readEntry(CONF_DECK);
 }
 if (conf->readEntry(CONF_RANDOMCARDDIR,false) || !conf->hasKey(CONF_CARDDIR)) {
	pCardDir = getRandomCardDir();
 } else {
	pCardDir = conf->readPathEntry(CONF_CARDDIR);
 }
 pScale = conf->readEntry(CONF_SCALE, 1.0);

 if (conf->readEntry(CONF_USEGLOBALDECK, false)) {
	bool random;
	getGlobalDeck(pCardDir, random);
	if (random || pDeck.isNull() ) {
		pDeck = getRandomDeck();
	}
 }
 if (conf->readEntry(CONF_USEGLOBALCARDDIR, false)) {
	bool random;
	getGlobalCardDir(pCardDir, random);
	if (random || pCardDir.isNull() ) {
		pCardDir = getRandomCardDir();
	}
 }

 conf->setGroup(origGroup);
}

QString KCardDialog::getDefaultDeck()
{
    KCardDialog::init();
    return QLatin1String("decks/") + KCARD_DEFAULTDECK;
}

QString KCardDialog::getDefaultCardDir()
{
    KCardDialog::init();

    return KCARD_DEFAULTCARDDIR;
}

QString KCardDialog::getCardPath(const QString &carddir, int index)
{
    KCardDialog::init();

    QString entry = carddir + QString::number(index) + QLatin1String(".png");
    return locate( "cards", entry );
}

const QString& KCardDialog::deck() const { return d->cDeck; }
void KCardDialog::setDeck(const QString& file) { d->cDeck=file; }
const QString& KCardDialog::cardDir() const { return d->cCardDir; }
void KCardDialog::setCardDir(const QString& dir) { d->cCardDir=dir; }
KCardDialog::CardFlags KCardDialog::flags() const { return d->cFlags; }
double KCardDialog::cardScale() const { return d->cScale; }
bool KCardDialog::isRandomDeck() const
{ return (d->randomDeck ? d->randomDeck->isChecked() : false); }
bool KCardDialog::isRandomCardDir() const
{ return (d->randomCardDir ? d->randomCardDir->isChecked() : false); }
bool KCardDialog::isGlobalDeck() const
{ return (d->globalDeck ? d->globalDeck->isChecked() : false); }
bool KCardDialog::isGlobalCardDir() const
{ return (d->globalCardDir ? d->globalCardDir->isChecked() : false); }

void KCardDialog::setupDialog(bool showResizeBox)
{
  QHBoxLayout* topLayout = new QHBoxLayout(plainPage(), spacingHint());
  QVBoxLayout* cardLayout = new QVBoxLayout(topLayout);
  QString path, file;
  QMatrix m;
  m.scale(0.8,0.8);

  setInitialSize(QSize(600,400));

  if (! (flags() & NoDeck))
  {
    QHBoxLayout* layout = new QHBoxLayout(cardLayout);

    // Deck iconview
    Q3GroupBox* grp1 = new Q3GroupBox(1, Qt::Horizontal, i18n("Choose Backside"), plainPage());
    layout->addWidget(grp1);

    d->deckIconView = new K3IconView(grp1,"decks");
    d->deckIconView->setSpacing(8);
    /*
    deckIconView->setGridX(-1);
    deckIconView->setGridY(50);
    */
    d->deckIconView->setGridX(82);
    d->deckIconView->setGridY(106);
    d->deckIconView->setSelectionMode(Q3IconView::Single);
    d->deckIconView->setResizeMode(Q3IconView::Adjust);
    d->deckIconView->setMinimumWidth(360);
    d->deckIconView->setMinimumHeight(170);
    d->deckIconView->setWordWrapIconText(false);
    d->deckIconView->showToolTips();

    // deck select
    QVBoxLayout* l = new QVBoxLayout(layout);
    Q3GroupBox* grp3 = new Q3GroupBox(i18n("Backside"), plainPage());
    grp3->setFixedSize(100, 130);
    l->addWidget(grp3, 0, Qt::AlignTop|Qt::AlignHCenter);
    d->deckLabel = new QLabel(grp3);
    d->deckLabel->setText(i18n("empty"));
    d->deckLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    d->deckLabel->setGeometry(10, 20, 80, 90);

    d->randomDeck = new QCheckBox(plainPage());
    d->randomDeck->setChecked(false);
    connect(d->randomDeck, SIGNAL(toggled(bool)), this,
            SLOT(slotRandomDeckToggled(bool)));
    d->randomDeck->setText(i18n("Random backside"));
    l->addWidget(d->randomDeck, 0, Qt::AlignTop|Qt::AlignHCenter);

    d->globalDeck = new QCheckBox(plainPage());
    d->globalDeck->setChecked(false);
    d->globalDeck->setText(i18n("Use global backside"));
    l->addWidget(d->globalDeck, 0, Qt::AlignTop|Qt::AlignHCenter);

    QPushButton* b = new QPushButton(i18n("Make Backside Global"), plainPage());
    connect(b, SIGNAL(pressed()), this, SLOT(slotSetGlobalDeck()));
    l->addWidget(b, 0, Qt::AlignTop|Qt::AlignHCenter);

    connect(d->deckIconView,SIGNAL(clicked(Q3IconViewItem *)),
            this,SLOT(slotDeckClicked(Q3IconViewItem *)));
  }

  if (! (flags() & NoCards))
  {
    // Cards iconview
    QHBoxLayout* layout = new QHBoxLayout(cardLayout);
    Q3GroupBox* grp2 = new Q3GroupBox(1, Qt::Horizontal, i18n("Choose Frontside"), plainPage());
    layout->addWidget(grp2);

    d->cardIconView =new K3IconView(grp2,"cards");
    /*
    cardIconView->setGridX(36);
    cardIconView->setGridY(50);
    */
    d->cardIconView->setGridX(82);
    d->cardIconView->setGridY(106);
    d->cardIconView->setResizeMode(Q3IconView::Adjust);
    d->cardIconView->setMinimumWidth(360);
    d->cardIconView->setMinimumHeight(170);
    d->cardIconView->setWordWrapIconText(false);
    d->cardIconView->showToolTips();

    // Card select
    QVBoxLayout* l = new QVBoxLayout(layout);
    Q3GroupBox* grp4 = new Q3GroupBox(i18n("Frontside"), plainPage());
    grp4->setFixedSize(100, 130);
    l->addWidget(grp4, 0, Qt::AlignTop|Qt::AlignHCenter);
    d->cardLabel = new QLabel(grp4);
    d->cardLabel->setText(i18n("empty"));
    d->cardLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    d->cardLabel->setGeometry(10, 20, 80, 90 );

    d->randomCardDir = new QCheckBox(plainPage());
    d->randomCardDir->setChecked(false);
    connect(d->randomCardDir, SIGNAL(toggled(bool)), this,
            SLOT(slotRandomCardDirToggled(bool)));
    d->randomCardDir->setText(i18n("Random frontside"));
    l->addWidget(d->randomCardDir, 0, Qt::AlignTop|Qt::AlignHCenter);

    d->globalCardDir = new QCheckBox(plainPage());
    d->globalCardDir->setChecked(false);
    d->globalCardDir->setText(i18n("Use global frontside"));
    l->addWidget(d->globalCardDir, 0, Qt::AlignTop|Qt::AlignHCenter);

    QPushButton* b = new QPushButton(i18n("Make Frontside Global"), plainPage());
    connect(b, SIGNAL(pressed()), this, SLOT(slotSetGlobalCardDir()));
    l->addWidget(b, 0, Qt::AlignTop|Qt::AlignHCenter);

    connect(d->cardIconView,SIGNAL(clicked(Q3IconViewItem *)),
            this,SLOT(slotCardClicked(Q3IconViewItem *)));
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
          pixmap=pixmap.transformed(m);
          d->deckLabel->setPixmap(pixmap);
          d->deckLabel->setToolTip(d->helpMap[file]);
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
        pixmap = pixmap.transformed(m);
        d->cardLabel->setPixmap(pixmap);
        d->cardLabel->setToolTip(d->helpMap[cardDir()]);
    }
  }

  // insert resize box
  if (showResizeBox)
  {
    // this part is a little bit...tricky.
    // i'm sure there is a cleaner way but i cannot find it.
    // whenever the pixmap is resized (aka scaled) the box is resized, too. This
    // leads to an always resizing dialog which is *very* ugly. i worked around
    // this by using a QWidget which is the only child widget of the group box.
    // The other widget are managed inside this QWidget - a stretch area on the
    // right ensures that the KIconViews are not resized...

    // note that the dialog is still resized if you you scale the pixmap very
    // large. This is desired behaviour as i don't want to make the box even
    // larger but i want the complete pixmap to be displayed. the dialog is not
    // resized if you make the pixmap smaller again.
    QVBoxLayout* layout = new QVBoxLayout(topLayout);
    Q3GroupBox* grp = new Q3GroupBox(1, Qt::Horizontal, i18n("Resize Cards"), plainPage());
    layout->setResizeMode(QLayout::Fixed);
    layout->addWidget(grp);
    QWidget* box = new QWidget(grp);
    QHBoxLayout* hbox = new QHBoxLayout(box, 0, spacingHint());
    QVBoxLayout* boxLayout = new QVBoxLayout(hbox);
    hbox->addStretch(0);

    d->scaleSlider = new QSlider(1, SLIDER_MAX, 1, (-1000+SLIDER_MIN+SLIDER_MAX), Qt::Horizontal, box);
    d->scaleSlider->setMinValue(SLIDER_MIN);
    connect(d->scaleSlider, SIGNAL(valueChanged(int)), this, SLOT(slotCardResized(int)));
    boxLayout->addWidget(d->scaleSlider, 0, Qt::AlignLeft);

    QPushButton* b = new QPushButton(i18n("Default Size"), box);
    connect(b, SIGNAL(pressed()), this, SLOT(slotDefaultSize()));
    boxLayout->addWidget(b, 0, Qt::AlignLeft);

    QLabel* l = new QLabel(i18n("Preview:"), box);
    boxLayout->addWidget(l);
    d->cPreviewPix.load(getDefaultDeck());
    d->cPreview = new QLabel(box);
    boxLayout->addWidget(d->cPreview, 0, Qt::AlignCenter|Qt::AlignVCenter);

    slotCardResized(d->scaleSlider->value());
  }
}

void KCardDialog::insertCardIcons()
{
    QStringList list = KGlobal::dirs()->findAllResources("cards", "card*/index.desktop", false, true);
    // kDebug(11000) << "insert " << list.count() << endl;
    if (list.isEmpty())
        return;

    // We shrink the icons a little
    //
    QMatrix m;
    m.scale(0.8,0.8);

    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        KSimpleConfig cfg(*it);
        cfg.setGroup(QString::fromLatin1("KDE Backdeck"));
        QString path = (*it).left((*it).lastIndexOf('/') + 1);
        assert(path[path.length() - 1] == '/');
        QPixmap pixmap(path + cfg.readEntry("Preview", "12c.png"));

        if (pixmap.isNull())
            continue;

        QString name=cfg.readEntry("Name", i18n("unnamed"));
        Q3IconViewItem *item= new Q3IconViewItem(d->cardIconView, name, pixmap);

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
    QMatrix m;
    m.scale(0.8,0.8);

    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        KSimpleConfig cfg(*it);
        QPixmap pixmap(getDeckName(*it));
        if (pixmap.isNull())
            continue;

        // pixmap=pixmap.transformed(m);

        cfg.setGroup(QString::fromLatin1("KDE Cards"));
        QString name=cfg.readEntry("Name", i18n("unnamed"));
        Q3IconViewItem *item= new Q3IconViewItem(d->deckIconView,name, pixmap);

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
    : KDialogBase( Plain, i18n("Carddeck Selection"), Ok|Cancel, Ok, parent, name, true, true)
{
    KCardDialog::init();

    d = new KCardDialogPrivate;
    d->cFlags = mFlags;
}

void KCardDialog::slotDeckClicked(Q3IconViewItem *item)
{
    if (item && item->pixmap())
    {
        d->deckLabel->setPixmap(* (item->pixmap()));
        d->deckLabel->setToolTip(d->helpMap[d->deckMap[item]]);
        setDeck(d->deckMap[item]);
    }
}
void KCardDialog::slotCardClicked(Q3IconViewItem *item)
{
    if (item && item->pixmap())
    {
        d->cardLabel->setPixmap(* (item->pixmap()));
        QString path = d->cardMap[item];
        d->cardLabel->setToolTip(d->helpMap[path]);
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

    int d = KRandom::random() % list.count();
    return getDeckName(list.at(d));
}

QString KCardDialog::getRandomCardDir()
{
    KCardDialog::init();

    QStringList list = KGlobal::dirs()->findAllResources("cards", "card*/index.desktop");
    if (list.isEmpty())
        return QString::null;

    int d = KRandom::random() % list.count();
    QString entry = list.at(d);
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

void KCardDialog::loadConfig(KConfig* conf)
{
 if (!conf) {
	return;
 }

 QString origGroup = conf->group();

 conf->setGroup(CONF_GROUP);
 if (! (flags() & NoDeck)) {
	if (conf->hasKey(CONF_DECK)) {
		setDeck(conf->readEntry(CONF_DECK));
	}

	bool random = conf->readEntry(CONF_RANDOMDECK, false);
	d->randomDeck->setChecked(random);
	slotRandomDeckToggled(random);

	if (conf->hasKey(CONF_USEGLOBALDECK) && conf->readEntry(CONF_USEGLOBALDECK,false)) {
		d->globalDeck->setChecked(true);
	} else {
		d->globalDeck->setChecked(false);
	}
 }
 if (! (flags() & NoCards)) {
	if (conf->hasKey(CONF_CARDDIR)) {
		setCardDir(conf->readPathEntry(CONF_CARDDIR));
	}

	bool random = conf->readEntry(CONF_RANDOMCARDDIR, false);
	d->randomCardDir->setChecked(random);
	slotRandomCardDirToggled(random);

	if (conf->hasKey(CONF_USEGLOBALCARDDIR) && conf->readEntry(CONF_USEGLOBALCARDDIR,false)) {
		d->globalCardDir->setChecked(true);
	} else {
		d->globalCardDir->setChecked(false);
	}
 }

 d->cScale = conf->readEntry(CONF_SCALE, 1.0);

 conf->setGroup(origGroup);
}

void KCardDialog::slotCardResized(int s)
{
 if (!d->cPreview) {
	return;
 }
 if (s < SLIDER_MIN || s > SLIDER_MAX) {
	kError(11000) << "invalid scaling value!" << endl;
	return;
 }

 s *= -1;
 s += (SLIDER_MIN + SLIDER_MAX);

 QMatrix m;
 double scale = (double)1000/s;
 m.scale(scale, scale);
 QPixmap pix = d->cPreviewPix.transformed(m);
 d->cPreview->setPixmap(pix);
 d->cScale = scale;
}

void KCardDialog::slotDefaultSize()
{
 if (!d->scaleSlider) {
	return;
 }
 d->scaleSlider->setValue(-1000 + SLIDER_MIN + SLIDER_MAX);
}

void KCardDialog::saveConfig(KConfig* conf)
{
 if (!conf) {
	return;
 }
 QString origGroup = conf->group();

 conf->setGroup(CONF_GROUP);
 if (! (flags() & NoDeck)) {
	conf->writeEntry(CONF_DECK, deck());
	conf->writeEntry(CONF_RANDOMDECK, isRandomDeck());
	conf->writeEntry(CONF_USEGLOBALDECK, d->globalDeck->isChecked());
 }
 if (! (flags() & NoCards)) {
	conf->writePathEntry(CONF_CARDDIR, cardDir());
	conf->writeEntry(CONF_RANDOMCARDDIR, isRandomCardDir());
	conf->writeEntry(CONF_USEGLOBALCARDDIR, d->globalCardDir->isChecked());
 }
 conf->writeEntry(CONF_SCALE, d->cScale);

 conf->setGroup(origGroup);
}

void KCardDialog::slotSetGlobalDeck()
{
 KSimpleConfig* conf = new KSimpleConfig(QString::fromLatin1("kdeglobals"), false);
 conf->setGroup(CONF_GLOBAL_GROUP);

 conf->writeEntry(CONF_GLOBAL_DECK, deck());
 conf->writeEntry(CONF_GLOBAL_RANDOMDECK, isRandomDeck());

 delete conf;
}

void KCardDialog::slotSetGlobalCardDir()
{
 KSimpleConfig* conf = new KSimpleConfig(QString::fromLatin1("kdeglobals"), false);
 conf->setGroup(CONF_GLOBAL_GROUP);

 conf->writePathEntry(CONF_GLOBAL_CARDDIR, cardDir());
 conf->writeEntry(CONF_GLOBAL_RANDOMCARDDIR, isRandomCardDir());

 delete conf;
}

void KCardDialog::getGlobalDeck(QString& deck, bool& random)
{
 KSimpleConfig* conf = new KSimpleConfig(QString::fromLatin1("kdeglobals"), true);
 conf->setGroup(CONF_GLOBAL_GROUP);

 if (!conf->hasKey(CONF_GLOBAL_DECK) || conf->readEntry(CONF_GLOBAL_RANDOMDECK, false)) {
	deck = getRandomDeck();
	random = true;
 } else {
	deck = conf->readEntry(CONF_GLOBAL_DECK);
	random = conf->readEntry(CONF_GLOBAL_RANDOMDECK, false);
 }

 delete conf;
}

void KCardDialog::getGlobalCardDir(QString& dir, bool& random)
{
 KSimpleConfig* conf = new KSimpleConfig(QString::fromLatin1("kdeglobals"), true);
 conf->setGroup(CONF_GLOBAL_GROUP);

 if (!conf->hasKey(CONF_GLOBAL_CARDDIR) || conf->readEntry(CONF_GLOBAL_RANDOMCARDDIR, false)) {
	dir = getRandomCardDir();
	random = true;
 } else {
	dir = conf->readPathEntry(CONF_GLOBAL_CARDDIR);
	random = conf->readEntry(CONF_GLOBAL_RANDOMCARDDIR, false);
 }

 delete conf;
}

void KCardDialog::init()
{
    static bool _inited = false;
    if (_inited)
        return;
    KGlobal::dirs()->addResourceType("cards", KStandardDirs::kde_default("data") + QString::fromLatin1("carddecks/"));

    KGlobal::locale()->insertCatalog("libkdegames");
    _inited = true;
}

#include "kcarddialog.moc"
