/*
    This file is part of the KDE games library
    Copyright (C) 2001 Martin Heni (martin@heni-online.de)
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef __KPLAYER_H_
#define __KPLAYER_H_

#include <qstring.h>
#include <qobject.h>
#include <qptrlist.h>

#include "kgameproperty.h"
#include <kdemacros.h>

class KGame;
class KGameIO;
class KGamePropertyBase;
class KGamePropertyHandler;

class KPlayerPrivate;

/**
 * @short Base class for a game player
 *
 * The KPlayer class is the central player object. It holds
 * information about the player and is responsible for any
 * input the player does. For this arbitrary many KGameIO
 * modules can be plugged into it. Main features are:
 * - Handling of IO devices
 * - load/save (mostly handled by KGamePropertyHandler)
 * - Turn handling (turn based, asynchronous)
 *
 * A KPlayer depends on a KGame object. Call KGame::addPlayer() to plug
 * a KPlayer into a KGame object. Note that you cannot do much with a
 * KPlayer object before it has been plugged into a KGame. This is because
 * most properties of KPlayer are KGameProperty which need to send messages
 * through a KGame object to be changed. 
 *
 * A KGameIO represents the input methods of a player and you should make all
 * player inputs through it. So call something like playerInput->move(4);
 * instead which should call KGameIO::sendInput() to actually move. This way
 * you gain a *very* big advantage: you can exchange a KGameIO whenever you
 * want! You can e.g. remove the KGameIO of a local (human) player and just
 * replace it by a computerIO on the fly! So from that point on all playerInputs
 * are done by the computerIO instead of the human player. You also can replace
 * all network players by computer players when the network connection is broken
 * or a player wants to quit. 
 * So remember: use KGameIO whenever possible! A KPlayer should just
 * contain all data of the player (KGameIO must not!) and several common
 * functions which are shared by all of your KGameIOs.
 *
 */
class KDE_EXPORT KPlayer : public QObject
{
  Q_OBJECT

public:
      typedef QPtrList<KGameIO> KGameIOList;

      // KPlayer(KGame *,KGameIO * input=0);
      /**
       * Create a new player object. It will be automatically
       * deleted if the game it belongs to is deleted.
       */
      KPlayer();

      /**
       * Create a new player object. It will be automatically
       * deleted if the game it belongs to is deleted. This constructor
       * automatically adds the player to the game using KGame::addPlayer()
       */
      KPlayer(KGame* game);

      virtual ~KPlayer();

      /**
      * The idendification of the player. Overwrite this in
      * classes inherting KPlayer to run time identify them.
      *
      * @return 0 for default KPlayer.
      */
      virtual int rtti() const {return 0;}

      /**
      * Gives debug output of the game status
      */
      void Debug();

      // properties
      /**
       * Returns a list of input devices 
       *
       * @return list of devices
       */
      KGameIOList *ioList() {return &mInputList;}

      /**
       * sets the game the player belongs to. This
       * is usually automatically done when adding a
       * player
       *
       * @param game the game
       */
      void setGame(KGame *game) {mGame=game;}

      /**
       * Query to which game the player belongs to
       *
       * @return the game
       */
      KGame *game() const {return mGame;}

      /**
       * Set whether this player can make turns/input
       * all the time (true) or only when it is its
       * turn (false) as it is used in turn based games
       *
       * @param a async=true turn based=false
       */
      void setAsyncInput(bool a) {mAsyncInput = a;}

      /**
       * Query whether this player does asynchronous 
       * input
       *
       * @return true/false
       */
      bool asyncInput() const {return mAsyncInput.value();}

      /**
       * Is this player a virtual player, ie is it 
       * created by mirroring a real player from another
       * network game. This mirroring is done autmatically
       * as soon as a network connection is build and it affects
       * all players regardless what type
       *
       * @return true/false
       */
      bool isVirtual() const;

      /**
       * @internal
       * Sets whether this player is virtual. This is internally
       * called
       *
       * @param v virtual true/false
       */
      void setVirtual(bool v);

      /**
       * Is this player an active player. An player is usually
       * inactivated if it is replaced by a network connection.
       * But this could also be called manually
       *
       * @return true/false
       */
      bool isActive() const {return mActive;}

      /**
       * Set an player as active (true) or inactive (false)
       *
       * @param v true=active, false=inactive
       */
      void setActive(bool v) {mActive=v;}

      /**
       * Returns the id of the player
       *
       * @return the player id
       */
      Q_UINT32 id() const; 

      /* Set the players id. This is done automatically by
       * the game object when adding a new player!
       *
       * @param i the player id
       */
      void setId(Q_UINT32 i);

      /**
       * Returns the user defined id of the player
       * This value can be used arbitrary by you to
       * have some user idendification for your player,
       * e.g. 0 for a white chess player, 1 for a black 
       * one. This value is more reliable than the player 
       * id whcih can even change when you make a network 
       * connection.
       *
       * @return the user defined player id
       */
      int userId() const {return mUserId.value();} 

      /* Set the user defined players id.
       *
       * @param i the user defined player id
       */
      void setUserId(int i) {mUserId = i;}

      /**
       * Returns whether this player can be replaced by a network
       * connection player. The name of this function can be 
       * improved ;-) If you do not overwrite the function to 
       * select what players shall play in a network the KGame
       * does an automatic selection based on the networkPriority
       * This is not a terrible important function at the moment.
       *
       * @return true/false
       */
      int networkPriority() const;

      /**
       * Set whether this player can be replaced by a network
       * player. There are to possible games. The first type
       * of game has arbitrary many players. As soon as a network
       * players connects the game runs with more players (not tagged
       * situation). The other type is e.g. games like chess which
       * require a constant player number. In a network game situation
       * you would tag one or both players of all participants. As
       * soon as the connect the tagged player will then be replaced
       * by the network partner and it is then controlled over the network.
       * On connection loss the old situation is automatically restored.
       *
       * The name of this function can be improved;-)
       *
       * @param b should this player be tagged
       */
      void setNetworkPriority(int b);

      /**
       * Returns the player which got inactivated to allow
       * this player to be set up via network. Mostly internal
       * function
       */
      KPlayer *networkPlayer() const;

      /**
       * Sets this network player replacement. Internal stuff 
       */
      void setNetworkPlayer(KPlayer *p);

      // A name and group the player belongs to
      /**
       * A group the player belongs to. This
       * Can be set arbitrary by you.
       */
      void setGroup(const QString& group);

      /**
       * Query the group the player belongs to.
       */
      virtual const QString& group() const;

      /**
       * Sets the name of the player.
       * This can be chosen arbitrary.
       * @param name The player's name
       */
      void setName(const QString& name);

      /**
       * @return The name of the player.
       */
      virtual const QString& name() const;


      // set devices
      /**
       * Adds an IO device for the player. Possible KGameIO devices
       * can either be taken from the existing ones or be self written.
       * Existing are e.g. Keyboard, Mouse, Computerplayer
       *
       * @param input the inut device
       * @return true if ok
       */
      bool addGameIO(KGameIO *input);

      /**
       * remove (and delete) a game IO device
       *
       * The remove IO(s) is/are deleted by default. If
       * you do not want this set the parameter deleteit to false
       *
       * @param input the device to be removed or 0 for all devices
       * @param deleteit true (default) to delete the device otherwisse just remove it
       * @return true on ok
       */
      bool removeGameIO(KGameIO *input=0,bool deleteit=true);

      /**
       * Finds the KGameIO devies with the given rtti code.
       * E.g. find the mouse or network device
       *
       * @param rtti the rtti code to be searched for
       * @return the KGameIO device
       */
      KGameIO *findRttiIO(int rtti) const;

      /**
       * Checks whether this player has a IO device of the
       * given rtti type
       *
       * @param rtti the rtti typed to be checked for
       * @return true if it exists
       */
      bool hasRtti(int rtti) const  {return findRttiIO(rtti)!=0;}

      // Message exchange
      /**
       * Forwards input to the game object..internal use only
       *
       * This method is used by KGameIO::sendInput(). Use that function
       * instead to send player inputs!
       *
       * This function forwards a player input (see KGameIO classes) to the
       * game object, see KGame, either to KGame::sendPlayerInput() (if
       * transmit=true, ie the message has just been created) or to
       * KGame::playerInput() (if player=false, ie the message *was* sent through
       * KGame::sendPlayerInput).
       */
      virtual bool forwardInput(QDataStream &msg,bool transmit=true, Q_UINT32 sender=0);

      /**
       * Forwards Message to the game object..internal use only
       */
      virtual bool forwardMessage(QDataStream &msg,int msgid,Q_UINT32 receiver=0,Q_UINT32 sender=0);

      // Game logic
      /**
       * is it my turn to go
       *
       * @return true/false
       */
      bool myTurn() const {return mMyTurn.value();}

      /**
       * Sets whether this player is the next to turn.
       * If exclusive is given all other players are set
       * to setTurn(false) and only this player can move
       *
       * @param b true/false
       * @param exclusive true (default)/ false
       * @return should be void
       */
      bool setTurn(bool b,bool exclusive=true);


      // load/save
     /**
      * Load a saved player, from file OR network. By default all 
      * KGameProperty objects in the dataHandler of this player are loaded
      * and saved when using load or save. If you need to save/load more
      * you have to replace this function (and save). You will probably
      * still want to call the default implementation additionally!
      * 
      * @param stream a data stream where you can stream the player from
      *
      * @return true?
      */
      virtual bool load(QDataStream &stream);

     /**
      * Save a player to a file OR to network. See also load
      *
      * @param stream a data stream to load the player from
      *
      * @return true?
      */
      virtual bool save(QDataStream &stream);

      /**
       * Receives a message
       * @param msgid The kind of the message. See messages.txt for further
       * information
       * @param stream The message itself
       * @param sender 
       **/
      void networkTransmission(QDataStream &stream,int msgid,Q_UINT32 sender);

      /**
       * Searches for a property of the player given its id. 
       * @param id The id of the property
       * @return The property with the specified id
       **/
      KGamePropertyBase* findProperty(int id) const;

      /**
       * Adds a property to a player. You would add all
       * your player specific game data as KGameProperty and
       * they are automatically saved and exchanged over network.
       *
       * @param data The property to be added. Must have an unique id!
       * @return false if the given id is not valid (ie another property owns
       * the id) or true if the property could be added successfully
       **/
      bool addProperty(KGamePropertyBase* data);

      /**
       * Calculates a checksum over the IO devices. Can be used to
       * restore the IO handlers. The value returned is the 'or'ed
       * value of the KGameIO rtti's. 
       * this is itnernally used for saving and restorign a player.
       */
      int calcIOValue();

       /**
        * @return the property handler
        */
       KGamePropertyHandler* dataHandler();

signals:
      /**
       *  The player object got a message which was targeted
       *  at it but has no default method to process it. This
       *  means probably a user message. Connecting to this signal
       *  allowed to process it.
       */
       void signalNetworkData(int msgid, const QByteArray& buffer, Q_UINT32 sender, KPlayer *me);

       /**
        * This signal is emmited if a player property changes its value and
        * the property is set to notify this change. This is an
        * important signal as you should base the actions on a reaction
        * to this property changes.
        */
       void signalPropertyChanged(KGamePropertyBase *property,KPlayer *me);

protected slots:
      /**
       * Called by KGameProperty only! Internal function!
       **/
      void sendProperty(int msgid, QDataStream& stream, bool* sent);
      /**
       * Called by KGameProperty only! Internal function!
       **/
      void emitSignal(KGamePropertyBase *me);


private:
      void init();

private:
      KGame *mGame;
      bool mActive;      // active player
      KGameIOList mInputList;

      // GameProperty // AB: I think we can't move them to KPlayerPrivate - inline
      // makes sense here
      KGamePropertyBool mAsyncInput;  // async input allowed
      KGamePropertyBool mMyTurn;      // Is it my turn to play (only useful if not async)?
      KGamePropertyInt  mUserId;      // a user defined id

      KPlayerPrivate* d;
};

#endif
