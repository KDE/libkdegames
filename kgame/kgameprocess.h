/* **************************************************************************
                           KGameProcess class
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
#ifndef __KGAMEPROCESS_H_
#define __KGAMEPROCESS_H_

#include <qstring.h>
#include <qobject.h>
#include <kplayerdata.h>

class KGameClientProcess;
class KPlayer;

/**
 * This is the process class used on the computer player
 * side to communicate with its counterpart KProcessIO class.
 * Using these two classes will give fully transparent communication
 * via QDataStreams.
 */
class KGameProcess:  public QObject
{
  Q_OBJECT

  public:
    /** 
     * Creates a KGameProcess class. Done only in the computer
     * player. 
     * Example:
     * <pre>
     *  KGameProcess proc;
     *  return proc.exec();
     *  </pre>
     */
    KGameProcess();
    /**
     * Destruct the process
     */
    ~KGameProcess();

    /**
     * Enters the event loop of the computer process
     */
    bool exec(int argc, char *argv[]);
    /**
     * Should the computer process leave its exec function?
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
     * returns the sleep time used in the event loop
     *
     * @return the sleep time
     */
    unsigned long sleepTime() const {return mSleep;}
    /**
     * Sets the sleep time used in the event loop
     *
     * @param t the sleep time
     */
    void setSleepTime(unsigned long t) {mSleep=t;}

    /**
     * Sends a message to the network
     * @param the QDataStream containing the message
     */
    void sendMessage(QDataStream &stream,int msgid,int receiver=0);

  protected:
    /**
     * processes the command line argumens to set up the computer player
     * Pass the argumens exactely as given by main()
     */
    void processArgs(int argc, char *argv[]);

    /**
     * Processes the incoming message and produces the signals
     */
    bool processInput();

  signals:
    /**
     * The main communication signal. You have to connect to this
     * signal to generate a valid computer response.
     * Example:
     * <pre>
     * void Computer::slotCommand(int &msgid,QDataStream &in,QDataStream &out)
     * {
     *   Q_INT32 data,move;
     *   in >> data;
     *   // compute move ...
     *   move=data*2;
     *   out << move;
     * }
     * </pre>
     *
     * @param id the message id of the message which got transmitted to the computer
     * @param inputStream the incoming data stream
     * @param outputStream the outgoing data stream for the move
     */
    void signalReceivedCommand(QDataStream &inputStream,int msgid,int receiver,int sender);



    
  protected:
    bool mTerminate;
    unsigned long mSleep;
    KGameClientProcess *mClient;
};
#endif
