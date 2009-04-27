/*
    This file is part of the KDE games library
    Copyright 2008 Andreas Pakulat <apaku@gmx.de>

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

#include <QPainter>
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


/**
 * Local information of the dialog.
 */
class KCardWidgetPrivate
{
public:

  /** Constructor.
   */
  KCardWidgetPrivate()
  {
  }

  /** Filter a fiven card front/back depending on its scalable
    * or non-scalable properties.
    * @param v The card info structure.
    * @return True if the card should bve discarded.
    */
  bool filterOutCard(const KCardThemeInfo& v)
  {
    return !fixedSizeAllowed && v.svgfile.isEmpty();
  }

  /** Currently chosen back side name.
   */
  QString currentBack;

  /** Currently chosen front side name.
   */
  QString currentFront;

  /** Determines if PNG based decks should be shown
   */
  bool fixedSizeAllowed;

  /** The UI elements.
   */
  Ui::KGameCardSelectorBase ui;

};

// Create the dialog from a config group
KCardWidget::KCardWidget(QWidget* parent)
           : QWidget(parent), d(new KCardWidgetPrivate)
{
  // GUI
  setupGUI();
  setLocked(true);
  setFixedSizeAllowed(false);
  insertCardIcons();
  insertDeckIcons();
  setFrontName(CardDeckInfo::defaultFrontName(false));
  setBackName(CardDeckInfo::defaultBackName(false));
}

void KCardWidget::readSettings(const KConfigGroup& group)
{
  setLocked(CardDeckInfo::lockFrontToBackside(group));
  setFrontName(CardDeckInfo::frontTheme(group));
  setBackName(CardDeckInfo::backTheme(group));
}

// Store the config group settings
void KCardWidget::saveSettings(KConfigGroup& group) const
{
  CardDeckInfo::writeLockFrontToBackside(group, !d->ui.backGroupBox->isChecked());
  CardDeckInfo::writeFrontTheme(group, d->currentFront);
  CardDeckInfo::writeBackTheme(group, d->currentBack);
}


// Setup the user interface
void KCardWidget::setupGUI()
{
  // Inner widget
  Ui::KGameCardSelectorBase* ui = &(d->ui);
  d->ui.setupUi(this);

  // Set lists and preview
  insertCardIcons();
  insertDeckIcons();

  // Connect signals
  connect(ui->frontList, SIGNAL(itemSelectionChanged()),
          this, SLOT(updateFront()));
  connect(ui->backList, SIGNAL(itemSelectionChanged()),
          this, SLOT(updateBack()));
  connect(ui->backGroupBox, SIGNAL(toggled(bool)), this, SLOT(setNotLocked(bool)));

  // Debug
  // kDebug() << "DEFAULT DECK: " << defaultDeckName(pAllowSVG, pAllowPNG);
  // kDebug() << "DEFAULT CARD: " << defaultCardName(pAllowSVG, pAllowPNG);
  // kDebug() << "RANDOM DECK: " << randomDeckName(pAllowSVG, pAllowPNG);
  // kDebug() << "RANDOM CARD: " << randomCardName(pAllowSVG, pAllowPNG);
}


// Destroy the dialog
KCardWidget::~KCardWidget()
{
  delete d;
}

// Retrieve selected deck name
QString KCardWidget::backName() const
{
  return d->currentBack;
}


// Retrieve selected card name
QString KCardWidget::frontName() const
{
  return d->currentFront;
}

bool KCardWidget::isFixedSizeAllowed() const
{
  return d->fixedSizeAllowed;
}

bool KCardWidget::isLocked() const
{
  return !d->ui.backGroupBox->isChecked();
}

// Build list widget
void KCardWidget::insertCardIcons()
{
  // Clear GUI
  d->ui.frontList->clear();

  // Rebuild list
  QSize itemSize;
  foreach(const QString &name, CardDeckInfo::frontNames())
  {
    KCardThemeInfo v = CardDeckInfo::frontInfo(name);
    // Show only SVG files?
    if (d->filterOutCard(v)) continue;

    const int iconSize = 48;
    QPixmap resizedCard = v.preview.scaled(QSize(iconSize, iconSize), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPixmap previewPixmap(iconSize, iconSize);
    previewPixmap.fill(Qt::transparent);
    QPainter p(&previewPixmap);
    p.drawPixmap((iconSize-resizedCard.width())/2, (iconSize-resizedCard.height())/2, resizedCard);
    p.end();

    QListWidgetItem *item = new QListWidgetItem(v.name, d->ui.frontList);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    item->setToolTip(v.name);
    item->setData(Qt::DecorationRole, previewPixmap);
    item->setData(Qt::UserRole, v.noi18Name);
    itemSize = itemSize.expandedTo(previewPixmap.size());
  }

  if(!isFixedSizeAllowed() && !CardDeckInfo::isSVGFront(d->currentFront))
  {
    setFrontName(CardDeckInfo::defaultFrontName(isFixedSizeAllowed()));
  }else
  {
    setFrontName(d->currentFront);
  }
  d->ui.frontList->setIconSize(itemSize);
}


// Update front preview
void KCardWidget::updateFront()
{
  QList<QListWidgetItem*> l = d->ui.frontList->selectedItems();
  if(!l.isEmpty())
      setFrontName(l.first()->data(Qt::UserRole).toString());
}


// Update front preview

void KCardWidget::setFrontName(const QString& name)
{
  // Clear item?
  if (name.isEmpty())
  {
    QList<QListWidgetItem*> items = d->ui.frontList->selectedItems();
    if(!items.isEmpty())
        items.first()->setSelected(false);
    d->ui.frontPreview->setPixmap(QPixmap());
    d->ui.cardName->setText(QString());
    d->ui.cardDescription->setText(QString());
  }
  else
  {
    for (int i = 0; i < d->ui.frontList->count(); ++i)
    {
      QListWidgetItem *item = d->ui.frontList->item(i);
      if (item->data(Qt::UserRole).toString() == name)
      {
        item->setSelected(true);
        d->ui.frontList->scrollToItem(item);
        break;
      }
    }

    KCardThemeInfo info = CardDeckInfo::frontInfo(name);
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
    if (isLocked() && !info.back.isEmpty())
    {
      setBackName(info.back);
    }
    else if (isLocked())
    {
      // QMap<QString, KCardThemeInfo>::const_iterator it = d->deckInfo.constBegin();
      QString name = CardDeckInfo::defaultBackName(isFixedSizeAllowed());
      setBackName(name);
    }
  }
  d->currentFront = name;
}

// Update the locking filter
void KCardWidget::setLocked(bool locked)
{
  d->ui.backGroupBox->setChecked(!locked);
  if (locked)
  {
    // Update previews
    setFrontName(d->currentFront);
  }
  d->ui.backList->setEnabled(!locked);
}

// An ugly internal slot needed for UI connections
void KCardWidget::setNotLocked(bool notLocked)
{
    setLocked(!notLocked);
}


// Update the PNG status filter
void KCardWidget::setFixedSizeAllowed(bool allowFixedSize)
{
    if ( allowFixedSize != d->fixedSizeAllowed )
    {
        d->fixedSizeAllowed = allowFixedSize;
        insertCardIcons();
        insertDeckIcons();
    }
}


// Update the back preview
void KCardWidget::updateBack()
{
  QList<QListWidgetItem*> l = d->ui.backList->selectedItems();
  if(!l.isEmpty())
    setBackName(l.first()->data(Qt::UserRole).toString());
}


// Update the back preview
void KCardWidget::setBackName(const QString& item)
{
  if (item.isEmpty())
  {
    QList<QListWidgetItem*> items = d->ui.backList->selectedItems();
    if(!items.isEmpty())
      items.first()->setSelected(false);
    d->ui.backPreview->setPixmap(QPixmap());
  }
  else
  {
    for (int i = 0; i < d->ui.backList->count(); ++i)
    {
      QListWidgetItem *lwi = d->ui.backList->item(i);
      if (lwi->data(Qt::UserRole).toString() == item)
      {
        lwi->setSelected(true);
        d->ui.backList->scrollToItem(lwi);
        break;
      }
    }
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
void KCardWidget::insertDeckIcons()
{
  // Clear GUI
  d->ui.backList->clear();

  // Rebuild list
  QSize itemSize;
  foreach(const QString &name, CardDeckInfo::backNames())
  {
    KCardThemeInfo v = CardDeckInfo::backInfo(name);
    // Show only SVG files?
    if (d->filterOutCard(v) || v.preview.isNull() ) continue;

    const int iconSize = 48;
    QPixmap resizedCard = v.preview.scaled(QSize(iconSize, iconSize), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPixmap previewPixmap(iconSize, iconSize);
    previewPixmap.fill(Qt::transparent);
    QPainter p(&previewPixmap);
    p.drawPixmap((iconSize-resizedCard.width())/2, (iconSize-resizedCard.height())/2, resizedCard);
    p.end();

    QListWidgetItem *item = new QListWidgetItem(v.name, d->ui.backList);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    item->setToolTip(v.name);
    item->setData(Qt::DecorationRole, previewPixmap);
    item->setData(Qt::UserRole, v.noi18Name);
    itemSize = itemSize.expandedTo(previewPixmap.size());
  }
  d->ui.backList->setIconSize(itemSize);

  // Prevent empty preview
  if (!isFixedSizeAllowed() && !CardDeckInfo::isSVGBack(d->currentBack))
    setBackName(CardDeckInfo::defaultBackName(isFixedSizeAllowed()));
  else
    setBackName(d->currentBack);
}

KCardDialog::KCardDialog( KCardWidget* widget )
{
  setMainWidget(widget);
  setCaption(i18n("Card Deck Selection"));
  setButtons(KDialog::Ok | KDialog::Cancel);
}

#include "kcarddialog.moc"
