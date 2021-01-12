/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Martin Heni <kde at heni-online.de>
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef __KGAMEIO_H__
#define __KGAMEIO_H__

// own
#include "libkdegamesprivate_export.h"
// Qt
#include <QString>
#include <QObject>
// Std
#include <memory>

class QEvent;
class QGraphicsScene;
class QKeyEvent;
class QMouseEvent;
class KPlayer;
class KGame;
class KGameIOPrivate;
class KGameKeyIOPrivate;
class KGameMouseIOPrivate;
class KGameProcessIOPrivate;
class KGameComputerIOPrivate;

/**
 *  \class KGameIO kgameio.h <KGame/KGameIO>
 *
 *  \short Base class for IO devices for games
 *
 *  This is the master class for
 *  creating IO game devices. You cannot use it directly.
 *  Either take one of the classes derived from it or
 *  you have to create your own IO class derived from it (more probably).
 *
 *  The idea behind this class is to provide a common interface
 *  for input devices into your game. By programming a KGameIO
 *  device you need not distinguish the actual IO in the game
 *  anymore. All work is done by the IO's. This allows very
 *  easy reuse in other games as well.
 *  A further advantage of using the IO's is that you can exchange
 *  the control of a player at runtime. E.g. you switch a player
 *  to be controlled by the computer or vice versa.
 *
 *  To achieve this you have to make all of your player inputs through a
 *  KGameIO. You will usually call KGameIO::sendInput to do so.
 *
 *  @author Martin Heni <kde at heni-online.de>
 */
class KDEGAMESPRIVATE_EXPORT KGameIO : public QObject
{
  Q_OBJECT

public:
    /**
     * Constructs a KGameIO object
     */
   KGameIO();
   explicit KGameIO(KPlayer*);
   virtual ~KGameIO();

    /**
     * Gives debug output of the game status
     */
   void Debug();

   /**
    * Identifies the KGameIO via the rtti function
    */
   enum IOMode {GenericIO=1,KeyIO=2,MouseIO=4,ProcessIO=8,ComputerIO=16};
   /**
    * Run time identification. Predefined values are from IOMode
    * You MUST overwrite this in derived classes!
    *
    * @return rtti value
    */
   virtual int rtti() const = 0;  // Computer, network, local, ...

   /**
    * This function returns the player who owns this IO
    *
    * @return the player this IO device is plugged into
    */
   KPlayer *player() const;

   /**
    * Equivalent to player()->game()
    * @return the @ref KGame object of this player
    **/
   KGame* game() const;

   /**
    * Sets the player to which this IO belongs to. This
    * is done automatically when adding a device to a
    * player
    *
    * @param p the player
    */
   void setPlayer(KPlayer *p);

   /**
    * Init this device by setting the player and e.g. sending an
    * init message to the device. This initialisation message is
    * very useful for computer players as you can transmit the
    * game status to them and only update this status in the setTurn
    * commands.
    *
    * Called by @ref KPlayer::addGameIO only!
    */
   virtual void initIO(KPlayer *p);

    /**
     * Notifies the IO device that the player's setTurn had been called
     * Called by KPlayer
     *
     * This emits @ref signalPrepareTurn and sends the turn if the send
     * parameter is set to true.
     *
     * @param b turn is true/false
     */
    virtual void notifyTurn(bool b);

    /**
     * Send an input message using @ref KPlayer::forwardInput
     **/
    bool sendInput(QDataStream& stream, bool transmit = true, quint32 sender = 0);

Q_SIGNALS:
    /**
     * Signal generated when @ref KPlayer::myTurn changes. This can either be
     * when you get the turn status or when you lose it.
     *
     * The datastream has to be filled with a move. If you set (or leave) the
     * send parameter to FALSE then nothing happens: the datastream will be
     * ignored. If you set it to TRUE @ref sendInput is used to
     * send the move.
     *
     * Often you want to ignore this signal (leave send=FALSE) and send the
     * message later. This is usually the case for a human player as he probably
     * doesn't react immediately. But you can still use this e.g. to notify the
     * player about the turn change.
     *
     * Example:
     * \code
     *  void GameWindow::slotPrepareTurn(QDataStream &stream,bool b,KGameIO *input,bool * )
     *  {
     *    KPlayer *player=input->player();
     *    if (!player->myTurn()) return ;
     *    if (!b) return ;        // only do something on setTurn(true)
     *    stream << 1 << 2 << 3;  // Some data for the process
     *  }
     * \endcode
     *
     * @param io the KGameIO object itself
     * @param stream the stream into which the move will be written
     * @param turn the argument of setTurn
     * @param send set this to true to send the generated move using @ref
     * sendInput
     **/
    void signalPrepareTurn(QDataStream & stream, bool turn, KGameIO *io, bool * send);

protected:
    explicit KGameIO(KGameIOPrivate &dd, KPlayer *player = nullptr);

private:
    Q_DECLARE_PRIVATE_D(d, KGameIO)
    friend class KGameIOPrivate;
    friend class KGameKeyIO;
    friend class KGameMouseIO;
    friend class KGameProcessIO;
    friend class KGameComputerIO;
    std::unique_ptr<KGameIOPrivate> const d;
    // KF6 TODO: change private d to protected d_ptr, use normal Q_DECLARE_PRIVATE, remove subclass friends

    Q_DISABLE_COPY(KGameIO)
};

/**
 *  \class KGameKeyIO kgameio.h <KGame/KGameIO>
 *
 *  The KGameKeyIO class. It is used to process keyboard input
 *  from a widget and create moves for the player it belongs to.
 *  @author Martin Heni <kde at heni-online.de>
 */
class KDEGAMESPRIVATE_EXPORT KGameKeyIO : public KGameIO
{
  Q_OBJECT

public:
    /**
     * Create a keyboard input devices. All keyboards
     * inputs of the given widgets are passed through a signal
     * handler signalKeyEvent and can be used to generate
     * a valid move for the player.
     * Note the widget you pass to the constructor must be
     * the main window of your application, e.g. view->parentWidget()
     * as QT does not forward your keyevents otherwise. This means
     * that this might be a different widget compared to the one you
     * use for mouse inputs!
     * Example:
     * \code
     * KGameKeyIO *input;
     *  input=new KGameKeyIO(myWidget);
     *  connect(input,SIGNAL(signalKeyEvent(KGameIO *,QDataStream &,QKeyEvent *,bool *)),
     *          this,SLOT(slotKeyInput(KGameIO *,QDataStream &,QKeyEvent *,bool *)));
     * \endcode
     *
     * @param parent The parents widget whose keyboard events * should be grabbed
     */
    explicit KGameKeyIO(QWidget *parent);
    virtual ~KGameKeyIO();

    /**
     * The identification of the IO
     *
     * @return KeyIO
     */
    int rtti() const override;

Q_SIGNALS:
      /**
       * Signal handler for keyboard events. This function is called
       * on every keyboard event. If appropriate it can generate a
       * move for the player the device belongs to. If this is done
       * and the event is eaten eatevent needs to be set to true.
       * What move you generate (i.e. what you write to the stream)
       * is totally up to you as it will not be evaluated but forwarded
       * to the player's/game's  input move function
       * Example:
       * \code
       * KPlayer *player=input->player(); // Get the player
       * qint32 key=e->key();
       * stream << key;
       * eatevent=true;
       * \endcode
       *
       * @param io the IO device we belong to
       * @param stream the stream where we write our move into
       * @param m The QKeyEvent we can evaluate
       * @param eatevent set this to true if we processed the event
       */
      void signalKeyEvent(KGameIO *io,QDataStream &stream,QKeyEvent *m,bool *eatevent);

protected:
       /**
       * Internal method to process the events
       */
       bool eventFilter( QObject *o, QEvent *e ) override;

private:
    Q_DECLARE_PRIVATE_D(KGameIO::d, KGameKeyIO)
    friend class KGameKeyIOPrivate;
#if KDEGAMESPRIVATE_BUILD_DEPRECATED_SINCE(7, 3)
	// Unused, kept for ABI compatibility
	const void * __kdegames_d_do_not_use;
#endif

    Q_DISABLE_COPY(KGameKeyIO)
};

/**
 *  \class KGameMouseIO kgameio.h <KGame/KGameIO>
 *
 *  The KGameMouseIO class. It is used to process mouse input
 *  from a widget and create moves for the player it belongs to.
 *  @author Martin Heni <kde at heni-online.de>
 */
class KDEGAMESPRIVATE_EXPORT KGameMouseIO : public KGameIO
{
  Q_OBJECT

public:
    /**
     * Creates a mouse IO device. It captures all mouse
     * event of the given widget and forwards them to the
     * signal handler signalMouseEvent.
     * Example:
     * \code
     * KGameMouseIO *input;
     * input=new KGameMouseIO(mView);
     * connect(input,SIGNAL(signalMouseEvent(KGameIO *,QDataStream &,QMouseEvent *,bool *)),
     *        this,SLOT(slotMouseInput(KGameIO *,QDataStream &,QMouseEvent *,bool *)));
     * \endcode
     *
     * @param parent The widget whose events should be captured
     * @param trackmouse enables mouse tracking (gives mouse move events)
     */
    explicit KGameMouseIO(QWidget *parent,bool trackmouse=false);
    explicit KGameMouseIO(QGraphicsScene *parent,bool trackmouse=false);
    virtual ~KGameMouseIO();

    /**
     * Manually activate or deactivate mouse tracking
     *
     * @param b true = tracking on
     */
    void setMouseTracking(bool b);
    /**
     * The identification of the IO
     *
     * @return MouseIO
     */
    int rtti() const override;

Q_SIGNALS:
      /**
       * Signal handler for mouse events. This function is called
       * on every mouse event. If appropriate it can generate a
       * move for the player the device belongs to. If this is done
       * and the event is eaten eatevent needs to be set to true.
       * @see signalKeyEvent
       * Example:
       * \code
       * KPlayer *player=input->player(); // Get the player
       * qint32 button=e->button();
       * stream << button;
       * eatevent=true;
       * \endcode
       *
       * @param io the IO device we belong to
       * @param stream the stream where we write our move into
       * @param m The QMouseEvent we can evaluate
       * @param eatevent set this to true if we processed the event
       */
      void signalMouseEvent(KGameIO *io,QDataStream &stream,QMouseEvent *m,bool *eatevent);

protected:
      /**
      * Internal event filter
      */
      bool eventFilter( QObject *o, QEvent *e ) override;

private:
    Q_DECLARE_PRIVATE_D(KGameIO::d, KGameMouseIO)
    friend class KGameMouseIOPrivate;
#if KDEGAMESPRIVATE_BUILD_DEPRECATED_SINCE(7, 3)
	// Unused, kept for ABI compatibility
	const void * __kdegames_d_do_not_use;
#endif

    Q_DISABLE_COPY(KGameMouseIO)
};


/**
 *  \class KGameProcessIO kgameio.h <KGame/KGameIO>
 *
 *  The KGameProcessIO class. It is used to create a computer player
 *  via a separate process and communicate transparently with it.
 *  Its counterpart is the KGameProcess class which needs
 *  to be used by the computer player. See its documentation
 *  for the definition of the computer player.
 */
class KDEGAMESPRIVATE_EXPORT KGameProcessIO : public KGameIO
{
  Q_OBJECT

public:
    /**
     * Creates a computer player via a separate process. The process
     * name is given as fully qualified filename.
     * Example:
     * \code
     * KGameProcessIO *input;
     *   input=new KGameProcessIO(executable_file);
     *  connect(input,SIGNAL(signalPrepareTurn(QDataStream &,bool,KGameIO *,bool *)),
     *          this,SLOT(slotPrepareTurn(QDataStream &,bool,KGameIO *,bool *)));
     *  connect(input,SIGNAL(signalProcessQuery(QDataStream &,KGameProcessIO *)),
     *          this,SLOT(slotProcessQuery(QDataStream &,KGameProcessIO *)));
     * \endcode
     *
     * @param name the filename of the process to start
     */
    explicit KGameProcessIO(const QString& name);

    /**
     * Deletes the process input devices
     */
    virtual ~KGameProcessIO();

    /**
     * The identification of the IO
     *
     * @return ProcessIO
     */
    int rtti() const override;

    /**
     * Send a message to the process. This is analogous to the sendMessage
     * commands of KGame. It will result in a signal of the computer player
     * on which you can react in the process player.
     *
     * @param stream  - the actual data
     * @param msgid - the id of the message
     * @param receiver - not used
     * @param sender - who send the message
     */
    void sendMessage(QDataStream &stream,int msgid, quint32 receiver, quint32 sender);

    /**
     * Send a system message to the process. This is analogous to the sendMessage
     * commands of KGame. It will result in a signal of the computer player
     * on which you can react in the process player.
     *
     * @param stream  - the actual data
     * @param msgid - the id of the message
     * @param receiver - not used
     * @param sender - who send the message
     */
    void sendSystemMessage(QDataStream &stream, int msgid, quint32 receiver, quint32 sender);

    /**
     * Init this device by setting the player and e.g. sending an
     * init message to the device. Calling this function will emit
     * the IOAdded signal on which you can react and initilise the
     * computer player.
     * This function is called automatically when adding the IO to
     * a player.
     */
    void initIO(KPlayer *p) override;

    /**
     *  Notifies the IO device that the player's setTurn had been called
     *  Called by KPlayer. You can react on the @ref signalPrepareTurn to
     *  prepare a message for the process, i.e. either update it on
     * the changes made to the game since the last turn or the initIO
     * has been called or transmit your gamestatus now.
     *
     *  @param turn is true/false
     */
    void notifyTurn(bool turn) override;

  protected:
    /**
     * Internal combined function for all message handling
     **/
    void sendAllMessages(QDataStream &stream,int msgid, quint32 receiver, quint32 sender, bool usermsg);

  protected Q_SLOTS:
  /**
  * Internal message handler to receive data from the process
  */
    void receivedMessage(const QByteArray& receiveBuffer);


Q_SIGNALS:
  /**
   * A computer query message is received. This is a 'dummy'
   * message sent by the process if it needs to communicate
   * with us. It is not forwarded over the network.
   * Reacting to this message allows you to 'answer' questions
   * of the process, e.g. sending addition data which the process
   * needs to calculate a move.
   *
   * Example:
   * \code
   *  void GameWindow::slotProcessQuery(QDataStream &stream,KGameProcessIO *reply)
   *  {
   *    int no;
   *    stream >> no;  // We assume the process sends us an integer question number
   *    if (no==1)     // but YOU have to do this in the process player
   *    {
   *      QByteArray buffer;
   *      QDataStream out(buffer,QIODevice::WriteOnly);
   *      reply->sendSystemMessage(out,4242,0,0);  // lets reply something...
   *    }
   *  }
   * \endcode
   */
  void signalProcessQuery(QDataStream &stream,KGameProcessIO *me);

  /**
  * Signal generated when the computer player is added.
  * You can use this to communicated with the process and
  * e.g. send initialisation information to the process.
  *
  * @param game the KGameIO object itself
  * @param stream the stream into which the move will be written
  * @param p the player itself
  * @param send set this to false if no move should be generated
  */
  void signalIOAdded(KGameIO *game,QDataStream &stream,KPlayer *p,bool *send);

  /** Text is received by the process on STDERR. This is usually a debug string.
    * @param msg The text
    */
  void signalReceivedStderr(const QString &msg);

private:
    Q_DECLARE_PRIVATE_D(KGameIO::d, KGameProcessIO)
    friend class KGameProcessIOPrivate;
#if KDEGAMESPRIVATE_BUILD_DEPRECATED_SINCE(7, 3)
	// Unused, kept for ABI compatibility
	const void * __kdegames_d_do_not_use;
#endif

    Q_DISABLE_COPY(KGameProcessIO)
};

/**
 *  \class KGameComputerIO kgameio.h <KGame/KGameIO>
 *
 *  \brief KGameIO variant for real-time games
 *
 *  The KGameComputerIO class. It is used to create a LOCAL computer player
 *  and communicate transparently with it.
 *  Question: Is this needed or is it overwritten anyway for a real game?
 *
 *  You most probably don't want to use this if you want to design a turn based
 *  game/player. You'll rather use @ref KGameIO directly, i.e. subclass it
 *  yourself. You just need to use @ref KGameIO::signalPrepareTurn and/or @ref
 *  KGameIO::notifyTurn there.
 *
 *  This is rather meant to be of use in real time games.
 *
 *  @author  <b_mann@gmx.de>
 */
class KDEGAMESPRIVATE_EXPORT KGameComputerIO : public KGameIO
{
  Q_OBJECT

public:
    /**
     * Creates a LOCAL computer player
     *
     */
    KGameComputerIO();
    explicit KGameComputerIO(KPlayer* player);
    ~KGameComputerIO();

    int rtti() const override;

    /**
     * The number of advance calls until the player (or rather: the IO)
     * does something (default: 1).
     **/
    void setReactionPeriod(int advanceCalls);
    int reactionPeriod() const;

    /**
     * Start a QTimer which calls advance every @p ms milli seconds.
     **/
    void setAdvancePeriod(int ms);

    void stopAdvancePeriod();

    /**
     * Ignore calls number of advance calls. if calls is -1 then all
     * following advance calls are ignored until unpause is called.
     *
     * This simply prevents the internal advance counter to be increased.
     *
     * You may want to use this to emulate a "thinking" computer player. Note
     * that this means if you increase the advance period (see
     * setAdvancePeriod), i.e. if you change the speed of your game, your
     * computer player thinks "faster".
     * @param calls Number of advance calls to be ignored
     **/
    void pause(int calls = -1);

    /**
     * Equivalent to pause(0). Immediately continue to increase the internal
     * advance counter.
     **/
    void unpause();

public Q_SLOTS:
    /**
     * Works kind of similar to QCanvas::advance. Increase the internal
     * advance counter. If @p reactionPeriod is reached the counter is set back to
     * 0 and @ref signalReaction is emitted. This is when the player is meant
     * to do something (move its units or so).
     *
     * This is very useful if you use QCanvas as you can use this in your
     * QCanvas::advance call. The advantage is that if you change the speed
     * of the game (i.e. change QCanvas::setAdvancePeriod) the computer
     * player gets slower as well.
     *
     * If you don't use QCanvas you can use setAdvancePeriod to get
     * the same result. Alternatively you can just use a QTimer.
     *
     **/
    virtual void advance();

Q_SIGNALS:
    /**
     * This signal is emitted when your computer player is meant to do
     * something, or better is meant to be allowed to do something.
     **/
    void signalReaction();

protected:
    /**
     * Default implementation simply emits signalReaction
     **/
    virtual void reaction();

private:
    Q_DECLARE_PRIVATE_D(KGameIO::d, KGameComputerIO)
    friend class KGameComputerIOPrivate;
#if KDEGAMESPRIVATE_BUILD_DEPRECATED_SINCE(7, 3)
	// Unused, kept for ABI compatibility
	const void * __kdegames_d_do_not_use;
#endif

    Q_DISABLE_COPY(KGameComputerIO)
};


#endif
