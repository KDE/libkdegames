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
#ifndef __KCARDDIALOG_H_
#define __KCARDDIALOG_H_

#include <QtGui/QWidget>
#include <kdialog.h>
#include <kconfig.h>

#include <libkdegames_export.h>

class QListWidgetItem;
class KCardWidgetPrivate;
class KCardWidget;

/**
 * \class KCardDialog kcarddialog.h <KCardDialog>
 * 
 * @short A convenience class to display a standalone card selection dialog.
 *
 * This is a simple convenience class to embed the @ref KCardWidget into a
 * KDialog that has an Ok and Cancel button and an appropriate caption.
 *
 * Usage Example:
 * \code
 *   KConfigGroup(KGlobal::config(),"CardOptions");
 *   KCardWidget* cardwiget = new KCardwidget();
 *   cardwidget->readSettings(configGroup);
 *   KCardDialog dlg(cardwidget);
 *   if(dlg.exec() == QDialog::Accepted)
 *   {
 *     cardwidget->saveSettings(configGroup);
 *     configGroup.sync();
 *     //now update the graphics
 *   }
 * \endcode
 *
 */
class KDEGAMES_EXPORT KCardDialog : public KDialog
{
  Q_OBJECT
public:
  KCardDialog(KCardWidget* widget);
};

/**
 * \class KCardWidget kcarddialog.h <KCardDialog>
 * 
 * @short A carddeck selection widget for card games.
 *
 * The KCardWidget provides a widget for interactive carddeck selection.
 * It gives cardgames an easy to use interface to select front and
 * back of the card sets. As card sets the KDE default cardsets are
 * offered as well as used specified ones.
 *
 * This class can be used in two ways: Embedding it into an existing
 * dialog or creating a small KDialog just for the carddeck selection.
 *
 * Card sets (front and back) are identified by their (translated) names.
 *
 * Here you can see a card widget in action
 * @image html "kcarddialog.png" KCardwidget
 *
 * You can use a KConfigGroup to initialize the state of the widget or
 * let the widget store its current state to a config group.
 *
 * For an example usage see @ref KCardDialog.
 *
 */
class KDEGAMES_EXPORT KCardWidget : public QWidget
{
  Q_OBJECT

public:

   /**
    * Constructs a card deck selection widget.
    *
    * @param parent The parent widget of the widget, if any.
    */
   KCardWidget (QWidget* parent = NULL);

   /**
    * Read the settings from a config file
    * @param group the configuration group
    */
   void readSettings(const KConfigGroup& group);

   /**
    * Destructs a card deck selection dialog.
    */
   ~KCardWidget();

   /**
    * Saves the KCardWidget config into a config file.
    * These settings are used by @ref KCardWidget.
    */
   void saveSettings(KConfigGroup& group) const;

   /**
    * set the name of the card deck (back side)
    * @param name the new name to select as back side
    */
   void setBackName(const QString& name);

   /**
    * Retrieve the name of the card deck (back side) from the widget.
    * @return The deck name.
    */
   QString backName() const;

   /**
    * Retrieve the locking status
    * @return true if the backside selection is locked to the frontside
    */
   bool isLocked() const;

   /**
    * Retrieve the current state of the "Show old style decks" checkbox
    * @return whether or not fixed size card decks are shown in the list
    */
   bool isFixedSizeAllowed() const;

   /**
    * set the name of the card set (front side)
    * @param name the new name to select as front side
    */
   void setFrontName(const QString& name);

   /**
    * Retrieve the name of the card set (front side) from the dialog.
    * @return The card set name.
    */
   QString frontName() const;

public Q_SLOTS:
   /**
    * Allow the user to select fixed size cards
    * @param fixedSizeAllowed if set to true will show scalable and also fixed
    * size card decks in the dialog
    */
   void setFixedSizeAllowed(bool fixedSizeAllowed);

   /**
    * enable or disable locked selection, when locking is enabled the user
    * can only choose the front sides and the backside will be determined
    * from the frontside
    * @param locked whether to lock the front and backside selection
    */
   void setLocked(bool locked);

protected:
    void insertCardIcons();

    /**
     * Insert the back sides into the list view.
     */
    void insertDeckIcons();

    /**
     * Configure the dialog GUI.
     */
    void setupGUI();

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

private Q_SLOTS:
   /**
    * Private slot used only for an internal connection
    */
    void setNotLocked(bool notLocked);

private:
   /**
    * The dialog data.
    */
   KCardWidgetPrivate* const d;
};

#endif
