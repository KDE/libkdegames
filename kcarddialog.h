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
 * Example:
 *
 * \code
 *      QString deck,card;
 *      int result = KCardDialog::getCardDeck(deck, card);
 *      if ( result == KCardDialog::Accepted )
 *            ...
 * \endcode
 *
 * Retrieve the SVG information for the card back (deck):
 *
 * \code
 *      QString deck,card;
 *      int result       = KCardDialog::getCardDeck(deck, card);
 *      ...
 *      QString deckFile = KCardDialog::deckSVGFilePath(deck);
 *      bool    isSVG    = KCardDialog::isSVGDeck(deck);
 *      ...
 * \endcode
 *
 *
 * Retrieve the SVG information for the card set (front):
 *
 * \code
 *      QString deck,card;
 *      int result       = KCardDialog::getCardDeck(deck, card);
 *      ...
 *      QString cardFile = KCardDialog::cardSVGFilePath(card);
 *      bool    isSVG    = KCardDialog::isSVGCards(card);
 *      ...
 * \endcode
 *
 * Here you can see a card dialog in action
 * @image html "kcarddialog.png" KCarddialog
 *
 * KCardDialog::getCardDeck takes a lot of different parameters which are
 * probably very useful. You can e.g. use the parameters randomDeck and
 * randomCardDir to give the end-user the ability to choose a random
 * deck/carddir. You have to save the value of those parameters in your config
 * file - that's why the parameters are needed. 
 *
 * You can also provide a KConfig pointer (usually KGlobal::config()). This
 * pointer is used to store information about the dialog in an own group
 * ("KCardDailog"). 
 * So you can just ignore the randomCardDir and randomDeck
 * values and call KCardDialog::getConfigCardDeck instead. The only reson
 * for this function is to read a previously written configuration and give you
 * the information about it. This way you don't have to save any configuration
 * on your own - KCardDialog does this for you.
 *
 * Another Parameter for KCardDialog::getCardDeck is scale. This pointer
 * to a double variable contains the scaling factor the user has chosen in the
 * dialog (the scale box won't be shown if you don't provide this parameter).
 * You might want to check out QPixmap::xFrom which gives you access to
 * scaling. You can e.g. use
 * \code
 * QMatrix m;
 * m.scale(s,s);
 * pixmap.transformed(m);
 * \endcode
 * to scale your pixmap.
 *
 */
class KDEGAMES_EXPORT KCardDialog : public KDialog
{
  Q_OBJECT

public:

   /**
   * Constructs a card deck selection dialog.
   *
   * @param parent The parent widget of the dialog, if any.
   * @param flags Specifies whether the dialog is modal or not.
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
   * @param parent The parent widget of the dialog, if any.
   * @param aconfig The configuration for the parameter
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
   * @param deck a reference to the filename used as backside of the
   *        cards. It is an absolute path and can directly be loaded as
   *        pixmap.
   *
   * @param carddir a reference to the directory name used as front of the
   *        cards. The directory contains the card images as 1.png to 52.png
   *
   * @param parent an optional pointer to the parent window of the dialog
   *
   * @param flags what to show
   *
   * @param randomDeck if this pointer is non-zero, *ok is set to TRUE if
   *        the user wants a random deck otherwise to FALSE. Use this in the
   *        config file of your game to load a random deck on startup.
   *        See @ref getRandomDeck()
   *
   * @param randomCardDir if this pointer is non-zero, *ok is set to TRUE if
   *        the user wants a random card otherwise to FALSE.
   *        Use this in the config file of your game to load a random card
   *        foregrounds on startup.
   *        See @ref getRandomCardDir()
   *
   * @param scale If non-zero a box is shown which provides the possibility to
   *        change the size of the cards. The desired scaling factor is returned to the
   *        game in this variable.
   *
   * @param conf If non-zero KCardDialog reads the initial settings for 
   *        this dialog from the applications config file and stores them there
   *        when the dialog is closed. You can just use getConfigCardDeck
   *        to get the deck/carddir the user selected before. Note that the 
   *        parameters randomDeck and randomCardDir overwrite the initial settings from the
   *        config file.
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
    * Saves the KCardDialog config into a config file. This should be the
    * applications config file - KCardDialog creates an own group
    * ("KCardDialog"). These settings are used by @ref loadConfig and @ref
    * getConfigCardDeck.
    **/
   void saveSettings(KConfigGroup& group);


   /** Retrieve the SVG file belonging to the given card back deck. 
     * @param name The name of the back deck.
     * @return The file name and path to the SVG file or QString() if not available. 
     */
   static QString deckSVGFilePath(const QString& name);

   /** Retrieve the SVG file belonging to the given card set. 
     * The SVG IDs used for the card back is '1_club' for Ace of clubs, '10_spade' for
     * 10 of spades, 'queen_heart' for Queen of Hearts, '2_diamond' for 2 of diamonds and
     * so on.
     * @param name The name of the card set.
     * @return The file name and path to the SVG file or QString() if not available. 
     */
   static QString cardSVGFilePath(const QString& name);

   /** Check whether the card set is SVG or not.
     * @param name The name of the card set.
     * @return True if SVG data is available.
     */
   static bool isSVGCard(const QString& name);

   /** Check whether the card back deck contains also an SVG file.
     * The deck is the complete file path to the deck (directory and filename).
     * @return True if SVG data is available.
     */
   static bool isSVGDeck(const QString& name);
   static QString defaultCardName(bool pAllowSVG = true, bool pAllowPNG = true);
   static QString defaultDeckName(bool pAllowSVG = true, bool pAllowPNG = true);
   static QString randomCardName(bool pAllowSVG = true, bool pAllowPNG = true);
   static QString randomDeckName(bool pAllowSVG = true, bool pAllowPNG = true);
   static QString cardDir(const QString& name);
   static QString deckFilename(const QString& name);

   QString deckName() const;
   QString cardName() const;

   static void reset();

   // Compat KDE 3.x
   static QString getDefaultCardDir(bool pAllowSVG = true, bool pAllowPNG = true);
   static QString getDefaultDeck(bool pAllowSVG = true, bool pAllowPNG = true);



protected:
    static void readFronts();
    static void readBacks();
    void insertCardIcons();
    void insertDeckIcons();
    void updateBack(QString item);
    void updateFront(QString item);
    void setupGUI();
    static QString findi18nBack(QString& name);


    /**
     * Retrieve the filename of the PNG file for the backside of a deck.
     * diven the desktop filename.
     * @param desktop The name of the index.desktop file.
     * @return The name of the PNG file.
     */
    static QString getDeckFileNameFromIndex(const QString& desktop);

    /**
     * @return the groupname used by functions like @ref saveConfig and @ref
     * loadConfig.
     **/
    static QString group();

protected Q_SLOTS:
    void updateFront(QListWidgetItem*, QListWidgetItem * );
    void updateBack(QListWidgetItem*, QListWidgetItem * );
    void updateLocking(int state);
    void updateSVG(int state);
    void updatePNG(int state);

private:
   static void init();

   KCardDialogPrivate* const d;
};

#endif
