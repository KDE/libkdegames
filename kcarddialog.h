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
#ifndef __KCARDDIALOG_H_
#define __KCARDDIALOG_H_

#include <KDialog>

#include <libkdegames_export.h>
#include <kglobal.h>

class QListWidgetItem;

class KConfig;

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
 *      int result = KCardDialog::getCardDeck(deck,card );
 *      if ( result == KCardDialog::Accepted )
 *            ...
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
 * @author Martin Heni <martin@heni-online.de>
 * @version $Id$
 */
class KDEGAMES_EXPORT KCardDialog : public KDialog
{
  Q_OBJECT

public:

  /**
   *  @li @p Both - both are shown
   *  @li @p NoDeck - The deck (back) selection is not shown
   *  @li @p NoCards - The cards (front) selection is not shown
   */
   enum CardFlags { Both=0, NoDeck=0x01, NoCards=0x02 };

   /**
   * Constructs a card deck selection dialog.
   *
   * @param parent The parent widget of the dialog, if any.
   * @param flags Specifies whether the dialog is modal or not.
   */
   KCardDialog (QWidget* parent = NULL, CardFlags flags = Both);
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
   static int getCardDeck(QString &deck,QString &carddir, QWidget *parent=0,
                          CardFlags flags=Both, bool* randomDeck=0,
                          bool* randomCardDir=0, double* scale=0, KConfig* conf=0);

   /**
    * Read the configuration from the applications rc file and put the
    * previously chosen deck/frontside in the parameter deck and carddir.
    *
    * You probably want to use this function on startup of your program so that
    * the user gets exactly the card/frontside he/she chose before. Note that
    * you don't have to care whether the user wants to get a random carddeck or
    * not as this function takes care of this.
    * @param conf The config file to read from
    * @param deck This will contain the chosen deck from the config file (or a
    * random deck if this is desired according to the config)
    * @param cardDir This will contain the chosen cardDir from the config file (or a
    * random cardDir if this is desired according to the config)
    * @param scale The scaling factor (usually 1)
    **/
   static void getConfigCardDeck(KConfig* conf, QString& deck, QString& cardDir, double& scale);

   /**
   * Returns the default path to the card deck backsides. You want
   * to use this usually before the user used the card dialog the first
   * time to get a default deck. You can assume that
   * \code
   *   getDefaultDeckPath()
   * \endcode
   * is a valid deck.
   *
   * @return The default path
   */
   static QString getDefaultDeck();

   /**
   * Returns the default path to the card frontsides. You want
   * to use this usually before the user used the card dialog the first
   * time to get an default deck. You can assume that
   * \code
   *   getCardPath(getDefaultCardPath(), *)
   * \endcode
   * are valid cards for * from 1 to 52.
   *
   * @return returns the path to the card directory
   */
   static QString getDefaultCardDir();

    /**
    * Returns the path to the card frontside specified in dir carddir
    *
    * @param index the card to open
    * @param carddir The carddir which's path shall be searched for
    * @return returns the path to the card
    */
    static QString getCardPath(const QString &carddir, int index);

   /**
    * Returns a random deck in deckPath()
    * @return A random deck
    **/
    static QString getRandomDeck();

   /**
    * Returns a random directory of cards
    * @return A random card dir
    **/
    static QString getRandomCardDir();

   /**
    * Show or hides the "random backside" checkbox
    * @param s Shows the checkbox if true otherwise hides it
    **/
   void showRandomDeckBox(bool s);

   /**
    * Show or hides the "random foreside" checkbox
    * @param s Shows the checkbox if true otherwise hides it
    **/
   void showRandomCardDirBox(bool s);

   /**
   * Returns the chosen deck, which is a valid path to a imagefile.
   *
   * @return The deck
   */
   const QString& deck() const;

   /**
   * Sets the default deck.
   * @param file The full path to an image file
   */
   void setDeck(const QString& file);

   /**
   * @return The chosen card directory
   */
   const QString& cardDir() const;

   /**
   * Sets the default card directory.
   * @param dir The full path to an card directory
   */
   void setCardDir(const QString& dir);

   /**
   * @return the flags set to the dialog
   */
   CardFlags flags() const;

   /**
   * Creates the default widgets in the dialog. Must be called after
   * all flags are set. This is only needed if you do NOT use the
   * getCardDeck static function which provides all calls for you.
   */
   void setupDialog(bool showResizeBox = false);

   /**
    * @return TRUE if the selected deck is a random deck (i.e. the user checked
    * the random checkbox) otherwise FALSE
    **/
   bool isRandomDeck() const;

   /**
    * @return TRUE if the selected carddir is a random dir (i.e. the user
    * checked the random checkbox) otherwise FALSE
    **/
   bool isRandomCardDir() const;

   /**
    * @return TRUE if the global checkbox was selected
    **/
   bool isGlobalDeck() const;

   /**
    * @return TRUE if the global checkbox was selected
    **/
   bool isGlobalCardDir() const;

   /**
    * @return The scaling factor of the card pixmap
    **/
   double cardScale() const;

   /**
    * Load the default settings into the dialog (e.g. whether the "use random
    * deck" checkbox is checked or not).
    **/
   void loadConfig(KConfig* conf);

   /**
    * Saves the KCardDialog config into a config file. This should be the
    * applications config file - KCardDialog creates an own group
    * ("KCardDialog"). These settings are used by @ref loadConfig and @ref
    * getConfigCardDeck.
    **/
   void saveConfig(KConfig* conf);


protected:
    void insertCardIcons();
    void insertDeckIcons();

    static void getGlobalDeck(QString& cardDir, bool& random);
    static void getGlobalCardDir(QString& deck, bool& random);

    static QString getDeckName(const QString& desktop);

    /**
     * @return the groupname used by functions like @ref saveConfig and @ref
     * loadConfig.
     **/
    static QString group();

protected Q_SLOTS:
   void slotDeckClicked(QListWidgetItem *);
   void slotCardClicked(QListWidgetItem *);
   void slotRandomCardDirToggled(bool on);
   void slotRandomDeckToggled(bool on);
   void slotCardResized(int);
   void slotDefaultSize();
   void slotSetGlobalDeck();
   void slotSetGlobalCardDir();

private:
   static void init();

   KCardDialogPrivate* d;
};

#endif
