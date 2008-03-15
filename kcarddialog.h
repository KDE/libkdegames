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
#ifndef __KCARDDIALOG_H_
#define __KCARDDIALOG_H_

#include <kdialog.h>
#include <kconfig.h>

#include <libkdegames_export.h>

class QListWidgetItem;
class KCardDialogPrivate;

/**
 * @short A carddeck selection dialog for card games.
 *
 * The KCardDialog provides a dialog for interactive carddeck selection.
 * It gives cardgames an easy to use interface to select front and
 * back of the card sets. As card sets the KDE default cardsets are
 * offered as well as used specified ones.
 *
 * In most cases, the simplest
 * use of this class is the static method KCardDialog::getCardDeck,
 * which pops up the dialog, allows the user to select a carddeck, and
 * returns when the dialog is closed. Only if you really need some specific
 * behaviour or if you overwrite the dialog you need all the other access
 * functions.
 *
 * Card sets (front and back) are identified by their (translated) names.
 * Access to further properties of the sets like their filenames or directories
 * is achieved by querying the dialog using static functions where as argument
 * the card deck or set name is given as argument.
 *
 * Example:
 *
 * \code
 *      QString front,back;
 *      int result = KCardDialog::getCardDeck(front, back);
 *      if ( result == KCardDialog::Accepted )
 *            ...
 * \endcode
 *
 * Retrieve the filename and information for the card back (deck):
 *
 * \code
 *      QString front,back;
 *      int result       = KCardDialog::getCardDeck(front, back);
 *      ...
 *      QString deckFile = KCardDialog::deckSVGFilePath(back);
 *      bool    isSVG    = KCardDialog::isSVGDeck(back);
 *      ...
 * \endcode
 *
 *
 * Retrieve the SVG information for the card set (front):
 *
 * \code
 *      QString front,back;
 *      int result       = KCardDialog::getCardDeck(front, back);
 *      ...
 *      QString cardFile = KCardDialog::cardSVGFilePath(front);
 *      bool    isSVG    = KCardDialog::isSVGCards(front);
 *      ...
 * \endcode
 *
 * Here you can see a card dialog in action
 * @image html "kcarddialog.png" KCarddialog
 *
 * KCardDialog::getCardDeck takes additionalparameters for custumization.
 * You can e.g. retrieve a random card deck, limit the choice to scalable or
 * fixed size decks, or lock the front and backside together.
 *
 * You can also provide a KConfigGroup reference to the dialog. This
 * is used to store information about the dialog. 
 *
 */
class KDEGAMES_EXPORT KCardDialog : public KDialog
{
  Q_OBJECT

public:

   /**
    * Constructs a card deck selection dialog.
    *
    * @param parent    The parent widget of the dialog, if any.
    * @param pAllowSVG Allow selection of scalable cards sets.
    * @param pAllowPNG Allow selection of fixed size cards sets.
    * @param pLock     Back side is set to match the front side of cards.
    * @param defFront  Default front side name.
    * @param defBack   Default back side name.
    */
   explicit KCardDialog (QWidget* parent = NULL, 
                         bool pAllowSVG = true, 
                         bool pAllowPNG = true, 
                         bool pLock = true,
                         QString defFront = QString(),
                         QString defBack = QString()
                         );

   /**
    * Constructs a card deck selection dialog.
    *
    * @param group  The configuration group for dialog
    * @param parent The parent widget of the dialog, if any.
    */
   explicit KCardDialog (KConfigGroup& group, QWidget* parent = NULL);

   /**
    * Destructs a card deck selection dialog.
    */
   ~KCardDialog();

   /**
    * Creates a modal carddeck dialog, lets the user choose a deck,
    * and returns when the dialog is closed.
    *
    * @param pFrontName A reference to the card front side name. Holds the
    *                   result of the user choice.
    *
    * @param pBackName  A reference to the card back side name. Holds the
    *                   result of the user choice.
    *
    * @param pParent    Pointer to the parent window of the dialog
    * @param pAllowSVG  Allow selection of scalable cards sets.
    * @param pAllowPNG  Allow selection of fixed size cards sets.
    * @param pLock      Back side is set to match the front side of cards.
    * @param pRandom    Return a random selection for front and back side.
    *
    * @return QDialog::result().
    */
   static int getCardDeck(QString &pFrontName,
                          QString &pBackName,
                          QWidget *pParent = NULL,
                          bool pAllowSVG = true,
                          bool pAllowPNG = true,
                          bool pLock = true, 
                          bool pRandom = false);

   /**
    * Convenience function, works like the above function.
    *
    * Creates a modal carddeck dialog, reading the settings from the
    * config @p group.
    *
    * @param group the KConfigGroup to read the settings from.
    * @return QDialog::result();
    */
   static int getCardDeck(const KConfigGroup& group);

   /**
    * Saves the KCardDialog config into a config file. 
    * These settings are used by @ref KCardDialog.
    */
   void saveSettings(KConfigGroup& group);

   /**
    * Retrieve the name of the card deck (back side) from the dialog.
    * @return The deck name.
    */
   QString backName() const;

   /**
    * Retrieve the name of the card set (front side) from the dialog.
    * @return The card set name.
    */
   QString frontName() const;

   /**
    * Retreive the default card directory.
    * @deprecated KDE 3.x compatibilty
    */
   static QString getDefaultCardDir(bool pAllowSVG = true, bool pAllowPNG = true);

   /**
    * Retrieve the default deck filename.
    * @deprecated KDE 3.x compatibilty
    */
   static QString getDefaultDeck(bool pAllowSVG = true, bool pAllowPNG = true);


protected:
    void insertCardIcons();
    
    /**
     * Insert the back sides into the list view.
     */
    void insertDeckIcons();
    
    /**
     * Update the back side preview image.
     * @param item The name of the deck.
     */
    void updateBack(QString item);

    /**
     * Update the front side preview image.
     * @param item The name of the card set.
     */
    void updateFront(QString item);
    
    /**
     * Configure the dialog GUI.
     */
    void setupGUI();

   /** Retrieve the filename of the PNG file for the backside of a deck.
    * diven the index.desktop filename.
    * @param desktop The name of the index.desktop file.
    * @return The name of the PNG file.
    */
    static QString getDeckFileNameFromIndex(const QString& desktop);

protected Q_SLOTS:
    /**
     * Called by the card set list view when a new item was selected.
     */
    void updateFront();
    
     /**
     * Called by the card deck list view when a new item was selected.
     * @param current The newly selected item.
     * @param last    The previously selected item.
     */  
    void updateBack();
    
    /**
     * Called by the checkboxes when the state of the locking changed.
     * @param state The new locking state.
     */
    void updateLocking(int state);

    /**
     * Called by the checkboxes when the state of the SVG filter changed.
     * @param state The new SVG filter state.
     */
    void updateSVG(int state);
    
    /**
     * Called by the checkboxes when the state of the PNG filter changed.
     * @param state The new PNG filter state.
     */
    void updatePNG(int state);

private:
   /**
    * The dialog data.
    */
   KCardDialogPrivate* const d;
};

#endif
