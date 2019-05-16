/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2007 Gael de Chalendar (aka Kleag) <kleag@free.fr>

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
#ifndef __KCHATBASEMODEL_H__
#define __KCHATBASEMODEL_H__

#include <QAbstractListModel>
#include <QPair>
#include <QLoggingCategory>

#include "libkdegamesprivate_export.h"

Q_DECLARE_LOGGING_CATEGORY(GAMES_LIB)

class KChatBaseModelPrivate;
class KChatBaseMessagePrivate;
class KConfig;

/**
  * \class KChatBaseMessage kchatbasemodel.h <KChatBaseModel>
  *
  * @short The class of the elements stored in the chat list model
  *
  * It's a pair of strings where the first element is the sender name and the
  * second one is the actual message. It furthermore indicates the type of the
  * message: normal or system
  */
class KDEGAMESPRIVATE_EXPORT KChatBaseMessage : public QPair< QString, QString >
{
  public:
    /** The different types of messages */
    enum MessageType
    {
      Normal,
      System
    };

    /** Default constructor. Necessary for Qt metatypes */
    KChatBaseMessage();

    /** Initializing constructor */
    KChatBaseMessage(const QString& sender, const QString& message,
          MessageType type=Normal);

    /** Copy constructor. Necessary for Qt metatypes */
    KChatBaseMessage(const KChatBaseMessage& m);

    /** Default destructor */
    virtual ~KChatBaseMessage();

  private:
    KChatBaseMessagePrivate* d;
};
Q_DECLARE_METATYPE(KChatBaseMessage)

/**
 * \class KChatBaseModel kchatbasemodel.h <KChatBaseModel>
 *
 * The model used to store messages displayed in the chat dialog messages
 * list. This is a list model and thus derived from @ref QAbstractListModel
 * and implementing its abstract API.
 */
class KDEGAMESPRIVATE_EXPORT KChatBaseModel : public QAbstractListModel
{
  Q_OBJECT

public:
  /** Default constructor */
  explicit KChatBaseModel(QObject *parent = nullptr);

  /** Default destructor */
  ~KChatBaseModel() override;

  /**
    * Reimplementation of the inherited method.
    * @return The current number of messages in the list
    */
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;


  /**
    * Reimplementation of the inherited method.
    * @return The KChatBaseMessage at the given index as a QVariant
    */
  QVariant data(const QModelIndex &index, int role) const override;

  /**
   * Set the font that is used for the name part of a message. See also
   * nameFont and setBothFont
   **/
  void setNameFont(const QFont& font);

  /**
   * Set the font that is used for the message part of a message.
   * @see messageFont, setBothFont
   **/
  void setMessageFont(const QFont& font);

  /**
   * This sets both - nameFont and messageFont to font. You
   * probably want to use this if you don't wish to distinguish between
   * these parts of a message.
   * @param font A font used for both nameFont and messageFont
   **/
  void setBothFont(const QFont& font);

  /**
   * Same as setNameFont but applies only to system messages.
   **/
  void setSystemNameFont(const QFont& font);

  /**
   * Same as setMessageFont but applies only to system messages.
   **/
  void setSystemMessageFont(const QFont& font);

  /**
   * Same as setBothFont but applies only to system messages.
   **/
  void setSystemBothFont(const QFont& font);

  /**
   * This font should be used for the name (the "from: " part) of a
   * message. layoutMessage uses this to set the font using
   * KChatBaseItemDelegate::setNameFont but if you want to overwrite
   * layoutMessage you should do this yourself.
   * @return The font that is used for the name part of the message.
   **/
  const QFont& nameFont() const;

  /**
   * This font should be used for a message. layoutMessage sets the
   * font of a message using KChatBaseItemDelegate::setMessageFont but if you
   * replace layoutMessage with your own function you should use
   * messageFont() yourself.
   * @return The font that is used for a message
   **/
  const QFont& messageFont() const;

  /**
   * Same as systemNameFont but applies only to system messages.
   **/
  const QFont& systemNameFont() const;

  /**
   * Same as systemMessageFont but applies only to system messages.
   **/
  const QFont& systemMessageFont() const;

  /**
   * Save the configuration of the dialog to a KConfig object. If
   * the supplied KConfig pointer is NULL then KGlobal::config() is used
   * instead (and the group is changed to "KChatBase") butr the current
   * group is restored at the end.
   * @param conf A pointer to the KConfig object to save the config
   * to. If you use 0 then KGlobal::config() is used and the group is changed
   * to "KChatBase" (the current group is restored at the end).
   **/
  virtual void saveConfig(KConfig* conf = 0);

  /**
   * Read the configuration from a KConfig object. If the pointer is
   * NULL KGlobal::config() is used and the group is changed to "KChatBase".
   * The current KConfig::group is restored after this call.
   **/
  virtual void readConfig(KConfig* conf = 0);

  /**
   * Set the maximum number of items in the list. If the number of item
   * exceeds the maximum as many items are deleted (oldest first) as
   * necessary. The number of items will never exceed this value.
   * @param maxItems the maximum number of items. -1 (default) for
   * unlimited.
   **/
  void setMaxItems(int maxItems);

  /**
   * Clear all messages in the list.
   **/
  void clear();

  /**
   * @return The maximum number of messages in the list. -1 is unlimited. See also
   * setMaxItems
   **/
  int maxItems() const;

public Q_SLOTS:
  /**
   * Add a text in the listbox. See also signalSendMessage()
   *
   * Maybe you want to replace this with a function that creates a nicer text
   * than "fromName: text"
   *
   * Update: the function layoutMessage is called by this now. This
   * means that you will get user defined outlook on the messages :-)
   * @param fromName The player who sent this message
   * @param text The text to be added
   **/
  virtual void addMessage(const QString& fromName, const QString& text);

  /**
   * This works just like addMessage but adds a system message. System
   * messages will have a different look than player messages.
   *
   * You may wish to  use this to display status information from your game.
   **/
  virtual void addSystemMessage(const QString& fromName, const QString& text);

  /**
   * This clears all messages in the view. Note that only the messages are
   * cleared, not the sender names in the combo box!
   **/
  void slotClear();

  private:
    KChatBaseModelPrivate* d;
};

#endif
