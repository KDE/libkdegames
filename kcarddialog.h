/***************************************************************************
                           KCardDialog
                           -------------------
    begin                : 30 October 2000
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
#ifndef __KCARDDLG_H_
#define __KCARDDLG_H_

#include <qstring.h>
#include <kdialogbase.h>

class QCheckBox;
class QIconViewItem;
class QLabel;
class KIconView;



/**
 * The KCardDialog provides a dialog for interactive carddeck selection.
 * It gives cardgames an easy to use interface to select front and
 * back of the card sets. As card sets the KDE default cardsets are
 * offered as well as used specified ones.
 *
 * In most cases, the simplest
 * use of this class is the static method @ref KCardDialog::getCardDeck(),
 * which pops up the dialog, allows the user to select a carddeck, and
 * returns when the dialog is closed. Only if you really need some specific
 * behaviour or if you overwrite the dialog you need all the other access
 * functions.
 *
 * Example:
 *
 * <pre>
 *      QString deck,card;
 *      int result = KCardDialog::getCardDeck(deck,card );
 *      if ( result == KCardDialog::Accepted )
 *            ...
 * </pre>
 *
 * Here you can see a card dialog in action
 * @image kcarddialog.png KCarddialog
 *
 * @short A carddeck selection dialog for card games.
 * @author Martin Heni <martin@heni-online.de>
 * @version $Id$
 */
class KCardDialog : public KDialogBase
{
  Q_OBJECT

public:

  /**
   *  @li @p Both - both are shown
   *  @li @p NoDeck - The deck (back) selection is not shown
   *  @li @p NoCards -  The cards (front) selection is not shown
   */
   enum CardFlags { Both=0, NoDeck=0x01, NoCards=0x02 };

   /**
   * Constructs a card deck selection dialog.
   *
   * @param parent The parent widget of the dialog, if any.
   * @param name The name of the dialog.
   * @param modal Specifies whether the dialog is modal or not.
   *
   */
   KCardDialog (QWidget* parent = NULL,const char* name = NULL,
                CardFlags mFlags = Both);
   /**
   * Destructs a card deck selection dialog.
   *
   */
   ~KCardDialog();

   /**
   * Creates a modal carddeck dialog, lets the user choose a deck,
   * and returns when the dialog is closed.
   *
   * @param mDeck a reference to the filename used as backside of the
   *        cards. It is an absolute path and can directly be loaded as
   *        pixmap.
   *
   * @param mCarddir a reference to the directory name used as front of the
   *        cards. The directory contains the card images as 1.png to 52.png
   *
   * @param mParent an optional pointer to the parent window of the dialog
   *
   * @param mFlags what to show
   *
   * @param mMask An optinonal filemask for the icons. Per default *.png
   *        images are assumed.
   *
   * @param mRandomDeck if this pointer is non-zero, *ok is set to TRUE if
   *        the user wants a random deck otherwise to FALSE. Use this in the
   *        config file of your game to load a random deck on startup.
   *        See @ref getRandomDeck()
   *
   * @param mRandomCardDir if this pointer is non-zero, *ok is set to TRUE if
   *        the user wants a random card otherwise to FALSE.
   *        Use this in the config file of your game to load a random card
   *        foregrounds on startup.
   *        See @ref getRandomCardDir()
   *
   * @return @ref #QDialog::result().
   */
   static int getCardDeck(QString &mDeck,QString &mCarddir, QWidget *mParent=0,
                          CardFlags mFlags=Both, bool* mRandomDeck=0,
                          bool* mRandomCardDir=0);

   /**
   * Returns the default path to the card deck backsides. You want
   * to use this usually before the user used the card dialog the first
   * time to get a default deck. You can assume that
   * <pre>
   *   getDefaultDeckPath()
   * </pre>
   * is a valid deck.
   *
   * @return The default path
   *
   */
   static QString getDefaultDeck();

   /**
   * Returns the default path to the card frontsides. You want
   * to use this usually before the user used the card dialog the first
   * time to get an default deck. You can assume that
   * <pre>
   *   getCardPath(getDefaultCardPath(), *)
   * </pre>
   * are valid cards for * from 1 to 52.
   *
   * @return returns the path to the card directory
   */
   static QString getDefaultCardDir();

    /**
    * Returns the path to the card frontside in dir @param carddir
    *
    * @param index the card to open
    *
    * @return returns the path to the card
    */
    static QString getCardPath(const QString &carddir, int index);

   /**
    * Returns a random deck in @ref deckPath()
    * @return A random deck
    **/
    static QString getRandomDeck();

   /**
    * Returns a random directory of cards in @ref cardPath()
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
   * Returns the choosen deck, which is a valid path to a imagefile.
   *
   * @return The deck
   */
   QString deck() const;

   /**
   * Sets the default deck.
   *
   * @param file The full path to an image file
   */
   void setDeck(const QString& file);

   /**
   * Returns the choosen card directory
   *
   * @return The card directory
   */
   QString cardDir() const;

   /**
   * Sets the default card directory.
   *
   * @param dir The full path to an card directory
   *
   */
   void setCardDir(const QString& dir);

   /**
   * Returns the flags set to the dialog
   *
   * @return the flags
   */
    CardFlags flags() const { return cFlags; }

   /**
   * Creates the default widgets in the dialog. Must be called after
   * all flags are set. This is only needed if you do NOT use the
   * @ref #getCardDeck static function which provides all calls for you.
   */
   void setupDialog();

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

    static void init();

protected:
    QString cardPath() const;
    void setCardPath(const QString& path);
    void insertCardIcons(const QString& path);
    void insertDeckIcons();

    static QString getDeckName(const QString& desktop);

protected slots:
   void slotDeckClicked(QIconViewItem *);
   void slotCardClicked(QIconViewItem *);
   void slotRandomCardDirToggled(bool on);
   void slotRandomDeckToggled(bool on);

private:
    QLabel *deckLabel;
    QLabel *cardLabel;
    KIconView *deckIconView;
    KIconView *cardIconView;
    QCheckBox* randomDeck;
    QCheckBox* randomCardDir;
    QMap<QIconViewItem*, QString> deckMap;

   // set/query variables
   QString cDeck, cCardDir;
   CardFlags cFlags;
   QString cDeckpath,cCardpath;
};

#endif
