/*
    This file is part of the KDE games library

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
#include "ui_kgamecardselector.h"

#include <QPixmap>
#include <QListWidgetItem>
#include <QFileInfo>
#include <QDir>

#include <klocale.h>
#include <kstandarddirs.h>
#include <krandom.h>
#include <kdebug.h>

// KConfig entries
#define CONF_GROUP  QString::fromLatin1("KCardDialog")
#define CONF_LOCKING QString::fromLatin1("Locking")
#define CONF_ALLOW_FIXED_CARDS QString::fromLatin1("Fixed")
#define CONF_ALLOW_SCALED_CARDS QString::fromLatin1("Scaled")
#define CONF_CARD QString::fromLatin1("Cardname")
#define CONF_DECK QString::fromLatin1("Deckname")

/**
 * Stores the information for one card front or back side.
 */
class KCardInfo
{
  public:
   /** The translated name.
    */
   QString name;

   /** The untranslated name.
    */
   QString noi18Name;

   /** The comment (author and description).
    */
   QString comment;
   
   /** The full path information.
    */
   QString path;

   /** The translated name of the back side.
    */
   QString back;

   /** The preview image.
    */
   QPixmap preview;

   /** The full filename of the SVG file.
    */
   QString svgfile;
   
   /** The default size.
   */
   QSizeF size;
   
   /** Is this a default deck or set.
   */
   bool isDefault;
};

/**
 * Local information of the dialog.
 */
class KCardDialogPrivate
{
  public:

    /** Constructor.
     */
    KCardDialogPrivate()
    {
       // Set default values
       useSVGOnly = false;
       usePNGOnly = false;
       useLocking = true;
    }

    /** Filter a fiven card front/back depending on its scalable
      * or non-scalable properties.
      * @param v The card info structure.
      * @return True if the card should bve discarded.
      */
    bool filterOutCard(const KCardInfo& v)
    {
      if (usePNGOnly && !v.svgfile.isNull()) return true;
      if (useSVGOnly && v.svgfile.isNull()) return true;
      return false;
    }

    /** Should scalable cards are shown.
     */
    bool useSVGOnly;

    /** Should non scalable cards are shown.
     */
    bool usePNGOnly;

    /** Are scalable cards allowed.
     */
    bool allowSVG;

    /** Are non scalable cards allowed.
     */
    bool allowPNG;

    /** Is the back side locked to the front side.
     */
    bool useLocking;

    /** Currently chosen back side name.
     */
    QString currentDeck;

    /** Currently chosen front side name.
     */
    QString currentCard;

    /** The UI elements.
     */
    Ui::KGameCardSelectorBase ui;
};

/**
 * Local static information.
 */
class KCardDialogStatic
{
  public:
    /** The card front sides.
     */
    QMap<QString, KCardInfo> cardInfo;

    /** The card back sides.
     */
    QMap<QString, KCardInfo> deckInfo;

    /** The default front side name.
     */
    QString defaultCard;
    
    /** The default back side name.
     */
    QString defaultDeck;

    /** Filter a fiven card front/back depending on its scalable
      * or non-scalable properties.
      * @param v The card info structure.
      * @param useSVGOnly Are only SVG cards shown
      * @param usePNGOnly Are only PNG cards shown
      * @return True if the card should bve discarded.
      */
    bool filterOutCard(const KCardInfo& v, bool useSVGOnly, bool usePNGOnly)
    {
      if (usePNGOnly && !v.svgfile.isNull()) return true;
      if (useSVGOnly && v.svgfile.isNull()) return true;
      return false;
    }
};

// Store local static information.
static KCardDialogStatic ds;


// Create the dialog
KCardDialog::KCardDialog(QWidget *parent, bool pAllowSVG, bool pAllowPNG, bool pLock, QString defFront, QString defBack)
           : KDialog( parent ), d( new KCardDialogPrivate )
{
  KCardDialog::init();

  // Copy parameter
  d->useLocking = pLock;
  d->allowPNG = pAllowPNG;
  d->allowSVG = pAllowSVG;
  d->currentCard = defFront;
  d->currentDeck = defBack;

  // GUI
  setupGUI();
}

// Create the dialog from a config group
KCardDialog::KCardDialog(KConfigGroup& group, QWidget* parent)
           : KDialog( parent ), d( new KCardDialogPrivate )
{
  KCardDialog::init();

  d->useLocking  = group.readEntry(CONF_LOCKING, true);
  d->allowPNG    = group.readEntry(CONF_ALLOW_FIXED_CARDS, true);
  d->allowSVG    = group.readEntry(CONF_ALLOW_SCALED_CARDS, true);
  d->currentCard = group.readEntry(CONF_CARD, QString());
  d->currentDeck = group.readEntry(CONF_DECK, QString());

  // GUI
  setupGUI();
}


// Store the config group settings
void KCardDialog::saveSettings(KConfigGroup& group)
{
  group.writeEntry(CONF_LOCKING, d->useLocking);
  group.writeEntry(CONF_ALLOW_FIXED_CARDS, d->allowPNG);
  group.writeEntry(CONF_ALLOW_SCALED_CARDS, d->allowSVG);
  group.writeEntry(CONF_CARD, d->currentCard);
  group.writeEntry(CONF_DECK, d->currentDeck); 
}


// Setup the user interface
void KCardDialog::setupGUI()
{
  // Dialog
  setCaption(i18n("Carddeck Selection"));
  setButtons(Ok|Cancel);
  setDefaultButton(Ok);
  setModal(true);
  showButtonSeparator(true);
  //adjustSize();

  // Inner widget
  Ui::KGameCardSelectorBase* ui = &(d->ui);
  d->ui.setupUi(mainWidget());
  
  // Game does not allow fixed sized cards
  if (!d->allowPNG)
  {
    ui->checkBoxPNG->setEnabled(false);
    ui->checkBoxSVG->setEnabled(false);
    ui->checkBoxPNG->setCheckState(Qt::Unchecked);
    d->usePNGOnly = false;
    d->useSVGOnly = true;
  }

  // Game does not allow scaled cards
  if (!d->allowSVG)
  {
    ui->checkBoxSVG->setEnabled(false);
    ui->checkBoxPNG->setEnabled(false);
    ui->checkBoxSVG->setCheckState(Qt::Unchecked);
    d->useSVGOnly = false;
    d->usePNGOnly = true;
  }

  // Set checkboxes
  if (d->useLocking) ui->checkBoxLock->setCheckState(Qt::Checked);
  if (d->useSVGOnly) ui->checkBoxSVG->setCheckState(Qt::Checked);
  if (d->usePNGOnly) ui->checkBoxPNG->setCheckState(Qt::Checked);

  // Game wants locked backsides?
  ui->backList->setEnabled(!d->useLocking);

  // Set lists and preview
   if (d->currentCard.isNull()) d->currentCard = defaultCardName(d->allowSVG, d->allowPNG);
   if (d->currentDeck.isNull()) d->currentDeck = defaultDeckName(d->allowSVG, d->allowPNG);
  insertCardIcons();
  insertDeckIcons();
  updateFront(d->currentCard);
  updateBack(d->currentDeck);

  
  // Connect signals
  connect(ui->frontList, SIGNAL(currentItemChanged( QListWidgetItem * , QListWidgetItem * )),
          this, SLOT(updateFront(QListWidgetItem * , QListWidgetItem * )));
  connect(ui->backList, SIGNAL(currentItemChanged( QListWidgetItem * , QListWidgetItem * )),
          this, SLOT(updateBack(QListWidgetItem * , QListWidgetItem * )));
  connect(ui->checkBoxLock, SIGNAL(stateChanged(int)), this, SLOT(updateLocking(int)));
  connect(ui->checkBoxSVG, SIGNAL(stateChanged(int)), this, SLOT(updateSVG(int)));
  connect(ui->checkBoxPNG, SIGNAL(stateChanged(int)), this, SLOT(updatePNG(int)));

  // Debug
  // kDebug() << "DEFAULT DECK: " << defaultDeckName(pAllowSVG, pAllowPNG);
  // kDebug() << "DEFAULT CARD: " << defaultCardName(pAllowSVG, pAllowPNG);
  // kDebug() << "RANDOM DECK: " << randomDeckName(pAllowSVG, pAllowPNG);
  // kDebug() << "RANDOM CARD: " << randomCardName(pAllowSVG, pAllowPNG);
}


// Destroy the dialog
KCardDialog::~KCardDialog()
{
  delete d;
}


// Perform static initialization
void KCardDialog::init()
{
  static bool _inited = false;
  if (_inited) return;
  _inited = true;

  KGlobal::dirs()->addResourceType("cards", "data", "carddecks/");
  KGlobal::locale()->insertCatalog("libkdegames");
  reset();
}


// Reset the static information. Reread cards.
void KCardDialog::reset()
{
  // Important to read backs first
  readBacks();
  readFronts();
}


// Retrieve card set and deck from dialog.
int KCardDialog::getCardDeck(QString &pFrontName,
                             QString &pBackName,
                             QWidget *pParent,
                             bool pAllowSVG,
                             bool pAllowPNG,
                             bool pLock, 
                             bool pRandom 
                             )
{
  KCardDialog::init();


  // If random cards we need no dialog (KDE 3.x compatibility)
  if (pRandom)
  {
    pFrontName = randomCardName();
    pBackName  = randomDeckName();
    return QDialog::Accepted;
  }

  KCardDialog dlg(pParent, pAllowSVG, pAllowPNG, pLock, pFrontName, pBackName);


  int result=dlg.exec();
  if (result==QDialog::Accepted)
  {
    pFrontName = dlg.cardName();
    pBackName  = dlg.deckName();
  }
  return result;
}


// Retrieve selected deck name
QString KCardDialog::deckName() const
{
  return d->currentDeck; 
}


// Retrieve selected card name
QString KCardDialog::cardName() const
{ 
  return d->currentCard; 
}


// Read card front side data
void KCardDialog::readFronts()
{
    // Empty data
    ds.cardInfo.clear();

    QStringList svg;
    // Add SVG card sets
    svg = KGlobal::dirs()->findAllResources("cards", "svg*/index.desktop", KStandardDirs::NoDuplicates);
    QStringList list = svg+KGlobal::dirs()->findAllResources("cards", "card*/index.desktop", KStandardDirs::NoDuplicates);

    if (list.isEmpty()) return;

    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        KConfig cfg(*it, KConfig::SimpleConfig);
        KConfigGroup cfgcg(&cfg, "KDE Backdeck");
        QString path = (*it).left((*it).lastIndexOf('/') + 1);
        Q_ASSERT(path[path.length() - 1] == '/');
        QPixmap pixmap(path + cfgcg.readEntry("Preview", "12c.png"));

        if (pixmap.isNull()) continue;

        QString idx  = cfgcg.readEntryUntranslated("Name", i18n("unnamed"));
        QString name = cfgcg.readEntry("Name", i18n("unnamed"));
        KCardInfo info;
        info.name         = name;
        info.noi18Name    = idx;
        info.comment      = cfgcg.readEntry("Comment",i18n("KDE card deck"));
        info.preview      = pixmap; 
        info.path         = path;
        info.back         = cfgcg.readEntry("Back",QString());
        // The back name is read UNTRANSLATED...we need to find the right name for it now
        info.back         = findi18nBack(info.back);
        // if (!info.back.isNull()) kDebug() << "FOUND BACK " << info.back;
        info.size         = cfgcg.readEntry("BackSize", QSizeF(pixmap.size()));
        info.isDefault    = cfgcg.readEntry("Default", false);

        QString svg    = cfgcg.readEntry("SVG", QString());
        if (!svg.isNull())
        {
          QFileInfo svgInfo(QDir(path), svg);
          info.svgfile = svgInfo.filePath();
        }
        else
        {
          info.svgfile = QString();
        }

        ds.cardInfo[name] = info;

    }
}


// Translate back side
QString KCardDialog::findi18nBack(QString& name)
{
  if (name.isNull()) return name;

  QMapIterator<QString, KCardInfo> it(ds.deckInfo);
  while (it.hasNext())
  {
      it.next();
      KCardInfo v = it.value();
      if (v.noi18Name == name) return v.name;
  }
  kError() << "No translation for back card " << name << "found";
  return name;
}


// Build list widget
void KCardDialog::insertCardIcons()
{
    // Prevent empty preview
    if (d->useSVGOnly && !isSVGCard(d->currentCard)) updateFront(defaultCardName(!d->usePNGOnly, !d->useSVGOnly));
    if (d->usePNGOnly && isSVGCard(d->currentCard)) updateFront(defaultCardName(!d->usePNGOnly, !d->useSVGOnly));

    // Clear GUI
    d->ui.frontList->clear();

    // Rebuild list
    QSize itemSize;
    QMapIterator<QString, KCardInfo> it(ds.cardInfo);
    while (it.hasNext())
    {
        it.next();
        KCardInfo v = it.value();
        // Show only SVG files?
        if (d->filterOutCard(v)) continue;

        QPixmap previewPixmap = v.preview.scaled(QSize(32,43), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        QListWidgetItem *item = new QListWidgetItem(v.name, d->ui.frontList);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        item->setToolTip(v.name);
        item->setData(Qt::DecorationRole, previewPixmap);
        itemSize = itemSize.expandedTo(previewPixmap.size());
    }
    d->ui.frontList->setIconSize(itemSize);
}


// Update front preview
void KCardDialog::updateFront(QListWidgetItem* current , QListWidgetItem* /*last*/)
{
  if (current != 0) updateFront(current->text());
}


// Update front preview
void KCardDialog::updateFront(QString item)
{
  // Clear item?
  if (item.isNull())
  {
    d->ui.frontPreview->setPixmap(QPixmap());
    d->ui.cardName->setText(QString());
    d->ui.cardDescription->setText(QString());
  }
  else
  {
    KCardInfo info = ds.cardInfo[item];
    QFont font;
    font.setBold(true);
    d->ui.cardName->setText(info.name);
    d->ui.cardName->setFont(font);

    d->ui.cardDescription->setText(info.comment);
    QPixmap pixmap= info.preview;
    if (pixmap.height() > d->ui.frontPreview->height())
      pixmap = pixmap.scaledToHeight(d->ui.frontPreview->height(), Qt::SmoothTransformation);
    if (pixmap.width() > d->ui.frontPreview->width())
      pixmap = pixmap.scaledToWidth(d->ui.frontPreview->width(), Qt::SmoothTransformation);
    d->ui.frontPreview->setPixmap(pixmap);

    // Lock front and back side?
    if (d->useLocking && !info.back.isNull())
    {
      updateBack(info.back);
    }
    else if (d->useLocking)
    {
      // QMap<QString, KCardInfo>::const_iterator it = d->deckInfo.constBegin();
      QString name = defaultDeckName(!d->usePNGOnly, !d->useSVGOnly);
      updateBack(name);
    }
  }
  d->currentCard = item;
}


// Retrieve default card directory
QString KCardDialog::getDefaultCardDir(bool pAllowSVG, bool pAllowPNG)
{
  KCardDialog::init();
  QString name = defaultCardName(pAllowSVG, pAllowPNG);
  return cardDir(name);
}


// Retrieve default deck file name
QString KCardDialog::getDefaultDeck(bool pAllowSVG, bool pAllowPNG)
{
  KCardDialog::init();
  QString name = defaultDeckName(pAllowSVG, pAllowPNG);
  return deckFilename(name);
}


// Retrieve default card set name
QString KCardDialog::defaultCardName(bool pAllowSVG, bool pAllowPNG)
{
  KCardDialog::init();
  QString noDefault;
  // Count filtered cards
  QMapIterator<QString, KCardInfo> it(ds.cardInfo);
  while (it.hasNext())
  {
      it.next();
      KCardInfo v = it.value();
      // Filter
      if (ds.filterOutCard(v, !pAllowPNG, !pAllowSVG)) continue;
      if (v.isDefault) return v.name;
      // Collect any deck if no default is stored
      noDefault = v.name;
  }
  if (noDefault.isNull()) kError() << "Could not find default card name";
  return noDefault;
}


// Retrieve default deck name
QString KCardDialog::defaultDeckName(bool pAllowSVG, bool pAllowPNG)
{
  KCardDialog::init();
  QString noDefault;
  // Count filtered cards
  QMapIterator<QString, KCardInfo> it(ds.deckInfo);
  while (it.hasNext())
  {
      it.next();
      KCardInfo v = it.value();
      // Filter
      if (ds.filterOutCard(v, !pAllowPNG, !pAllowSVG)) continue;
      if (v.isDefault) 
      {
        return v.name;
      }
      // Collect any deck if no default is stored
      noDefault = v.name;
  }
  if (noDefault.isNull()) kError() << "Could not find default deck name";
  return noDefault;
}


// Retrieve a random card name
QString KCardDialog::randomCardName(bool pAllowSVG, bool pAllowPNG)
{
  KCardDialog::init();
  // Collect matching items
  QStringList list;

  // Count filtered cards
  QMapIterator<QString, KCardInfo> it(ds.cardInfo);
  while (it.hasNext())
  {
      it.next();
      KCardInfo v = it.value();
      // Filter
      if (ds.filterOutCard(v, !pAllowPNG, !pAllowSVG)) continue;
      list.append(v.name);
  }

  // Draw random one
  int d = KRandom::random() % list.count();
  return list.at(d);
}


// Retrieve a random deck name
QString KCardDialog::randomDeckName(bool pAllowSVG, bool pAllowPNG)
{
  KCardDialog::init();
  // Collect matching items
  QStringList list;

  // Count filtered cards
  QMapIterator<QString, KCardInfo> it(ds.deckInfo);
  while (it.hasNext())
  {
      it.next();
      KCardInfo v = it.value();
      // Filter
      if (ds.filterOutCard(v, !pAllowPNG, !pAllowSVG)) continue;
      list.append(v.name);
  }

  // Draw random one
  int d = KRandom::random() % list.count();
  return list.at(d);
}


// Update the locking filter
void KCardDialog::updateLocking(int state)
{
  if (state == Qt::Checked)
  {
    d->useLocking = true;
    // Update previews
    updateFront(d->currentCard);
  }
  else
  {
    d->useLocking = false;
  }
  d->ui.backList->setEnabled(!d->useLocking);
}


// Update the SVG status filter
void KCardDialog::updateSVG(int state)
{
  if (state == Qt::Checked)
  {
    d->useSVGOnly = true;
  }
  else
  {
    d->useSVGOnly = false;
  }
  //Prevent filtering out everything
  if (d->usePNGOnly && d->useSVGOnly)
  { 
    d->usePNGOnly = false;
    d->ui.checkBoxPNG->setCheckState(Qt::Unchecked);
  }
  insertCardIcons();
  insertDeckIcons();
}


// Update the PNG status filter
void KCardDialog::updatePNG(int state)
{
  if (state == Qt::Checked)
  {
    d->usePNGOnly = true;
  }
  else
  {
    d->usePNGOnly = false;
  }
  //Prevent filtering out everything
  if (d->usePNGOnly && d->useSVGOnly)
  { 
    d->useSVGOnly = false;
    d->ui.checkBoxSVG->setCheckState(Qt::Unchecked);
  }
  insertCardIcons();
  insertDeckIcons();
}


// Update the back preview
void KCardDialog::updateBack(QListWidgetItem* current , QListWidgetItem* /*last*/)
{
  if (current != 0) updateBack(current->text());
}


// Update the back preview
void KCardDialog::updateBack(QString item)
{
  if (item.isNull())
  {
    d->ui.backPreview->setPixmap(QPixmap());
  }
  else
  {
    KCardInfo info = ds.deckInfo[item];
    QPixmap pixmap= info.preview;
    if (pixmap.height() > d->ui.backPreview->height())
      pixmap = pixmap.scaledToHeight(d->ui.backPreview->height(), Qt::SmoothTransformation);
    if (pixmap.width() > d->ui.backPreview->width())
      pixmap = pixmap.scaledToWidth(d->ui.backPreview->width(), Qt::SmoothTransformation);
    d->ui.backPreview->setPixmap(pixmap);
  }
  d->currentDeck = item;
}


// Insert the deck icons into the list widget
void KCardDialog::insertDeckIcons()
{
    // Prevent empty preview
    if (d->useSVGOnly && !isSVGDeck(d->currentDeck)) updateBack(defaultDeckName(!d->usePNGOnly, !d->useSVGOnly));
    if (d->usePNGOnly && isSVGDeck(d->currentDeck)) updateBack(defaultDeckName(!d->usePNGOnly, !d->useSVGOnly));

    // Clear GUI
    d->ui.backList->clear();

    // Rebuild list
    QSize itemSize;
    QMapIterator<QString, KCardInfo> it(ds.deckInfo);
    while (it.hasNext())
    {
        it.next();
        KCardInfo v = it.value();
        // Show only SVG files?
        if (d->filterOutCard(v)) continue;
        QPixmap previewPixmap = v.preview.scaled(QSize(32,43), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        QString name = v.name;
        QListWidgetItem *item = new QListWidgetItem(name, d->ui.backList);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        item->setToolTip(name);
        item->setData(Qt::DecorationRole, previewPixmap);
        itemSize = itemSize.expandedTo(previewPixmap.size());
    }
    d->ui.backList->setIconSize(itemSize);
}


// Read the card deck data
void KCardDialog::readBacks()
{
    // Empty data
    ds.deckInfo.clear();

    QStringList list = KGlobal::dirs()->findAllResources("cards", "decks/*.desktop", KStandardDirs::NoDuplicates);
    if (list.isEmpty()) return;

    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        KConfig cfg(*it, KConfig::SimpleConfig);
        QString path = (*it).left((*it).lastIndexOf('/') + 1);
        Q_ASSERT(path[path.length() - 1] == '/');
        QPixmap pixmap(getDeckFileNameFromIndex(*it));
        if (pixmap.isNull()) continue;
        //pixmap = pixmap.scaledToWidth(72, Qt::SmoothTransformation);
        QPixmap previewPixmap = pixmap.scaled(QSize(32,43), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        KConfigGroup cfgcg(&cfg, "KDE Cards");
        QString idx  = cfgcg.readEntryUntranslated("Name", i18n("unnamed"));
        QString name = cfgcg.readEntry("Name", i18n("unnamed"));
        KCardInfo info;
        info.name         = name;
        info.noi18Name    = idx;
        info.path         = getDeckFileNameFromIndex(*it);
        info.comment      = cfgcg.readEntry("Comment",i18n("KDE card deck"));
        info.preview      = pixmap; 
        info.size         = cfgcg.readEntry("Size", QSizeF(pixmap.size()));
        info.isDefault    = cfgcg.readEntry("Default", false);

        QString svg    = cfgcg.readEntry("SVG", QString());
        if (!svg.isNull())
        {
          QFileInfo svgInfo(QDir(path), svg);
          info.svgfile = svgInfo.filePath();
        }
        else
        {
          info.svgfile = QString();
        }
        ds.deckInfo[name] = info;
    }
}


// Retrieve the PNG filename for a back side from its index.desktop filename
QString KCardDialog::getDeckFileNameFromIndex(const QString &desktop)
{
    QString entry = desktop.left(desktop.length() - strlen(".desktop"));
    if (KStandardDirs::exists(entry + QString::fromLatin1(".png")))
        return entry + QString::fromLatin1(".png");

    // rather theoretical
    if (KStandardDirs::exists(entry + QString::fromLatin1(".xpm")))
        return entry + QString::fromLatin1(".xpm");
    return QString();
}



// Retrieve the SVG file belonging to the given card back deck.
QString KCardDialog::deckSVGFilePath(const QString& name)
{
  KCardDialog::init();
  if (!ds.deckInfo.contains(name)) return QString();
  KCardInfo v = ds.deckInfo.value(name);
  return v.svgfile;
}


// Retrieve the SVG file belonging to the given card fronts.
QString KCardDialog::cardSVGFilePath(const QString& name)
{
  KCardDialog::init();
  if (!ds.cardInfo.contains(name)) return QString();
  KCardInfo v = ds.cardInfo.value(name);
  return v.svgfile;
}


// Retrieve the PNG file belonging to the given card back deck.
QString KCardDialog::deckFilename(const QString& name)
{
  KCardDialog::init();
  if (!ds.deckInfo.contains(name)) return QString();
  KCardInfo v = ds.deckInfo.value(name);
  return v.path;
}


// Retrieve the directory belonging to the given card fronts.
QString KCardDialog::cardDir(const QString& name)
{
  KCardDialog::init();
  if (!ds.cardInfo.contains(name)) return QString();
  KCardInfo v = ds.cardInfo.value(name);
  return v.path;
}


// Check whether a card set is SVG
bool KCardDialog::isSVGCard(const QString& name)
{
  KCardDialog::init();
  if (!ds.cardInfo.contains(name)) return false;
  KCardInfo v = ds.cardInfo.value(name);
  return !v.svgfile.isNull();
}


// Check whether a card deck is SVG
bool KCardDialog::isSVGDeck(const QString& name)
{
  KCardDialog::init();
  if (!ds.deckInfo.contains(name)) return false;
  KCardInfo v = ds.deckInfo.value(name);
  return !v.svgfile.isNull();
}

#include "kcarddialog.moc"
