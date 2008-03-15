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

#include "carddeckinfo.h"
#include "carddeckinfo_p.h"

// KConfig entries
#define CONF_GROUP  QString::fromLatin1("KCardDialog")
#define CONF_LOCKING QString::fromLatin1("Locking")
#define CONF_ALLOW_FIXED_CARDS QString::fromLatin1("AllowFixed")
#define CONF_ALLOW_SCALED_CARDS QString::fromLatin1("AllowScaled")
#define CONF_SHOWONLY_FIXED_CARDS QString::fromLatin1("ShowFixedOnly")
#define CONF_SHOWONLY_SCALED_CARDS QString::fromLatin1("ShowScaledOnly")
#define CONF_CARD QString::fromLatin1("Cardname")
#define CONF_DECK QString::fromLatin1("Deckname")

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
    bool filterOutCard(const KCardThemeInfo& v)
    {
      if (usePNGOnly && !v.svgfile.isNull()) return true;
      if (useSVGOnly && v.svgfile.isNull()) return true;
      return false;
    }

    /** Currently chosen back side name.
     */
    QString currentBack;

    /** Currently chosen front side name.
     */
    QString currentFront;

    /** The UI elements.
     */
    Ui::KGameCardSelectorBase ui;

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
};

// Create the dialog
KCardDialog::KCardDialog(QWidget *parent, bool pAllowSVG, bool pAllowPNG, bool pLock, QString defFront, QString defBack)
           : KDialog( parent ), d( new KCardDialogPrivate )
{
  // Copy parameter
  d->useLocking = pLock;
  d->allowPNG = pAllowPNG;
  d->allowSVG = pAllowSVG;
  d->currentFront = defFront;
  d->currentBack = defBack;

  // GUI
  setupGUI();
}

// Create the dialog from a config group
KCardDialog::KCardDialog(KConfigGroup& group, QWidget* parent)
           : KDialog( parent ), d( new KCardDialogPrivate )
{
  d->useLocking  = group.readEntry(CONF_LOCKING, true);
  d->allowPNG    = group.readEntry(CONF_ALLOW_FIXED_CARDS, true);
  d->allowSVG    = group.readEntry(CONF_ALLOW_SCALED_CARDS, true);
  d->usePNGOnly  = group.readEntry(CONF_SHOWONLY_FIXED_CARDS, false);
  d->useSVGOnly  = group.readEntry(CONF_SHOWONLY_SCALED_CARDS, false);
  d->currentFront = group.readEntry(CONF_CARD, QString());
  d->currentBack = group.readEntry(CONF_DECK, QString());

  // GUI
  setupGUI();
}


// Store the config group settings
void KCardDialog::saveSettings(KConfigGroup& group)
{
  group.writeEntry(CONF_LOCKING, d->useLocking);
  group.writeEntry(CONF_ALLOW_FIXED_CARDS, d->allowPNG );
  group.writeEntry(CONF_ALLOW_SCALED_CARDS, d->allowSVG );
  group.writeEntry(CONF_SHOWONLY_FIXED_CARDS, d->usePNGOnly );
  group.writeEntry(CONF_SHOWONLY_SCALED_CARDS, d->useSVGOnly );
  group.writeEntry(CONF_CARD, d->currentFront);
  group.writeEntry(CONF_DECK, d->currentBack);
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
   if (d->currentFront.isNull()) d->currentFront = CardDeckInfo::defaultFrontName(d->allowSVG, d->allowPNG);
   if (d->currentBack.isNull()) d->currentBack = CardDeckInfo::defaultBackName(d->allowSVG, d->allowPNG);
  insertCardIcons();
  insertDeckIcons();
  updateFront(d->currentFront);
  updateBack(d->currentBack);

  
  // Connect signals
  connect(ui->frontList, SIGNAL(itemSelectionChanged()),
          this, SLOT(updateFront()));
  connect(ui->backList, SIGNAL(itemSelectionChanged()),
          this, SLOT(updateBack()));
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
  // If random cards we need no dialog (KDE 3.x compatibility)
  if (pRandom)
  {
    pFrontName = CardDeckInfo::randomFrontName();
    pBackName  = CardDeckInfo::randomBackName();
    return QDialog::Accepted;
  }

  KCardDialog dlg(pParent, pAllowSVG, pAllowPNG, pLock, pFrontName, pBackName);


  int result=dlg.exec();
  if (result==QDialog::Accepted)
  {
    pFrontName = dlg.frontName();
    pBackName  = dlg.backName();
  }
  return result;
}


// Retrieve selected deck name
QString KCardDialog::backName() const
{
  return d->currentBack;
}


// Retrieve selected card name
QString KCardDialog::frontName() const
{ 
  return d->currentFront;
}

// Build list widget
void KCardDialog::insertCardIcons()
{

    // Clear GUI
    d->ui.frontList->clear();

    // Rebuild list
    QSize itemSize;
    foreach( QString name, CardDeckInfo::frontNames() )
    {
        KCardThemeInfo v = CardDeckInfo::frontInfo( name );
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
    
    // Prevent empty preview
    if (d->useSVGOnly && !CardDeckInfo::isSVGFront(d->currentFront))
        updateFront(CardDeckInfo::defaultFrontName(!d->usePNGOnly, !d->useSVGOnly));
    else if (d->usePNGOnly && CardDeckInfo::isSVGFront(d->currentFront))
        updateFront(CardDeckInfo::defaultFrontName(!d->usePNGOnly, !d->useSVGOnly));
    else
        updateFront(d->currentFront);
}


// Update front preview
void KCardDialog::updateFront()
{
  QList<QListWidgetItem*> l = d->ui.frontList->selectedItems();
  if( !l.isEmpty() )
      updateFront(l.first()->text());
}


// Update front preview

void KCardDialog::updateFront(QString item)
{
  // Clear item?
  if (item.isNull())
  {
    QList<QListWidgetItem*> items = d->ui.frontList->selectedItems();
    if( !items.isEmpty() )
        items.first()->setSelected( false );
    d->ui.frontPreview->setPixmap(QPixmap());
    d->ui.cardName->setText(QString());
    d->ui.cardDescription->setText(QString());
  }
  else
  {
    QList<QListWidgetItem*> items = d->ui.frontList->findItems(item, Qt::MatchExactly );
    if( !items.isEmpty() )
        items.first()->setSelected( true );
    KCardThemeInfo info = CardDeckInfo::frontInfo(item);
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
      // QMap<QString, KCardThemeInfo>::const_iterator it = d->deckInfo.constBegin();
      QString name = CardDeckInfo::defaultBackName(!d->usePNGOnly, !d->useSVGOnly);
      updateBack(name);
    }
  }
  d->currentFront = item;
}


// Retrieve default card directory
QString KCardDialog::getDefaultCardDir(bool pAllowSVG, bool pAllowPNG)
{
  QString name = CardDeckInfo::defaultFrontName(pAllowSVG, pAllowPNG);
  return CardDeckInfo::frontDir(name);
}


// Retrieve default deck file name
QString KCardDialog::getDefaultDeck(bool pAllowSVG, bool pAllowPNG)
{
  QString name = CardDeckInfo::defaultBackName(pAllowSVG, pAllowPNG);
  return CardDeckInfo::backFilename(name);
}


// Update the locking filter
void KCardDialog::updateLocking(int state)
{
  if (state == Qt::Checked)
  {
    d->useLocking = true;
    // Update previews
    updateFront(d->currentFront);
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
void KCardDialog::updateBack()
{
  QList<QListWidgetItem*> l = d->ui.backList->selectedItems();
  if( !l.isEmpty() )
      updateBack(l.first()->text());
}


// Update the back preview
void KCardDialog::updateBack(QString item)
{
  if (item.isNull())
  {
    QList<QListWidgetItem*> items = d->ui.backList->selectedItems();
    if( !items.isEmpty() )
        items.first()->setSelected( false );
    d->ui.backPreview->setPixmap(QPixmap());
  }
  else
  {
    QList<QListWidgetItem*> items = d->ui.backList->findItems(item, Qt::MatchExactly );
    if( !items.isEmpty() )
        items.first()->setSelected( true );
    KCardThemeInfo info = CardDeckInfo::backInfo(item);
    QPixmap pixmap= info.preview;
    if (pixmap.height() > d->ui.backPreview->height())
      pixmap = pixmap.scaledToHeight(d->ui.backPreview->height(), Qt::SmoothTransformation);
    if (pixmap.width() > d->ui.backPreview->width())
      pixmap = pixmap.scaledToWidth(d->ui.backPreview->width(), Qt::SmoothTransformation);
    d->ui.backPreview->setPixmap(pixmap);
  }
  d->currentBack = item;
}


// Insert the deck icons into the list widget
void KCardDialog::insertDeckIcons()
{
    // Clear GUI
    d->ui.backList->clear();

    // Rebuild list
    QSize itemSize;
    foreach( QString name, CardDeckInfo::backNames() )
    {
        KCardThemeInfo v = CardDeckInfo::backInfo( name );
        // Show only SVG files?
        if (d->filterOutCard(v)) continue;
        QPixmap previewPixmap = v.preview.scaled(QSize(32,43), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        QListWidgetItem *item = new QListWidgetItem(v.name, d->ui.backList);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        item->setToolTip(v.name);
        item->setData(Qt::DecorationRole, previewPixmap);
        itemSize = itemSize.expandedTo(previewPixmap.size());
    }
    d->ui.backList->setIconSize(itemSize);
    
    // Prevent empty preview
    if (d->useSVGOnly && !CardDeckInfo::isSVGBack(d->currentBack))
        updateBack(CardDeckInfo::defaultBackName(!d->usePNGOnly, !d->useSVGOnly));
    else if (d->usePNGOnly && CardDeckInfo::isSVGBack(d->currentBack))
        updateBack(CardDeckInfo::defaultBackName(!d->usePNGOnly, !d->useSVGOnly));
    else
        updateBack(d->currentBack);

}

#include "kcarddialog.moc"
