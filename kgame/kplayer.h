/* **************************************************************************
                           KPlayer Class
                           -------------------
    begin                : 1 January 2001
    copyright            : (C) 2001 by Martin Heni and Andreas Beckermann
    email                : martin@heni-online.de and b_mann@gmx.de
 ***************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Additional license: Any of the above copyright holders can add an     *
 *   enhanced license which complies with the license of the KDE core      *
 *   libraries so that this file resp. this library is compatible with     *
 *   the KDE core libraries.                                               *
 *   The user of this program shall have the choice which license to use   *
 *                                                                         *
 ***************************************************************************/
#ifndef __KPLAYER_H_
#define __KPLAYER_H_

#include <qstring.h>
#include <qobject.h>
#include <qlist.h>

#include "kgameproperty.h"

class KGame;
class KGameIO;
class KGamePropertyBase;

class KPlayerPrivate;

/**
 * The KPlayer class is the central player object. It holds
 * information about the player and is responsible for any
 * input the player does. For this arbitrary many KGameIO
 * modules can be plugged into it. Main features are:
 * - Handling of IO devices
 * - load/save
 * - Turn handling (turn based, asynchronous)
 */
class KPlayer : public QObject
{
  Q_OBJECT

public:
      typedef QList<KGameIO> KGameIOList;

      // KPlayer(KGame *,KGameIO * input=0);
      /**
       * Create a new player object. It will be automatically
       * deleted if the game it belongs to is deleted. An optional
       * constructor could be KPlayer(KGame *)
       */
      KPlayer();
      ~KPlayer();

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
      void setAsyncInput(bool a) {mAsyncInput.setValue(a);}
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
       * as soon as a network connectionis build and it affects
       * all players regardless what type
       *
       * @return true/false
       */
      bool isVirtual() const;

      /**
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
      int id() const; 
      /* Set the players id. This is done automatically by
       * the game object when adding a new player!
       *
       * @param i the player id
       */
      void setId(int i);

      /**
       * Returns the user defined id of the player
       *
       * @return the user defined player id
       */
      int userId() const {return mUserId.value();} 
      /* Set the user defined players id.
       *
       * @param i the user defined player id
       */
      void setUserId(int i) {mUserId.setValue(i);}

      /**
       * returns whether this player can be replaced by a network
       * connection player. The name of this function can be 
       * improved ;-)
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
       * On connection loss the old situation is automatically retored.
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
       * A group the player belongs to.
       */
      void setGroup(const QString& group);
      /**
       * Query the group the player belongs to.
       */
      virtual const QString& group() const;
      /**
       * Sets the name of the player.
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
       * remove a game IO device
       *
       * @param the device to be removed or 0 for all devices
       * @return true on ok
       */
      bool removeGameIO(KGameIO *input=0);
      
      /**
       * Finds the KGameIO devies with the given rtti code.
       * E.g. find the mouse or network device
       *
       * @param the rtti code to be searched for
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
       * This function forwards a player input (see @ref KGameIO classes) to the
       * game object, see @ref KGame, either to @ref KGame::sendPlayerInput (if
       * transmit=true, ie the message has just been created) or to @ref
       * KGame::playerInput (if player=false, ie the message *was* sent through
       * @ref KGame::sendPlayerInput).
       */
      virtual bool forwardInput(QDataStream &msg,bool transmit=true,int sender=0);

      /**
       * Forwards Message to the game object..internal use only
       */
      virtual bool forwardMessage(QDataStream &msg,int msgid,int receiver=0,int sender=0);

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
       * @param exlusive true (default)/ false
       * @return should be void
       */
      bool setTurn(bool b,bool exclusive=true);


      // load/save
     /**
      * Load a saved player, from file OR network. This function has
      * to be overwritten or you need to connect to the load signal!
      * 
      * @param stream a data stream where you can stream the player from
      *
      * @return true?
      */
      virtual bool load(QDataStream &stream);
     /**
      * Save a player to a file OR to network. Otherwise the same as 
      * the load function
      *
      * @param stream a data stream to load the player from
      *
      * @return true?
      */
      virtual bool save(QDataStream &stream);

      /**
       * Called by KGameProperty only! Internal function!
       **/
      void sendProperty(QDataStream& s);
      /**
       * Called by KGameProperty only! Internal function!
       **/
      void emitSignal(KGamePropertyBase *me);
     
      /**
       * Receives a message
       * @param msgid The kind of the message. See messages.txt for further
       * information
       * @param stream The message itself
       * @param sender 
       **/
      void networkTransmission(QDataStream &stream,int msgid,int sender);

      /**
       * Searches for a property of the player.
       * @param id The id of the property
       * @return The property with the specified id
       **/
      KGamePropertyBase* findProperty(int id) const;

      /**
       * Adds a property
       * @param data The property to be added. Must have an unique id!
       * @return false if the given id is not valid (ie another property owns
       * the id) or true if the property could be added successfully
       **/
      bool addProperty(KGamePropertyBase* data);

      /**
       * Calls @ref KGamePropertyBase::setLocked(false) for all properties of this
       * player
       **/
      void unlockProperties();
      /**
       * Calls @ref KGamePropertyBase::setLocked(true) for all properties of this
       * player
       *
       * Use with care! This will even lock the core properties, like name,
       * group and myTurn!!
       **/
      void lockProperties();

      /**
       * Calculates a checksum over the IO devices. Can be used to
       * restore the IO handlers. The value returned is the 'or'ed
       * value of the KGameIO rtti's
       */
      int calcIOValue();

signals:
     /**
      * the player will be loaded from the given stream. We better
      * fill our data.
      *
      * @param stream the load stream
      */
      void signalLoad(QDataStream &stream);
     /**
      * the player will be saved to the given stream. We better
      * put our data.
      *
      * @param stream the save stream
      */
      void signalSave(QDataStream &stream);
      /**
       *  The player object got a message which was targeted
       *  at it but has no default method to process it. This
       *  means probagbly a user message. Conencting to this signal
       *  alloed to process it.
       */
       void signalNetworkData(int msgid,QDataStream &stream,int sender,KPlayer *me);

       /**
        * This signal is emmited if a player property changes its value and
        * the property is set to notify this change
        *
        */
       void signalPropertyChanged(KGamePropertyBase *property,KPlayer *me);


protected:
       /**
        * @return the property handler
        */
       KGamePropertyHandlerBase* dataHandler();

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
