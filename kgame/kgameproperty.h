/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001 Martin Heni (martin@heni-online.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __KGAMEPROPERTY_H_
#define __KGAMEPROPERTY_H_

#include <qdatastream.h>

#include <kdebug.h>
#include <typeinfo>

class KGame;
class KPlayer;
class KGamePropertyHandler;

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
	enum PropertyDataIds  { // these belong to KPlayer/KGame!
		//KPlayer
		IdGroup=1,
		IdName=2,
		IdAsyncInput=3,
		IdTurn=4,
		IdUserId=5,

		//KGame
		IdGameStatus=6,
		IdMaxPlayer=7,
		IdMinPlayer=8,

		IdCommand, // Reserved for internal use
		IdUser=256
	};

	/**
	 * Commands for advanced properties (Q_INT8)
	 **/
	enum PropertyCommandIds 
	{
		// Array
		CmdAt=1,
		CmdResize=2,
		CmdFill=3,
		CmdSort=4,
		// List (could be the same id's actually
		CmdInsert=5,
		CmdRemove=6,
		CmdClear=7
	};

	/**
	 * The policy of the property. This is either clean (@ref setVale uses
	 * @ref send) or dirty (@ref setValue uses @ref changeValue).
	 *
	 * A "clean" policy means that the property is always the same on every
	 * client. This is achieved by calling @ref send which actually changes
	 * the value only when the message from the MessageServer is received.
	 *
	 * A "dirty" policy means that as soon as @ref setValue is called the
	 * property is changed immediately. This can sometimes lead to bugs as
	 * the other clients do not immediately have the same value. For more
	 * information see @ref changeValue
	 **/
	enum PropertyPolicy
	{
		PolicyClean = 1,
		PolicyDirty = 2
	};

	/**
	 * Changes the consistency policy of a property. This is either true (default)
	 * which means a "clean" (always consistent) policy or a "dirty" policy. 
	 *
	 * A clean policy (true) means that you cannot work immediately with the new
	 * value of the property as it has to be received from the Message
	 * Server first.
	 *
	 * On the other hand a dirty policy (false) might introduce evil bugs as the
	 * property has a different value on different clients until the network
	 * value is received.
	 *
	 * It is up to you to decide how you want to work. 
	 **/
	void setAlwaysConsistent(bool c) { mFlags.bits.cleanPolicy=c&1; }

	/**
	 * Constructs a KGamePropertyBase object and calls @ref registerData.
	 * @param id The id of this property. MUST be UNIQUE! Used to send and
	 * receive changes in the property of the playere automatically via
	 * network. 
	 * @param owner The owner of the object. Must be a KGamePropertyHandler which manages
	 * the changes made to this object, i.e. which will send the new data
	 **/
	KGamePropertyBase(int id, KGamePropertyHandler* owner);

	KGamePropertyBase(int id, KGame* parent);
	KGamePropertyBase(int id, KPlayer* parent);

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
	 * See also @ref setAlwaysConsistent
	 * @return TRUE if the property uses a "clean" (always consistent)
	 * policy, otherwise FALSE.
	 **/
	bool isAlwaysConsistent() const { return mFlags.bits.cleanPolicy; }

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
	 **/
	virtual void command(QDataStream & ,int ) {};

	/**
	 * @return The id of this property
	 **/
	int id() const { return mId; }

	virtual const type_info * typeinfo() {return &typeid(this);};

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
	 * using @ref KPropertyHandler::sendProperty whenever you call @ref send
	 *
	 **/
	void registerData(int id, KGamePropertyHandler* owner,QString name=0);

	void registerData(int id, KGame* owner,QString name=0);
	void registerData(int id, KPlayer* owner,QString name=0);

protected:
	//AB: I had problems when putting this into KGameProperty::setValue() as I
	//had to include kplayer.h and kgame.h which caused problems e.g.
	//because they both include kgameproperty.h this member function is a
	//workaround to put the stuff into the .cpp file (as i cannot put
	//anything from KGameProperty into the .cpp file)
	//The problem now is that there is one more function call and could one
	//day be bad for performance
	/**
	 * Forward the data to the owner of this property which then sends it
	 * over network. @ref save is used to store the data into a stream so
	 * you have to make sure that function is working properly if you
	 * implement your own property!
	 **/
	bool sendProperty();
	
	/**
	 * @return TRUE if the message could be sent successfully, otherwise
	 * FALSE
	 **/
	bool sendProperty(const QByteArray& b);
	
	/**
	 * Causes the parent object to emit a signal on value change
	 **/
	void emitSignal();

	KGamePropertyHandler* mOwner;
	
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
			unsigned char cleanPolicy : 1; // whether the property is always consistent (true)
		} bits;
	} mFlags;
	
private:
	void init();
	
	bool mPublic;
	int mId;
};

/**
 * The class KGameProperty can store any form of data and will transmit it via
 * network whenver you call @ref send. This makes network transparent games
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
 * s << type;
 * s << suite;
 * return s;
 * }
 * QDataStream& operator>>(QDataStream& stream, Card& card)
 * {
 * Q_INT16 type;
 * Q_INT16 suite;
 * s >> type;
 * s >> suite;
 * card.type = (int)type;
 * card.suite = (int)suite;
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
	KGameProperty(int id, KGamePropertyHandler* owner) : KGamePropertyBase(id, owner) { init(); }

	/**
	 * This constructor does nothing. You have to call @ref
	 * KGamePropertyBase::registerData
	 * yourself before using the KGameProperty object.
	 **/
	KGameProperty() : KGamePropertyBase() { init(); }

	virtual ~KGameProperty() {}

	/**
	 * Set the value depending on the current policy (see @ref
	 * setConsistent). By default KGameProperty just uses @ref send to set
	 * the value of a property. This behaviour can be changed by using @ref
	 * setConsistent and using this funcition.
	 **/
	void setValue(type v)
	{
		if (isAlwaysConsistent()) {
			send(v);
		} else {
			changeValue(v);
		}
	}


	/**
	 * This is the central function for changing the value of a
	 * KGameProperty. You can usually call myProperty = myValue; but
	 * sometimes you might want to call this function directly.
	 *
	 * Note that the value DOES NOT change when you call this function. This
	 * function saves the value into a @ref QDataStream and calls @ref
	 * sendProperty where it gets forwarded to the owner and finally the
	 * value is sent over network. The @ref KMessageServer now send the
	 * value to ALL clients - even the one who called this function. As soon
	 * as the value from the message server is received @ref load is called
	 * and _then_ the value of the KGameProperty has been set.
	 *
	 * This way ensures that a KGameProperty has _always_ the same value on
	 * _every_ client in the network. Note that this means you can NOT do
	 * something like
	 * <pre>
	 * myProperty = 1;
	 * doSomething(myProperty);
	 * </pre>
	 * as myProperty has not yet been set when doSomething is being called.
	 * You are informed about a value change by a singal from the parent of
	 * the property which can be deactivated by @ref setEmittingSignal because of
	 * performance (you probably don't have to deactivate it - except you
	 * want to write a real-time game like command&conquer with a lot of
	 * acitvity). See @ref emitSignal
	 *
	 * Sometimes you want to go the "dirty" way and have a property set
	 * imediately even if that means your local client differs from all
	 * others. You can do this by using @ref setLocal or @ref changeValue.
	 * Please try to avoid these functions - this will save you a lot of
	 * trouble one day.
	 * @see setLocal changeValue value localValue emitSignal
	 * setEmittingSignal load
	 * 
	 * @param sendOnly If FALSE and the value couldn't be sent by any reason
	 * (e.g. because no owner has been set who can send the value) then the
	 * local value is set using @ref setLocal. If TRUE then the value is
	 * sent only and nothing is done if sending is not possible
	 **/
	void send(type v, bool sendOnly = false)
	{
		if (!isOptimized() || mData!=v) {
			if (isReadOnly()) {
				return;
			}
			QByteArray b;
			QDataStream stream(b, IO_WriteOnly);
			stream << v;
			if (!sendProperty(b) && !sendOnly) {
				setLocal(v);
			}
		} 
	}

	/**
	 * This function is a workaround for a big problem of KProperty: if you
	 * do e.g.
	 * <pre>
	 * myProperty = 1;
	 * doSomething(myProperty);
	 * </pre>
	 * then this will fail. The myProperty = 1 part calls @ref send which
	 * sends the value over network. But it doesn't actually set the value
	 * of the property - this is done as soon as the @ref KMessageServer
	 * sends this message to this client. So doSomething(myProperty) still
	 * contains the old value (i.e. NOT 1).
	 *
	 * To solve this problem you can call setLocal(). This function creates
	 * a duplicated version of the property which is ONLY locally available.
	 * The value is NOT sent over network. 
	 *
	 * As soon as a value over network is received the local duplication is
	 * deleted again. Note that this behaviour might be disturbing or even
	 * confusing!
	 *
	 * If you want to set the value locally AND send it over network you
	 * want to call @ref changeValue!
	 *
	 * @see send changeValue value localValue
	 **/
	void setLocal(type v) 
	{
		kdDebug(11001) << "setLocal" << endl;
		if (!mLocalData) {
			mLocalData = new type;
		}
		if (!isOptimized() || *mLocalData != v) {
			if (isReadOnly()) {
				return;
			}
			*mLocalData = v;
		}
	}

	/**
	 * Sometime you have to call the network value (i.e. the value that is
	 * set by @ref send) on your own, so without sending it. You can do this
	 * with initData but note that you should _only_ do this when the @ref
	 * KMessageServer is not available. This is usually the case in the
	 * constructor only
	 *
	 * Do not call this if there is another possibility! You should only
	 * call this to set the initial value!
	 *
	 * Better use @ref setLocal if possible
	 **/
	void initData(type v)
	{
		mData = v;
	}

	/**
	 * This function does both, change the local value and change the
	 * network value. The local value is created/changed first, then the
	 * value is sent over network.
	 *
	 * This function is a convenience function and just calls @ref setLocal
	 * followed by @ref send
	 *
	 * @see send setLocal value localValue
	 **/
	void changeValue(type v)
	{
		setLocal(v);
		send(v);
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
	 * @return The local value (see @ref setLocal) if it is existing,
	 * otherwise the same as @ref value.
	 **/
	const type& value() const
  {
    if (mLocalData) return *mLocalData;
    else return mData;
  }

	/**
	 * You usually don't want to call this but rather @ref value
	 * @return The network value which is __always__ consistent among all
	 * clients. 
	 **/
	const type& networkValue() const { return mData; }

	/**
	 * Reads from a stream and assigns the read value to this object. This
	 * also deletes an existing local value (see @ref setLocal and @ref
	 * localValue).
	 *
	 * This function is called automatically when a new value is received
	 * over network (i.e. it has been sent using @ref send on this or any
	 * other client) or when a game is loaded (and maybe on some other
	 * events).
	 * @param s The stream to read from
	 **/
	virtual void load(QDataStream& s)
	{
		if (mLocalData) {
			delete mLocalData;
			mLocalData = 0;
		}
		s >> mData;
		if (isEmittingSignal()) {
			emitSignal();
		}
	}

	/**
	 * This calls @ref setValue to change the value of the property. Not
	 * that depending on the policy (see @ref setAlwaysConsistent) the
	 * returned value might be different from the assigned value!!
	 *
	 * So if you use @ref setAlwaysConsistent(false):
	 * <pre>
	 * int a, b = 10;
	 * myProperty = b;
	 * a = myProperty.value();
	 * </pre>
	 * Here a and b would differ (except if myProperty.value() has been 10
	 * before)!
	 * The value is actually set as soon as it is received form the @ref
	 * KMessageServer which forwards it to ALL clients in the network.
	 *
	 * If you use a clean policy (i.e. @ref alwaysConsistent == true) then
	 * the returned value is the assigned value
	 * @return See @ref value
	 **/
	const type& operator=(const type& t) 
	{ 
		setValue(t); 
		return value();
	}

	/**
	 * Yeah, you can do it!
	 * <pre>
	 * 	KGameProperty<int> integerData(0, owner);
	 * 	integerData.setValue(100);
	 * 	kdDebug(11001) << integerData << endl;
	 * </pre>
	 * If you don't see it: you don't have to use integerData.value()
	 * @return See @ref value
	 **/
	operator type() const { return value(); }

  virtual const type_info * typeinfo() {return &typeid(type);};

private:
	void init()
	{
		mLocalData = 0;
	}
		
private:
	type mData;
	type* mLocalData;
};


typedef KGameProperty<int>   KGamePropertyInt;
typedef KGameProperty<unsigned int>   KGamePropertyUInt;
typedef KGameProperty<QString>   KGamePropertyQString;
typedef KGameProperty<Q_INT8>   KGamePropertyBool;

#endif
