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

#define KCARD_DECKPATH QString::fromLatin1("carddecks/decks/")
#define KCARD_DEFAULTDECK QString::fromLatin1("deck0")
#define KCARD_CARDPATH QString::fromLatin1("carddecks/")
#define KCARD_DEFAULTCARD QString::fromLatin1("11")
#define KCARD_DEFAULTCARDDIR QString::fromLatin1("cards1/")
#define KCARD_DEFAULTFILEMASK QString::fromLatin1(".png");


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
   * Constructs a card deck selection dialog.
   *
   * @param parent The parent widget of the dialog, if any.
   * @param name The name of the dialog.
   * @param modal Specifies whether the dialog is modal or not.
   *
   */
   KCardDialog (QWidget* parent = NULL,const char* name = NULL,
             bool modal = true);
   /**
   * Destructs a card deck selection dialog.
   *
   */
   ~KCardDialog();

   /**
   *  @li @p NoDeck - The deck (back) selection is not shown
   *  @li @p NoCards -  The cards (front) selection is not shown
   *  @li @p ProbeDefaultDir - Always probes for the KDE default carddir
   */
   enum CardMask { NoDeck=0x01, NoCards=0x02, ProbeDefaultDir=0x04 };

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
   * @param mFlags An or'ed combination of @p NoDeck, @p NoCards and
   *               @p ProbeDefaultDir 
   *
   * @param mAlternateDeck An optinal path to the directory where the
   *        card decks are stored. If @p ProbeDefaultDir is given and
   *        the default directory is found the default directory is
   *        taken otherwise the alternative one
   *
   * @param  mAlternateCarddir An optinoal path to the directory where
   *        the carddecks are stored (in subdirectories cards1,cards2,...).
   *        It behaves as @p mAlternateDeck otherwise
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
                       int mFlags=0, QString mAlternateDeck=0,
                       QString mAlternateCarddir=0, QString mMask=0, 
                       bool* mRandomDeck=0, bool* mRandomCardDir=0);

   /**
   * Returns the default path to the card deck backsides. You want
   * to use this usually before the user used the card dialog the first
   * time to get a default deck. You can assume that
   * <pre>
   *   getDefaultDeckPath()+QString::fromLatin1("deck0.png")
   * </pre>
   * is a valid deck.
   *
   * @return The default path
   *
   * @param Optional
   *        you can give the flag @p ProbeDefaultDir and an
   *        alternative deckpath. It works as in @ref #getCardDeck, i.e.
   *        the decks are taken from the alternative path if given. If
   *        the probing option is given the default directory is preferred
   *        if existant
   */
   static QString getDefaultDeckPath(int mFlags=0,QString mAlternateDeck=0 );

   /**
   * Returns the default path to the card frontsides. You want
   * to use this usually before the user used the card dialog the first
   * time to get an default deck. You can assume that
   * <pre>
   *   getDefaultCardPath()+QString::fromLatin1("*.png")
   * </pre>
   * are valid cards for * from 1 to 52.
   *
   * @param Optionial you can give the flag @p ProbeDefaultDir and an
   *        alternative deckpath. It works as in @ref #getCardDeck, i.e.
   *        the decks are taken from the alternative path if given. If
   *        the probing option is given the default directory is preferred
   *        if existant
   *
   * @return returns the path to the card directory
   */
   static QString getDefaultCardPath(int mFlags=0,QString mAlternateCarddir=0);

   /**
    * Returns a random deck in @ref deckPath() or mAlternateDeck if given
    * @param mFlags You can assign @p ProbeDefaultDir to this
    * @param mAlternateDeck an alternative dir to search for the decks
    * @param mMask The filemask. See @ref setFileMask() - .png is default
    * @return A random deck
    **/
   static QString getRandomDeck(int mFlags=0, QString mAlternateDeck=0, QString mMask=0);

   /**
    * Returns a random directory of cards in @ref cardPath() or 
    * mAlternateCardDir if given.
    * @param mFlags You can assign @p ProbeDefaultDir to this
    * @param mAlternateDeck An alternative dir to search for the card dirs 
    * @param mMask The filemask. See @ref setFileMask() - .png is default
    * @return A random card dir
    **/
   static QString getRandomCardDir(int mFlags=0, QString mAlternateCardDir=0, QString mMask=0);

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
   void setDeck(QString file);

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
   void setCardDir(QString dir);

   /**
   * Returns the flags set to the dialog
   * 
   * @return the flags
   */
   int flags() const;

   /**
   * Sets the flags for the dialog
   *
   * @param f The flags @p NoDeck, @p NoCards and @p ProbeDefaultDir 
   * 
   */
   void setFlags(int f);

   /**
   * Returns the filemask
   * 
   * @return the filemask
   */
   QString filemask() const;

   /**
   * Sets the filemask
   *
   * @param mask The filemask (default ".png")
   * 
   */
   void setFilemask(QString mask);

   /**
   * Returns the alternative deck directory
   *
   * @return the directory
   * 
   */
   QString alternateDeck() const;

   /**
   * Sets the alternative deck directory
   *
   * @param file The directory where the decks (deck0.png, deck1.png,...)
   *       are stored
   * 
   * @return the alternative deck directory
   */
   void setAlternateDeck(QString file);

   /**
   * Returns the alternative card directory
   * 
   * @return the alternative card directory
   */
   QString alternateCardDir() const;

   /**
   * Sets the alternative card directory
   *
   * @param dir The directory where the card subdirectories as
   *       cards1, cards2, ... are stored
   * 
   */
   void setAlternateCardDir(QString dir);

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

protected:
   QString deckPath() const;
   void setDeckPath(QString path);
   QString cardPath() const;
   void setCardPath(QString path);
   void insertCardIcons(QString path);
   void insertDeckIcons(QString path);


protected slots:
   void slotDeckClicked(QIconViewItem *);
   void slotCardClicked(QIconViewItem *);
   void slotRandomCardDirToggled(bool on);
   void slotRandomDeckToggled(bool on);

   

private:
   QList <QPixmap > pixmapList;
   QLabel *deckLabel;
   QLabel *cardLabel;
   KIconView *deckIconView;
   KIconView *cardIconView;
   QCheckBox* randomDeck;
   QCheckBox* randomCardDir;

   // set/query variables
   QString cDeck,cCardDir,cMask;
   QString cAltDeck,cAltCarddir;
   int cFlags;
   QString cDeckpath,cCardpath;
};
#endif
