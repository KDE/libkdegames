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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
/*
    $Id$
*/
#ifndef __KGAMEPROCESS_H_
#define __KGAMEPROCESS_H_

#include <QString>
#include <QObject>
#include <QFile>

#include "kgameproperty.h"
#include <krandomsequence.h>
#include <libkdegames_export.h>
class KPlayer;
class KMessageFilePipe;

/**
 * This is the process class used on the computer player
 * side to communicate with its counterpart KProcessIO class.
 * Using these two classes will give fully transparent communication
 * via QDataStreams.
 */
class KDEGAMES_EXPORT KGameProcess:  public QObject
{
  Q_OBJECT

  public:
    /**
     * Creates a KGameProcess class. Done only in the computer
     * player. To activate the communication you have to call
     * the exec function of this class which will listen
     * to the communication and emit signals to notify you of
     * any incoming messages.
     * Note: This function will only return after you set
     * setTerminate(true) in one of the received signals.
     * So you can not do any computer calculation after the exec function.
     * Instead you react on the signals which are emitted after a
     * message is received and perform the calculations there!
     * Example:
     * \code
     *  int main(int argc ,char * argv[])
     *  {
     *    KGameProcess proc;
     *    connect(&proc,SIGNAL(signalCommand(QDataStream &,int ,int ,int )),
     *                    this,SLOT(slotCommand(QDataStream & ,int ,int ,int )));
     *    connect(&proc,SIGNAL(signalInit(QDataStream &,int)),
     *                    this,SLOT(slotInit(QDataStream & ,int )));
     *    connect(&proc,SIGNAL(signalTurn(QDataStream &,bool )),
     *                    this,SLOT(slotTurn(QDataStream & ,bool )));
     *    return proc.exec(argc,argv);
     *  }
     *  \endcode
     */
    KGameProcess();
    /**
     * Destruct the process
     */
    ~KGameProcess();

    /**
     * Enters the event loop of the computer process. Does only
     * return on setTerminate(true)!
     */
    bool exec(int argc, char *argv[]);

    /**
     * Should the computer process leave its exec function?
     * Activated if you setTerminate(true);
     *
     * @return true/false
     */
    bool terminate() const {return mTerminate;}

    /**
     * Set this to true if the computer process should end, ie
     * leave its exec function.
     *
     * @param b true for exit the exec function
     */
    void setTerminate(bool b) {mTerminate=b;}

    /**
     * Sends a message to the corresponding KGameIO
     * device. Works like the sendSystemMessage but
     * for user id's
     *
     * @param stream the QDataStream containing the message
     * @param msgid the message id for the message
     * @param receiver unused
     */
    void sendMessage(QDataStream &stream,int msgid,quint32 receiver=0);

    /**
     * Sends a system message to the corresonding KGameIO device.
     * This will normally be either a performed move or a query
     * (IdProcessQuery). The query option is a way to communicate
     * with the KGameIO at the other side and e.g. retrieve some
     * game relevant data from here.
     * Exmaple for a query:
     * \code
     *  QByteArray buffer;
     *  QDataStream out(buffer,QIODevice::WriteOnly);
     *  int msgid=KGameMessage::IdProcessQuery;
     *  out << (int)1;
     *  proc.sendSystemMessage(out,msgid,0);
     * \endcode
     *
     * @param stream the QDataStream containing the message
     * @param msgid the message id for the message
     * @param receiver unused
     */
    void sendSystemMessage(QDataStream &stream,int msgid,quint32 receiver=0);

    /**
     * Returns a pointer to a KRandomSequence. You can generate
     * random numbers via e.g.
     * \code
     *   random()->getLong(100);
     * \endcode
     * 
     * @return KRandomSequence pointer
     */
    KRandomSequence *random() {return mRandom;}

  protected:
    /**
     * processes the command line argumens to set up the computer player
     * Pass the argumens exactely as given by main()
     */
    void processArgs(int argc, char *argv[]);

  protected Q_SLOTS:
    /**
     * A message is received via the interprocess connection. The
     * appropriate signals are called.
     */
      void receivedMessage(const QByteArray& receiveBuffer);

  Q_SIGNALS:
    /**
     * The generic communication signal. You have to connect to this
     * signal to generate a valid computer response onto arbitrary messages.
     * All signals but IdIOAdded and IdTurn end up here!
     * Example:
     * \code
     * void Computer::slotCommand(int &msgid,QDataStream &in,QDataStream &out)
     * {
     *   qint32 data,move;
     *   in >> data;
     *   // compute move ...
     *   move=data*2;
     *   out << move;
     * }
     * \endcode
     *
     * @param inputStream the incoming data stream
     * @param msgid the message id of the message which got transmitted to the computer
     * @param receiver the id of the receiver
     * @param sender the id of the sender
     */
     void signalCommand(QDataStream &inputStream,int msgid,int receiver,int sender);

     /**
      * This signal is emmited if the computer player should perform a turn.
      * Calculations can be made here and the move can then be send back with
      * sendSystemMessage with the message id KGameMessage::IdPlayerInput.
      * These must provide a move which complies to your other move syntax as
      * e.g. produces by keyboard or mouse input.
      * Additonal data which have been written into the stream from the
      * ProcessIO's signal signalPrepareTurn can be retrieved from the
      * stream here.
      * Example:
      * \code
      * void slotTurn(QDataStream &in,bool turn)
      * {
      *   int id;
      *   int recv;
      *   QByteArray buffer;
      *   QDataStream out(buffer,QIODevice::WriteOnly);
      *   if (turn)
      *   {
      *     // Create a move - the format is yours to decide
      *     // It arrives exactly as this in the kgame inputMove function!!
      *     qint8 x1,y1,pl;
      *     pl=-1;
      *     x1=proc.random()->getLong(8);
      *     y1=proc.random()->getLong(8);
      *     // Stream it
      *     out << pl << x1 << y1;
      *     id=KGameMessage::IdPlayerInput;
      *     proc.sendSystemMessage(out,id,0);
      *   }
      * }
      * \endcode
      *
      * @param stream The datastream which contains user data
      * @param turn True or false whether the turn is activated or deactivated
      *
      */
     void signalTurn(QDataStream &stream,bool turn);

     /**
      * This signal is emmited when the process is initialized, i.e. added
      * to a KPlayer. Initial initialisation can be performed here be reacting
      * to the KProcessIO signal signalIOAdded and retrieving the data here
      * from the stream. 
      * It works just as the signalTurn() but is only send when the player is
      * added to the game, i.e. it needs some initialization data
      *
      * @param stream The datastream which contains user data 
      * @param userid The userId of the player. (Careful to rely on it yet)
      */
     void signalInit(QDataStream &stream,int userid);

  protected:
    bool mTerminate;
    KMessageFilePipe *mMessageIO;
  private:
    QFile rFile;
    QFile wFile;
    KRandomSequence* mRandom;
};
#endif
