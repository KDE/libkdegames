/* **************************************************************************
                           KGameIO class
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
/*
    $Id$
*/
#ifndef __KGAMEINPUT_H_
#define __KGAMEINPUT_H_


#include <qstring.h>
#include <qobject.h>

class KPlayer;
class KGameClient;
class KGameClientProcess;
class KProcess;

/**
 *  The KGameIO class. This is the master class for
 *  creating IO game devices. You cannot use it directly.
 *  Either take one of the classes derived from it or
 *  you have to create your own IO class derived from it.
 */
class KGameIO : public QObject
{
  Q_OBJECT

public:
    /**
     * Constructs a KGameIO object
     */
   KGameIO();
   ~KGameIO();

    /**
     * Gives debug output of the game status
     */
   void Debug();

   enum IOMode {GenericIO=1,KeyIO=2,MouseIO=4,ProcessIO=8,ComputerIO=16};
   /**
    * Run time idendification. Predefined values are from IOMode
    *
    * You MUST overwrite this in derived classes!
    *
    * @return rtti value
    */
   virtual int rtti() const = 0;  // Computer, network, local, ...
   /**
    * @return the player this IO device is plugged into
    */
   KPlayer *player() const {return mPlayer;}
   /**
    * Sets the player to which this IO belongs to. This
    * is done automatically when adding a device to a 
    * player
    *
    * @param p the player
    */
   void setPlayer(KPlayer *p) {mPlayer=p;}

   /**
    * Message to clients. Needs to be overwritten to be
    * of use.
    * You have to call @ref KGameNetwork::registerListener to receive messages!
    *
    * This is used for messages sent through sendSystemMessage, like adding
    * player. Use @ref receiveMessage for your own messages instead!
    */
    virtual void receiveSystemMessage(QDataStream &stream, int msgid, int receiver, int sender);

    /**
    * Message to clients. Needs to be overwritten to be
    * of use.
    * You have to call @ref KGameNetwork::registerListener to receive messages!
    *
    * This is used for user messages, ie messages sent through @ref
    * KGame::sendMessage
    *
    * You probably want to use this for your game messages instead of @ref
    * receiveSystemMessage
    **/
    virtual void receiveMessage(QDataStream &stream, int msgid, int receiver, int sender);
    
    /**
     *  Notifies the IO device that the player's setTurn had been called
     *  Called by KPlayer
     *
     *  @param turn is true/false
     */
    virtual void notifyTurn(bool b);

    

private:  
   KPlayer *mPlayer;
};

/**
 *  The KGameKeyIO class. It is used to process keyboard input
 *  from a widget and create moves for the player it belongs to.
 */
class KGameKeyIO : public KGameIO
{
  Q_OBJECT
    
public:
    /**
     * Create a Keyboard input devices. All keyboards
     * inputs of the given widgets are passed through a signal
     * handler @ref #signalKeyEvent and can beused to generate
     * a valid move for the player.
     *
     * @param parent The parents widget whose keyboard events * should be grabbed
     */
    KGameKeyIO(QWidget *parent);
    /**
     * The idendification of the IO
     *
     * @return KeyIO
     */
    virtual int rtti() const;

signals:
      // calulate move message from mouse event
      /**
       * Signal handler for keyboard events. This function is called
       * on every keyboard event. If appropriate it can generate a
       * move for the player the device belongs to. If this is done
       * and the event is eaten eatevent needs to be set to true.
       * Example:
       * <pre>
       * KPlayer *player=input->player(); // Get the player
       * Q_INT32 key=e->key();
       * stream << key;
       * eatevent=true;
       * </pre>
       *
       * @param io the IO device we belong to
       * @param stream the stream where we write our move into
       * @param m The QKeyEvent we can evaluate
       * @param eatevent set this to true if we processed the event
       */
      void signalKeyEvent(KGameIO *,QDataStream &stream,QKeyEvent *m,bool &eatevent);

protected:
  virtual bool eventFilter( QObject *o, QEvent *e );
};

/**
 *  The KMouseKeyIO class. It is used to process mouse input
 *  from a widget and create moves for the player it belongs to.
 */
class KGameMouseIO : public KGameIO
{
  Q_OBJECT
    
public:
    /**
     * Creates a mouse IO device. It captures all mouse
     * event of the given widget and forwards them to the
     * signal handler @ref #signalMouseEvent.
     *
     * @param parent the parent widget whose events shoudl be captured
     * @param trackmouse enables mouse tracking (gives mouse move events)
     */
    KGameMouseIO(QWidget *parent,bool trackmouse=false);
    /**
     * Manually activate or deactivate mouse tracking
     *
     * @param b true = tracking on
     */
    void setMouseTracking(bool b);
    /**
     * The idendification of the IO
     *
     * @return MouseIO
     */
    virtual int rtti() const; 

signals:
      // calulate move message from mouse event
      /**
       * Signal handler for mouse events. This function is called
       * on every mouse event. If appropriate it can generate a
       * move for the player the device belongs to. If this is done
       * and the event is eaten eatevent needs to be set to true.
       * See alsp the analogous @ref #signalKeyEvent
       * Example:
       * <pre>
       * KPlayer *player=input->player(); // Get the player
       * Q_INT32 button=e->button();
       * stream << button;
       * eatevent=true;
       * </pre>
       *
       * @param io the IO device we belong to
       * @param stream the stream where we write our move into
       * @param m The QMouseEvent we can evaluate
       * @param eatevent set this to true if we processed the event
       */
      void signalMouseEvent(KGameIO *,QDataStream &stream,QMouseEvent *m,bool &eatevent);

protected:
  virtual bool eventFilter( QObject *o, QEvent *e );

};


class KGameProcessIOPrivate;
/**
 *  The KProcessIO class. It is used to create a computer player
 *  via a separate process and communicate transparetly with it.
 */
class KGameProcessIO : public KGameIO
{
  Q_OBJECT
    
public:
    /** 
     * Creates a computer player via a separate process. The process
     * name is given as fully qualified filename. As soon as any input
     * goes to this process it is started and it is killed when the input
     * devices is deleted.
     *
     * @param name the filename of the process to start
     */
    KGameProcessIO(const QString& name);
    ~KGameProcessIO();
    /**
     * Returns whether the process is still running
     *
     * @return true/false
     */
    bool isRunning();

    /**
     * The idendification of the IO
     *
     * @return ProcessIO
     */
    virtual int rtti() const;
    /** 
     * The internal sleep time 
     */
    unsigned long sleepTime() const {return mSleep;}
    /**
     * Change the internal sleep time which is used
     * to wait for communication events
     */
    void  setSleepTime(unsigned long t) {mSleep=t;}
    /**
     * forward a message from the game to the process. Do not
     * call directly. Use @ref sendProcess instead.
     */
    virtual void receiveMessage(QDataStream &stream,int msgid, int receiver, int sender);
    virtual void receiveSystemMessage(QDataStream &stream, int msgid, int receiver, int sender);
    virtual void receiveAllMessage(QDataStream &stream,int msgid, int receiver, int sender, bool usermsg);

    /**
     * Sends a message to the process. Cookie needs to be the application cookie
     * (or 4242 as generic one...).
     *
     * @param stream is the message
     * @param msgid   is the message id
     * @param receiver is the receiver and does not really matter
     * @param sender  is the sender of the message and does not really matter
     * @param sender  is the sender of the message and does not really matter
     * @param cookie is the application cookie and does not really matter
     *
     */ 
    bool sendProcess(const QDataStream &stream,int msgid,int receiver=0,int sender=0);

    /**
     * Returns the KGameClientProcess object used
     */
    KGameClientProcess *client() const {return mClient;}

    /**
     *  Notifies the IO device that the player's setTurn had been called
     *  Called by KPlayer
     *
     *  @param turn is true/false
     */
    virtual void notifyTurn(bool b);

public slots:
  void  slotReceivedStdout(KProcess *proc, char *buffer, int buflen);
  void  slotReceivedStderr(KProcess *proc, char *buffer, int buflen);
  void  slotProcessExited(KProcess *p);
  void  slotWroteStdin(KProcess *p);

    /**
     * forward to the computer player. Needs to be improved 
     */
  //virtual bool SendIOMessage(QDataStream &msg,int cmd=0); // msg to client
  
signals:
  /**
   * Gives a signal that the computer player ended. Maybe we could give
   * an option to automatically restart the player?
   */
  void signalProcessExited(KGameProcessIO *me);

  /**
   * This signal is emmited if the computer player is about to
   * receive a message. If the parameter killit is set to
   * true the message is not forwarded to the player. This can
   * be used as a message filter. A more detailed filter
   * has to be implemented inthe computer player itself. But
   * stopping messages here saves traffic.
   *
   * @param msgid the message id
   * @param killit set this to true to not deliver the message
   */
  void signalProcessIOMessage(int msgid, bool &killit);

  /**
   * A computer query message is received. This is a 'dummy'
   * message sent by the process if it needs to communicate
   * with us. It is not forwarded over the network.
   */
  void signalProcessQuery(const QDataStream &stream,KGameProcessIO *me);

  /**
  * Signal generated when the computer player is activated. 
  * The datastream has to be filled with a move
  *
  * @param the KGameIO object itself
  * @param the stream into which themove will be written
  * @param the argument of setTurn
  * @param set this to false if no move should be generated
  */
  void signalPrepareTurn(KGameIO *,QDataStream &,bool,bool &);


protected:
  void Exit();
  bool Init();
    /**
     * write the collected buffers to stdout
     */
    bool writeBuffers();

private:
  unsigned long mSleep;
  KGameClientProcess *mClient;

  KGameProcessIOPrivate* d;
};

/**
 *  The KComputerIO class. It is used to create a LOCAL computer player
 *  and communicate transparetly with it.
 *  TODO: All functions are just dummy. I hopy Andi fills in his
 *  kpoker computer player class!
 */
class KGameComputerIO : public KGameIO
{
  Q_OBJECT
    
public:
    /** 
     * Creates a LOCAL computer player 
     *
     */
    KGameComputerIO();
    ~KGameComputerIO();
    virtual int rtti() const;
    /**
     *  Notifies the IO device that the player's setTurn had been called
     *  Called by KPlayer
     *
     *  @param turn is true/false
     */
    virtual void notifyTurn(bool b);

public slots:
  
signals:
      /**
      * Signal generated when the computer player is activated. 
      * The datastream has to be filled with a move
      *
      * @param the KGameIO object itself
      * @param the stream into which themove will be written
      * @param the argument of setTurn
      * @param set this to false if no move should be generated
      */
      void signalPrepareTurn(KGameIO *,QDataStream &,bool,bool &);

protected:

private:
};


#endif
