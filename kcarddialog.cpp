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

#include "kcarddialog.h"

#include <QLabel>
#include <QCheckBox>
#include <QLayout>
#include <QGroupBox>
#include <QSlider>
#include <QMatrix>
#include <QPixmap>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>

#include <klocale.h>
#include <kstandarddirs.h>
#include <krandom.h>
#include <kconfig.h>

#include <kdebug.h>

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
    QListWidget* deckIconView;
    QListWidget* cardIconView;
    QCheckBox* randomDeck;
    QCheckBox* randomCardDir;
    QCheckBox* globalDeck;
    QCheckBox* globalCardDir;

    QSlider* scaleSlider;
    QPixmap cPreviewPix;
    QLabel* cPreview;

    QMap<QListWidgetItem*, QString> deckMap;
    QMap<QListWidgetItem*, QString> cardMap;
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
    KCardDialog dlg(pParent, pFlags);

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
 KConfigGroup cg(conf, CONF_GROUP);

 if (cg.readEntry(CONF_RANDOMDECK,false) || !cg.hasKey(CONF_DECK)) {
	pDeck = getRandomDeck();
 } else {
	pDeck = cg.readEntry(CONF_DECK);
 }
 if (cg.readEntry(CONF_RANDOMCARDDIR,false) || !cg.hasKey(CONF_CARDDIR)) {
	pCardDir = getRandomCardDir();
 } else {
	pCardDir = cg.readPathEntry(CONF_CARDDIR);
 }
 pScale = cg.readEntry(CONF_SCALE, 1.0);

 if (cg.readEntry(CONF_USEGLOBALDECK, false)) {
	bool random;
	getGlobalDeck(pCardDir, random);
	if (random || pDeck.isNull() ) {
		pDeck = getRandomDeck();
	}
 }
 if (cg.readEntry(CONF_USEGLOBALCARDDIR, false)) {
	bool random;
	getGlobalCardDir(pCardDir, random);
	if (random || pCardDir.isNull() ) {
		pCardDir = getRandomCardDir();
	}
 }
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
    return KStandardDirs::locate( "cards", entry );
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
  QFrame *main =new QFrame(this);
  setMainWidget(main);
  QHBoxLayout* topLayout = new QHBoxLayout(main);
  topLayout->setSpacing( spacingHint() );
  QVBoxLayout* cardLayout = new QVBoxLayout;
  QVBoxLayout* gboxLayout;
  topLayout->addLayout( cardLayout );
  QString path, file;
  QMatrix m;
  m.scale(0.8,0.8);

  setInitialSize(QSize(600,400));

  if (! (flags() & NoDeck))
  {
    QHBoxLayout* layout = new QHBoxLayout;
    cardLayout->addLayout( layout );

    // Deck iconview
    QGroupBox* grp1 = new QGroupBox(i18n("Choose Backside"), main);
    layout->addWidget(grp1);
    gboxLayout = new QVBoxLayout(grp1);

    d->deckIconView = new QListWidget(grp1);
    d->deckIconView->setObjectName(QLatin1String("decks"));
    d->deckIconView->setSpacing(8);
//    d->deckIconView->setUniformItemSizes(true);
    d->deckIconView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->deckIconView->setResizeMode(QListView::Adjust);
    d->deckIconView->setMinimumWidth(360);
    d->deckIconView->setMinimumHeight(170);
    d->deckIconView->setWordWrap(false);
    d->deckIconView->setViewMode(QListView::IconMode);
    gboxLayout->addWidget(d->deckIconView);

    // deck select
    QVBoxLayout* l = new QVBoxLayout;
    layout->addLayout(l);
    QGroupBox* grp3 = new QGroupBox(i18n("Backside"), main);
    grp3->setFixedSize(100, 130);
    gboxLayout = new QVBoxLayout(grp3);
    l->addWidget(grp3, 0, Qt::AlignTop|Qt::AlignHCenter);
    d->deckLabel = new QLabel(grp3);
    d->deckLabel->setText(i18n("empty"));
    d->deckLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    d->deckLabel->setGeometry(10, 20, 80, 90);
    gboxLayout->addWidget(d->deckLabel);

    d->randomDeck = new QCheckBox(main);
    d->randomDeck->setChecked(false);
    connect(d->randomDeck, SIGNAL(toggled(bool)), this,
            SLOT(slotRandomDeckToggled(bool)));
    d->randomDeck->setText(i18n("Random backside"));
    l->addWidget(d->randomDeck, 0, Qt::AlignTop|Qt::AlignHCenter);

    d->globalDeck = new QCheckBox(main);
    d->globalDeck->setChecked(false);
    d->globalDeck->setText(i18n("Use global backside"));
    l->addWidget(d->globalDeck, 0, Qt::AlignTop|Qt::AlignHCenter);

    QPushButton* b = new QPushButton(i18n("Make Backside Global"), main);
    connect(b, SIGNAL(pressed()), this, SLOT(slotSetGlobalDeck()));
    l->addWidget(b, 0, Qt::AlignTop|Qt::AlignHCenter);

    connect(d->deckIconView,SIGNAL(itemClicked(QListWidgetItem *)),
            this,SLOT(slotDeckClicked(QListWidgetItem *)));
  }

  if (! (flags() & NoCards))
  {
    // Cards iconview
    QHBoxLayout* layout = new QHBoxLayout;
    cardLayout->addLayout(layout);
    QGroupBox* grp2 = new QGroupBox(i18n("Choose Frontside"), main);
    layout->addWidget(grp2);
    gboxLayout = new QVBoxLayout(grp2);

    d->cardIconView = new QListWidget(grp2);
    d->cardIconView->setObjectName(QLatin1String("cards"));
    d->cardIconView->setSpacing(8);
//    d->cardIconView->setUniformItemSizes(true);
    d->cardIconView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->cardIconView->setResizeMode(QListView::Adjust);
    d->cardIconView->setMinimumWidth(360);
    d->cardIconView->setMinimumHeight(170);
    d->cardIconView->setWordWrap(false);
    d->cardIconView->setViewMode(QListView::IconMode);
    gboxLayout->addWidget(d->cardIconView);

    // Card select
    QVBoxLayout* l = new QVBoxLayout;
    layout->addLayout(l);
    QGroupBox* grp4 = new QGroupBox(i18n("Frontside"), main);
    grp4->setFixedSize(100, 130);
    gboxLayout = new QVBoxLayout(grp4);
    l->addWidget(grp4, 0, Qt::AlignTop|Qt::AlignHCenter);
    d->cardLabel = new QLabel(grp4);
    d->cardLabel->setText(i18n("empty"));
    d->cardLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    d->cardLabel->setGeometry(10, 20, 80, 90 );
    gboxLayout->addWidget(d->cardLabel);

    d->randomCardDir = new QCheckBox(main);
    d->randomCardDir->setChecked(false);
    connect(d->randomCardDir, SIGNAL(toggled(bool)), this,
            SLOT(slotRandomCardDirToggled(bool)));
    d->randomCardDir->setText(i18n("Random frontside"));
    l->addWidget(d->randomCardDir, 0, Qt::AlignTop|Qt::AlignHCenter);

    d->globalCardDir = new QCheckBox(main);
    d->globalCardDir->setChecked(false);
    d->globalCardDir->setText(i18n("Use global frontside"));
    l->addWidget(d->globalCardDir, 0, Qt::AlignTop|Qt::AlignHCenter);

    QPushButton* b = new QPushButton(i18n("Make Frontside Global"), main);
    connect(b, SIGNAL(pressed()), this, SLOT(slotSetGlobalCardDir()));
    l->addWidget(b, 0, Qt::AlignTop|Qt::AlignHCenter);

    connect(d->cardIconView,SIGNAL(itemClicked(QListWidgetItem *)),
            this,SLOT(slotCardClicked(QListWidgetItem *)));
  }

  // Insert deck icons
  // First find the default or alternate path
  if (! (flags() & NoDeck))
  {
      insertDeckIcons();

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
    // I'm sure there is a cleaner way but I cannot find it.
    // whenever the pixmap is resized (aka scaled) the box is resized, too. This
    // leads to an always resizing dialog which is *very* ugly. i worked around
    // this by using a QWidget which is the only child widget of the group box.
    // The other widget are managed inside this QWidget - a stretch area on the
    // right ensures that the KIconViews are not resized...

    // note that the dialog is still resized if you you scale the pixmap very
    // large. This is desired behaviour as i don't want to make the box even
    // larger but i want the complete pixmap to be displayed. the dialog is not
    // resized if you make the pixmap smaller again.
    QVBoxLayout* layout = new QVBoxLayout;
    topLayout->addLayout(layout);
    QGroupBox* grp = new QGroupBox(i18n("Resize Cards"), main);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->addWidget(grp);
    gboxLayout = new QVBoxLayout(grp);
    QWidget* box = new QWidget(grp);
    gboxLayout->addWidget(box);
    QHBoxLayout* hbox = new QHBoxLayout(box);
    hbox->setMargin(0);
    hbox->setSpacing(spacingHint());
    QVBoxLayout* boxLayout = new QVBoxLayout;
    hbox->addLayout(boxLayout);
    hbox->addStretch(0);

    d->scaleSlider = new QSlider(Qt::Horizontal, box);
    d->scaleSlider->setRange( SLIDER_MIN, SLIDER_MAX );
    d->scaleSlider->setPageStep(1);
    d->scaleSlider->setValue( -1000+SLIDER_MIN+SLIDER_MAX );

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
    QStringList list = KGlobal::dirs()->findAllResources("cards", "card*/index.desktop",
                                                         KStandardDirs::NoDuplicates);
    // kDebug(11000) << "insert " << list.count() << endl;
    if (list.isEmpty())
        return;

    // We shrink the icons a little
    //
    QMatrix m;
    m.scale(0.8,0.8);

    QSize itemSize;

    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        KConfig cfg(*it, KConfig::OnlyLocal);
        KConfigGroup cfgcg(&cfg, "KDE Backdeck");
        QString path = (*it).left((*it).lastIndexOf('/') + 1);
        Q_ASSERT(path[path.length() - 1] == '/');
        QPixmap pixmap(path + cfgcg.readEntry("Preview", "12c.png"));

        if (pixmap.isNull())
            continue;

        QString name = cfgcg.readEntry("Name", i18n("unnamed"));
        QListWidgetItem *item = new QListWidgetItem(name, d->cardIconView);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        item->setToolTip(name);
        item->setData(Qt::DecorationRole, pixmap);

        d->cardMap[item] = path;
        d->helpMap[path] = cfgcg.readEntry("Comment",name);

        itemSize = itemSize.expandedTo(pixmap.size());
    }
//    d->cardIconView->setUniformItemSizes(true);
    d->cardIconView->setIconSize(itemSize);
}

void KCardDialog::insertDeckIcons()
{
    QStringList list = KGlobal::dirs()->findAllResources("cards", "decks/*.desktop",
                                                         KStandardDirs::NoDuplicates);
    if (list.isEmpty())
        return;

    QString label;

    // We shrink the icons a little
    QMatrix m;
    m.scale(0.8,0.8);

    QSize itemSize;

    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        KConfig cfg(*it, KConfig::OnlyLocal);
        QPixmap pixmap(getDeckName(*it));
        if (pixmap.isNull())
            continue;

        // pixmap=pixmap.transformed(m);

        KConfigGroup cfgcg(&cfg, "KDE Cards");
        QString name = cfgcg.readEntry("Name", i18n("unnamed"));
        QListWidgetItem *item = new QListWidgetItem(name, d->deckIconView);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        item->setToolTip(name);
        item->setData(Qt::DecorationRole, pixmap);

        d->deckMap[item] = getDeckName(*it);
        d->helpMap[d->deckMap[item]] = cfgcg.readEntry("Comment",name);

        itemSize = itemSize.expandedTo(pixmap.size());
    }
//    d->deckIconView->setUniformItemSizes(true);
    d->deckIconView->setIconSize(itemSize);
}


KCardDialog::~KCardDialog()
{
 delete d;
}


// Create the dialog
KCardDialog::KCardDialog( QWidget *parent, CardFlags mFlags)
    : KDialog( parent)
{
	setCaption(i18n("Carddeck Selection"));
	setButtons(Ok|Cancel);
	setDefaultButton(Ok);
	setModal(true);
	showButtonSeparator(true);
    KCardDialog::init();

    d = new KCardDialogPrivate;
    d->cFlags = mFlags;
}

void KCardDialog::slotDeckClicked(QListWidgetItem *item)
{
    if (item && item->data(Qt::DecorationRole).type() == QVariant::Pixmap)
    {
        d->deckLabel->setPixmap(item->data(Qt::DecorationRole).value<QPixmap>());
        d->deckLabel->setToolTip(d->helpMap[d->deckMap[item]]);
        setDeck(d->deckMap[item]);
    }
}

void KCardDialog::slotCardClicked(QListWidgetItem *item)
{
    if (item && item->data(Qt::DecorationRole).type() == QVariant::Pixmap)
    {
        d->cardLabel->setPixmap(item->data(Qt::DecorationRole).value<QPixmap>());
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
        return QString();

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

 KConfigGroup cg(conf, CONF_GROUP);

 if (! (flags() & NoDeck)) {
	if (cg.hasKey(CONF_DECK)) {
		setDeck(cg.readEntry(CONF_DECK));
	}

	bool random = cg.readEntry(CONF_RANDOMDECK, false);
	d->randomDeck->setChecked(random);
	slotRandomDeckToggled(random);

	if (cg.hasKey(CONF_USEGLOBALDECK) && cg.readEntry(CONF_USEGLOBALDECK,false)) {
		d->globalDeck->setChecked(true);
	} else {
		d->globalDeck->setChecked(false);
	}
 }
 if (! (flags() & NoCards)) {
	if (cg.hasKey(CONF_CARDDIR)) {
		setCardDir(cg.readPathEntry(CONF_CARDDIR));
	}

	bool random = cg.readEntry(CONF_RANDOMCARDDIR, false);
	d->randomCardDir->setChecked(random);
	slotRandomCardDirToggled(random);

	if (cg.hasKey(CONF_USEGLOBALCARDDIR) && cg.readEntry(CONF_USEGLOBALCARDDIR,false)) {
		d->globalCardDir->setChecked(true);
	} else {
		d->globalCardDir->setChecked(false);
	}
 }

 d->cScale = cg.readEntry(CONF_SCALE, 1.0);
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
 KConfigGroup cg(conf, CONF_GROUP);
 if (! (flags() & NoDeck)) {
	cg.writeEntry(CONF_DECK, deck());
	cg.writeEntry(CONF_RANDOMDECK, isRandomDeck());
	cg.writeEntry(CONF_USEGLOBALDECK, d->globalDeck->isChecked());
 }
 if (! (flags() & NoCards)) {
	cg.writePathEntry(CONF_CARDDIR, cardDir());
	cg.writeEntry(CONF_RANDOMCARDDIR, isRandomCardDir());
	cg.writeEntry(CONF_USEGLOBALCARDDIR, d->globalCardDir->isChecked());
 }
 cg.writeEntry(CONF_SCALE, d->cScale);
}

void KCardDialog::slotSetGlobalDeck()
{
 KConfig conf(QString::fromLatin1("kdeglobals"), KConfig::OnlyLocal);
 KConfigGroup cg(&conf, CONF_GLOBAL_GROUP);

 cg.writeEntry(CONF_GLOBAL_DECK, deck());
 cg.writeEntry(CONF_GLOBAL_RANDOMDECK, isRandomDeck());
}

void KCardDialog::slotSetGlobalCardDir()
{
 KConfig conf(QString::fromLatin1("kdeglobals"), KConfig::OnlyLocal);
 KConfigGroup cg(&conf, CONF_GLOBAL_GROUP);

 cg.writePathEntry(CONF_GLOBAL_CARDDIR, cardDir());
 cg.writeEntry(CONF_GLOBAL_RANDOMCARDDIR, isRandomCardDir());
}

void KCardDialog::getGlobalDeck(QString& deck, bool& random)
{
 KConfig* conf = new KConfig(QString::fromLatin1("kdeglobals"), KConfig::OnlyLocal);
 KConfigGroup cg(conf, CONF_GLOBAL_GROUP);

 if (!cg.hasKey(CONF_GLOBAL_DECK) || cg.readEntry(CONF_GLOBAL_RANDOMDECK, false)) {
	deck = getRandomDeck();
	random = true;
 } else {
	deck = cg.readEntry(CONF_GLOBAL_DECK);
	random = cg.readEntry(CONF_GLOBAL_RANDOMDECK, false);
 }

 delete conf;
}

void KCardDialog::getGlobalCardDir(QString& dir, bool& random)
{
 KConfig* conf = new KConfig(QString::fromLatin1("kdeglobals"), KConfig::OnlyLocal);
 KConfigGroup cg(conf, CONF_GLOBAL_GROUP);

 if (!cg.hasKey(CONF_GLOBAL_CARDDIR) || cg.readEntry(CONF_GLOBAL_RANDOMCARDDIR, false)) {
	dir = getRandomCardDir();
	random = true;
 } else {
	dir = cg.readPathEntry(CONF_GLOBAL_CARDDIR);
	random = cg.readEntry(CONF_GLOBAL_RANDOMCARDDIR, false);
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
