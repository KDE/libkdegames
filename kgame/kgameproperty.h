/* **************************************************************************
                           KGameProperty Class
                           -------------------
    begin                : 1 January 2001
    copyright            : (C) 2001 by Andreas Beckermann and Martin Heni
    email                : b_mann@gmx.de and martin@heni-online.de
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
#ifndef __KGAMEPROPERTY_H_
#define __KGAMEPROPERTY_H_

#include <qdatastream.h>
#include <qintdict.h>
#include <kdebug.h>
#include "kgamemessage.h"

class KGame;
class KPlayer;
class KGamePropertyHandlerBase;

/**
 * The KGamePropertyBase class is the base class of KGameProperty. See @ref
 * KGameProperty for further information.
 * 
 * @short Base class of KGameProperty
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KGamePropertyBase
{
public:
	enum PropertyDataIds  { // these belong to KPlayer!
		IdGroup=1,
		IdName=2,
		IdAsyncInput=3,
		IdTurn=4,
		IdGameStatus=5,
		IdMaxPlayer=6,
		IdMinPlayer=7,
		IdUserId=8,
    IdCommand, // Reserved for internal use
		IdUser=256
	};
  /**
   * Commands for advanced properties (Q_INT8)
   */
  enum PropertyCommandIds 
  {
    CmdAt=1,
    CmdResize=2,
    CmdFill=3
  };

	/**
	 * Constructs a KGamePropertyBase object and calls @ref registerData.
	 * @param id The id of this property. MUST be UNIQUE! Used to send and
	 * receive changes in the property of the playere automatically via
	 * network. 
	 * @param owner The owner of the object. Must be a KGamePropertyHandler which manages
	 * the changes made to this object, i.e. which will send the new data
	 **/
	KGamePropertyBase(int id, KGamePropertyHandlerBase* owner);

	/**
	 * Creates a KGamePropertyBase object without an owner. Remember to call
	 * @ref registerData!
	 **/
	KGamePropertyBase();

	virtual ~KGamePropertyBase();

	/**
	 * Sets this property to emit a signal on value changed.
	 * As the proerties do not inehrit QObject for optimisation
	 * this signal is emited via the KPlayer or KGame object
	 **/
	void setEmittingSignal(bool p)	{ mFlags.bits.emitsignal=p&1; }

	/**
	 * See also @ref setEmittingSignal
	 * @return Whether this property emits a signal on value change
	 **/
	bool isEmittingSignal()	const { return mFlags.bits.emitsignal; }

	/**
	 * See also @ref setReadOnly
	 * @return Whether the property can be changed
	 **/
	bool isReadOnly() const { return mFlags.bits.readonly; }

	/**
	 * Sets this property to try to optimize signal and network handling
   * by not sending it out when the property value is not changed.
	 **/
	void setOptimized(bool p)	{ mFlags.bits.optimize=p&1; }
	/**
	 * See also @ref setOptimize
	 * @return Whether the property optimizes access (signals,network traffic)
	 **/
	bool isOptimized() const { return mFlags.bits.optimize; }

	/**
	 * A readonly property cannot be changed. Use this if you to prevent a
	 * player from changing something, e.g. for a money-bases card game you
	 * will want to lock the "bet" property after a player has bet.
	 * 
	 * You have to call setReadOnly(false) before you are able to change the
	 * value of the property again. The default is not readonly.
	 * @param p True to lock this property, false to unlock it
	 **/
	void setReadOnly(bool p) { mFlags.bits.readonly = p&1; }
	

	/**
	 * This will read the value of this property from the stream. You MUST
	 * overwrite this method in order to use this class
	 * @param s The stream to read from
	 **/
	virtual void load(QDataStream& s) = 0;
	virtual void save(QDataStream& s) = 0;

  /** 
   * send a command to advanced properties like arrays
   */
  virtual void command(QDataStream & s,int cmd) {};

	/**
	 * @return The id of this property
	 **/
	int id() const { return mId; }

	/**
	 * You have to register a KGamePropertyBase before you can use it.
	 *
	 * You MUST call this before you can use KGamePropertyBase!
	 *
	 * @param id the id of this KGamePropertyBase object. The id MUST be
	 * unique, i.e. you cannot have two properties with the same id for one
	 * player, although (currently) nothing prevents you from doing so. But
	 * you will get strange results!
	 * @param owner The owner of this data. This player will send the data
	 * using @ref KPlayer::sendProperty whenever you call @ref setValue
	 *
	 **/
	void registerData(int id, KGamePropertyHandlerBase* owner);

protected:
	//AB: I had problems when putting this into KGameProperty::setValue() as I
	//had to include kplayer.h and kgame.h which caused problems e.g.
	//because they both include kgameproperty.h this member function is a
	//workaround to put the stuff into the .cpp file (as i cannot put
	//anything from KGameProperty into the .cpp file)
	//The problem now is that there is one more function call and could one
	//day be bad for performance
	/**
	 * Send the data to the owner of this property. Uses @ref save() to read
	 * the data
	 **/
	void send();
	
	/**
	 * Causes the parent object to emit a signal on value change
	 **/
	void emitSignal();

	KGamePropertyHandlerBase* mOwner;
	
	// Having this as a union of the bitfield and the char
	// allows us to stream this quantity easily (if we need to)
	// At the moment it is not yet transmitted
	union Flags {
		char flag;
		struct {
			// unsigned char dosave : 1;   // do save this property
			// unsigned char delaytransmit : 1;   // do not send immediately on
                                             // change but a KPlayer:QTimer
                                             // sends it later on - fast
                                             // changing variables
			unsigned char emitsignal:1; // KPlayer notifies on variable change (true)
						// can used 2 more
			unsigned char readonly : 1; // whether the property can be changed (false)
			unsigned char optimize : 1; // whether the property tries to optimize send/emit (false)
		} bits;
	} mFlags;
	
private:
	void init();
	
	bool mPublic;
	int mId;
};

/**
 * The class KGameProperty can store any form of data and will transmit it via
 * network whenver you call @ref setValue. This makes network transparent games
 * very easy. You first have to register the data to a @ref KPlayer using @ref
 * KGamePropertyBase::registerData (which is called by the constructor)
 *
 * If you want to use a custum class with KGameProperty you have to implement the
 * operators << and >> for QDataStream:
 * <pre>
 * class Card
 * {
 * public:
 * int type;
 * int suite;
 * };
 * QDataStream& operator<<(QDataStream& stream, Card& card)
 * {
 * Q_INT16 type = card.type;
 * Q_INT16 suite = card.suite;
 * s << type;//note: here is type first
 * s << suite;
 * return s;
 * }
 * QDataStream& operator>>(QDataStream& stream, Card& card)
 * {
 * Q_INT16 type;
 * Q_INT16 suite;
 * s >> type;
 * s >> suite;
 * card.suite = (int)suite;
 * card.type = (int)type;
 * return s;
 * }
 *
 * class Player : KPlayer
 * {
 * [...]
 * KGameProperty<Card> mCards;
 * };
 * </pre>
 *
 *
 * Note: unlike most QT classes KGameProperty objects are *not* deleted
 * automatically! So if you create an object using e.g. KGameProperty<int>* data =
 * new KGameProperty(0, this) you have to put a delete data into your destructor!
 * @short A class for network transparent games
 **/
template<class type>
class KGameProperty  : public KGamePropertyBase
{

public:
	/**
	 * Constructs a KGameProperty object. A KGameProperty object will transmit
	 * any changes to the server/master and, if public() is true, to the
	 * other clients in the game.
	 * @param id The id of this property. MUST be UNIQUE! Used to send and
	 * receive changes in the property of the playere automatically via
	 * network. TODO: Very ugly - better use something like
	 * parent()->propertyId() or so which assigns a free id automatically.
	 * @param parent The parent of the object. Must be a KGame which manages
	 * the changes made to this object, i.e. which will send the new data
	 **/
	KGameProperty(int id, KGamePropertyHandlerBase* owner) : KGamePropertyBase(id, owner) {}

	/**
	 * This constructor does nothing. You have to call @ref
	 * KGamePropertyBase::registerData
	 * yourself before using the KGameProperty object.
	 **/
	KGameProperty() : KGamePropertyBase() {}

	virtual ~KGameProperty() {}


	/**
	 * Sets the value of this object.
	 * Uses @ref KPlayer::sendProperty to send it to all clients
	 * @param v The value to assign to this object
	 * @param sendValue Specifies whether you want the value to be sent over
	 * network (defaul) or not. Use this e.g. if you send a message to all
	 * clients which sets the values of the properties on receiving. 
	 **/
	void setValue(type v, bool sendValue = true)
	// i am not able to put this to kgameproperty.cpp - why?
	{
		//kdDebug(11001) << "+++KGameProperty::setValue(" << id() << ") = " << v << endl;
		if (!isOptimized() || mData!=v)
    {
			if (isReadOnly()) return;
			mData = v;
			if (isEmittingSignal())
      {
				//AB: cannot be done here!!
				//the remote clients won't receive this signal!
//				emitSignal();
			}
			if (sendValue) { send(); }
		} 
	}

	/**
	 * Saves the object to a stream.
	 * @ref stream The stream to save to
	 **/
	virtual void save(QDataStream &stream)
	{
		stream << mData;
	}

	/**
	 * @return The value of this object
	 **/
	const type& value() const	{ return mData; }

	/**
	 * Reads from a stream and assigns the read value to this object. Does
	 * not use @ref setValue
	 * @param s The stream to read from
	 **/
	virtual void load(QDataStream& s)
	{
	// Hmm, is this really necessary?
//		type oldValue=mData;
		s >> mData;
		if (isEmittingSignal()) {
			emitSignal();//TODO: removed as we set
//		it indirect now. must be emitted elsewhere!!!
		}

//		does not work for non-default types, when the operator "!=" is not
//		implemented:
//		if (oldValue!=mData && isEmittingSignal()) emitSignal();
	}


	/**
	 * Well, look at this code:
	 * <pre>
	 * 	KGameProperty<int> integerData(0, owner);
	 * 	integerData = 100;
	 * 	kdDebug(11001) << integerData.value() << endl;
	 * </pre>
	 * What do you think will be the output?
	 **/
	const type& operator=(const type& t) 
	{ 
		setValue(t); 
		return mData;
	}

	/**
	 * Yeah, you can do it!
	 * <pre>
	 * 	KGameProperty<int> integerData(0, owner);
	 * 	integerData.setValue(100);
	 * 	kdDebug(11001) << integerData << endl;
	 * </pre>
	 * If you don't see it: you don't have to use integerData.value()
	 **/
	operator type() const { return value(); }

private:
	type mData;
};


typedef KGameProperty<int>   KGamePropertyInt;
typedef KGameProperty<unsigned int>   KGamePropertyUInt;
typedef KGameProperty<QString>   KGamePropertyQString;
typedef KGameProperty<Q_INT8>   KGamePropertyBool;


class KGamePropertyHandlerBasePrivate; // wow - what a name ;-)
class KGamePropertyHandlerBase : public QIntDict<KGamePropertyBase>
{
public:
	KGamePropertyHandlerBase();
	~KGamePropertyHandlerBase();

	/**
	 * Main message process function. This has to be called by
	 * the parent's message event handler. If the id of the message
	 * agrees with the id of the handler, the message is extracted 
	 * and processed. Otherwise false is returned.
	 * Example:
	 * <pre>
	 *   if (mProperties.processMessage(stream,msgid)) return ;
	 * </pre>
	 * 
	 * @param stream The data stream containing the message
	 * @param id the message id of the message
	 * @return true on message processed otherwise false
	 **/
	bool processMessage(QDataStream &stream, int id);
	
	/**
	 * @return the id of the handler
	 **/
	int id() const { return mId; }
	
	/**
	 * Adds a @ref KGameProperty property to the handler
	 * @param data the property
	 * @return true on success
	 **/
	bool addProperty(KGamePropertyBase *data);

	/**
	 * Removes a property from the handler
	 * @param data the property
	 * @return true on success
	 **/
	bool removeProperty(KGamePropertyBase *data);

	/**
	 * Loads properties from the datastream
	 *
	 * @param stream the datastream to save from
	 * @return true on success otherwise false
	 **/
	virtual bool load(QDataStream &stream);

	/**
	 * Saves properties into the datastream
	 *
	 * @param stream the datastream to save to
	 * @return true on success otherwise false
	 **/
	virtual bool save(QDataStream &stream);
	
	/**
	 * called by a property to send itself into the
	 * datastream. This call is simply forwarded to
	 * the parent object
	 **/ 
	virtual void sendProperty(QDataStream &s) = 0;

	/**
	 * called by a property to emit a signal 
	 * This call is simply forwarded to
	 * the parent object
	 **/ 
	virtual void emitSignal(KGamePropertyBase *data) = 0;

	void setId(int id)//AB: TODO: make this protected in KGamePropertyHandler!!
	{
		mId = id;
	}

private:
	void init();
	KGamePropertyHandlerBasePrivate* d;
	int mId;
};


template<class type>
class KGamePropertyHandler : public KGamePropertyHandlerBase
{
public:
	/** 
	 * Constructs a KGamePropertyHandler object
	 **/
	KGamePropertyHandler() : KGamePropertyHandlerBase()
	{
		init();
	}

	/**
	 * Just for convenience, same as @ref KGamePropertyHandler
	 **/
	KGamePropertyHandler(int id, type* owner) : KGamePropertyHandlerBase()
	{
		init();
		registerHandler(id, owner);
	}
    
	/**
	 * Register the handler with a parent. This is to use
	 * if the constructor without arguments has been choosen.
	 * Otherwise you need not call this.
	 *
	 * @param id The id of the message to listen for
	 * @param owner the parent object
	 **/
	void registerHandler(int id, type* owner)
	{
		setId(id);
		mOwner = owner;
	}

	/**
	 *  Destruct the KGamePropertyHandler
	 **/
	~KGamePropertyHandler()
	{
	}

	/**
	 * @return the owner of the handler
	 **/
	type* owner() const { return mOwner; }

	/**
	 * called by a property to send itself into the
	 * datastream. This call is simply forwarded to
	 * the parent object
	 **/ 
	void sendProperty(QDataStream &s)
	{
		if (mOwner) {
			mOwner->sendProperty(s);
		}
	}
	
	/**
	 * called by a property to emit a signal 
	 * This call is simply forwarded to
	 * the parent object
	 **/ 
	void emitSignal(KGamePropertyBase *data)
	{
		if (mOwner) {
			mOwner->emitSignal(data);
		}
	}
	
private:
	void init()
	{
		mOwner = 0;
	}
	type* mOwner;
};

#endif
