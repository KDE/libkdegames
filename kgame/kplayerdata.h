/* **************************************************************************
                           KPlayerData Class
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
#ifndef __KPLAYERDATA_H_
#define __KPLAYERDATA_H_

#include <qdatastream.h>
#include <qintdict.h>
#include <kdebug.h>

class KGame;
class KPlayer;
class KPlayerDataHandler;

/**
 * The KPlayerDataBase class is the base class of KPlayerData. See @ref
 * KPlayerData for further information.
 * 
 * @short Base class of KPlayerData
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class KPlayerDataBase
{
public:
	enum PlayerDataIds  { // these belong to KPlayer!
		IdGroup=1,
		IdName=2,
		IdAsyncInput=3,
		IdTurn=4,
		IdGameStatus=5,
		IdMaxPlayer=6,
		IdMinPlayer=7,
		IdUserId=8,
		IdUser=256
	};

	/**
	 * Constructs a KPlayerDataBase object and calls @ref registerData.
	 * @param id The id of this property. MUST be UNIQUE! Used to send and
	 * receive changes in the property of the playere automatically via
	 * network. 
	 * @param owner The owner of the object. Must be a KPlayerDataHandler which manages
	 * the changes made to this object, i.e. which will send the new data
	 **/
	KPlayerDataBase(int id, KPlayerDataHandler* owner);

	/**
	 * Creates a KPlayerDataBase object without an owner. Remember to call
	 * @ref registerData!
	 **/
	KPlayerDataBase();

	virtual ~KPlayerDataBase();

	/**
	 * Sets this property to public or non-public. Public means that all
	 * players receive the new value when it has changed. 
	 * NOTE: non-public use is a TODO
	 **/
	void setPublic(bool p)	{ mFlags.bits.ispublic=p&1; }

	/**
	 * See also @ref setPublic
	 * NOTE: non-public use is a TODO
	 * @return Whether this is a public property
	 **/
	bool isPublic()	const { return mFlags.bits.ispublic; }
	
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
	 * See also @ref setLocked
	 * @return Whether the property can be changed
	 **/
	bool isLocked() const { return mFlags.bits.locked; }

	/**
	 * A locked property cannot be changed. Use this if you to prevent a
	 * player from changing something, e.g. for a money-bases card game you
	 * will want to lock the "bet" property after a player has bet.
	 * 
	 * You have to call setLocked(false) before you are able to change the
	 * value of the property again. The default is not locked.
	 * @param p True to lock this property, false to unlock it
	 **/
	void setLocked(bool p) { mFlags.bits.locked = p&1; }
	

	/**
	 * This will read the value of this property from the stream. You MUST
	 * overwrite this method in order to use this class
	 * @param s The stream to read from
	 **/
	virtual void load(QDataStream& s) = 0;
	virtual void save(QDataStream& s) = 0;

	/**
	 * @return The id of this property
	 **/
	int id() const { return mId; }

	/**
	 * You have to register a KPlayerDataBase before you can use it.
	 *
	 * You MUST call this before you can use KPlayerDataBase!
	 *
	 * @param id the id of this KPlayerDataBase object. The id MUST be
	 * unique, i.e. you cannot have two properties with the same id for one
	 * player, although (currently) nothing prevents you from doing so. But
	 * you will get strange results!
	 * @param owner The owner of this data. This player will send the data
	 * using @ref KPlayer::sendProperty whenever you call @ref setValue
	 *
	 **/
	void registerData(int id, KPlayerDataHandler* owner);

protected:
	//AB: I had problems when putting this into KPlayerData::setValue() as I
	//had to include kplayer.h and kgame.h which caused problems e.g.
	//because they both include kplayerdata.h this member function is a
	//workaround to put the stuff into the .cpp file (as i cannot put
	//anything from KPlayerData into the .cpp file)
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

		KPlayerDataHandler* mOwner;
	
	// Having this as a union of the bitfield and the char
	// allows us to stream this quantity easily (if we need to)
	// At the moment it is not yet transmitted
	union Flags {
		char flag;
		struct {
			unsigned char ispublic : 1;   // is Public
			// unsigned char dosave : 1;   // do save this property
			// unsigned char delaytransmit : 1;   // do not send immediately on
                                             // change but a KPlayer:QTimer
                                             // sends it later on - fast
                                             // changing variables
			unsigned char emitsignal:1; // KPlayer notifies on variable change
						// can used 2 more
			unsigned char locked : 1; // whether the property can be changed
		} bits;
	} mFlags;
	
private:
	void init();
	
	bool mPublic;
	int mId;
};

/**
 * The class KPlayerData can store any form of data and will transmit it via
 * network whenver you call @ref setValue. This makes network transparent games
 * very easy. You first have to register the data to a @ref KPlayer using @ref
 * KPlayerDataBase::registerData (which is called by the constructor)
 *
 * If you want to use a custum class with KPlayerData you have to implement the
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
 * KPlayerData<Card> mCards;
 * };
 * </pre>
 *
 *
 * Note: unlike most QT classes KPlayerData objects are *not* deleted
 * automatically! So if you create an object using e.g. KPlayerData<int>* data =
 * new KPlayerData(0, this) you have to put a delete data into your destructor!
 * @short A class for network transparent games
 **/
template<class type>
class KPlayerData  : public KPlayerDataBase
{

public:
	/**
	 * Constructs a KPlayerData object. A KPlayerData object will transmit
	 * any changes to the server/master and, if public() is true, to the
	 * other clients in the game.
	 * @param id The id of this property. MUST be UNIQUE! Used to send and
	 * receive changes in the property of the playere automatically via
	 * network. TODO: Very ugly - better use something like
	 * parent()->propertyId() or so which assigns a free id automatically.
	 * @param parent The parent of the object. Must be a KGame which manages
	 * the changes made to this object, i.e. which will send the new data
	 **/
	KPlayerData(int id, KPlayerDataHandler* owner) : KPlayerDataBase(id, owner) {}

	/**
	 * This constructor does nothing. You have to call @ref
	 * KPlayerDataBase::registerData
	 * yourself before using the KPlayerData object.
	 **/
	KPlayerData() : KPlayerDataBase() {}

	virtual ~KPlayerData() {}


	/**
	 * Sets the value of this object.
	 * Uses @ref KPlayer::sendProperty to send it to all clients
	 * @param v The value to assign to this object
	 * @param sendValue Specifies whether you want the value to be sent over
	 * network (defaul) or not. Use this e.g. if you send a message to all
	 * clients which sets the values of the properties on receiving. 
	 **/
	void setValue(type v, bool sendValue = true)
	// i am not able to put this to kplayerdata.cpp - why?
	{
		//kdDebug(11001) << "+++KPlayerData::setValue(" << id() << ") = " << v << endl;
		if (mData!=v) { // not possible as "!=" is not always implemented
			if (isLocked()) {
				return;
			}
			mData = v;
			if (isEmittingSignal()) {
				emitSignal();
			}
			if (sendValue) {
				send();
			}
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
//		if (isEmittingSignal()) emitSignal();//TODO: removed as we set
//		it indirect now. must be emitted elsewhere!!!

//		does not work for non-default types, when the operator "!=" is not
//		implemented:
//		if (oldValue!=mData && isEmittingSignal()) emitSignal();
	}


	/**
	 * Well, look at this code:
	 * <pre>
	 * 	KPlayerData<int> integerData(0, owner);
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
	 * 	KPlayerData<int> integerData(0, owner);
	 * 	integerData.setValue(100);
	 * 	kdDebug(11001) << integerData << endl;
	 * </pre>
	 * If you don't see it: you don't have to use integerData.value()
	 **/
	operator type() const { return value(); }

private:
	type mData;
};


typedef KPlayerData<int>   KPlayerDataInt;
typedef KPlayerData<unsigned int>   KPlayerDataUInt;
typedef KPlayerData<QString>   KPlayerDataQString;
typedef KPlayerData<Q_INT8>   KPlayerDataBool;



class KPlayerDataHandler : public QIntDict<KPlayerDataBase>
{
  public:
    /** 
     * Constructs a KPlayerDataHandler object
     **/
    KPlayerDataHandler();
    /**
     * Just for convenience, same as @ref KPlayerDataHandler
     **/
    KPlayerDataHandler(int id, KPlayer *parent);
    /**
     * Just for convenience, same as @ref KPlayerDataHandler
     *
     * @param The id of the message to listen for
     * @param the parent object
     *
     **/
    KPlayerDataHandler(int id, KGame *parent);
    /**
     * Just for convenience, same as @ref KPlayerDataHandler
     *
     * @param The id of the message to listen for
     * @param the parent object
     **/
    KPlayerDataHandler(int id, void *parent,int type);
    
    /**
     * Register the handler with a parent. This is to use
     * if the constructor without arguments has been choosen.
     * Otherwise you need not call this.
     *
     * @param The id of the message to listen for
     * @param the parent object
     **/
    void registerHandler(int id, KPlayer *parent);
    /**
     * Same as @ref registerHandler but with different
     * parent class
     *
     * @param The id of the message to listen for
     * @param the parent object
     **/
    void registerHandler(int id, KGame *parent);
    /**
     * Same as @ref registerHandler but with different
     * parent class
     *
     * @param The id of the message to listen for
     * @param the parent object
     **/
    void registerHandler(int id, void *parent,int type);
    /**
     *  Destruct the KPlayerDataHandler
     **/
    ~KPlayerDataHandler();

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
     * @param The data stream containing the message
     * @param the message id of the message
     * @return true on message processed otherwise false
     *
     **/
    bool processMessage(QDataStream &stream, int id);
    /**
     * returns the type of the handler 
     **/
    int type() const {return mType;}
    /**
     * returns the id of the handler
     **/
    int id() const {return mId;}
    /**
     * returns the owner of the handler
     **/
    void *owner() const {return mOwner;}
    /**
     * Adds a KPlayerData property to the handler
     *
     * @param the property
     * @return true on success
     **/
    bool addProperty(KPlayerDataBase *data);
    /**
     * Removes a property from the handler
     *
     * @param the property
     * @return true on success
     */
    bool removeProperty(KPlayerDataBase *data);
    /**
     * called by a property to send itself into the
     * datastream. This call is simply forwarded to
     * the parent object
     **/ 
    virtual void sendProperty(QDataStream &s,bool isPublic=true);
    /**
     * called by a property to emit a signal 
     * This call is simply forwarded to
     * the parent object
     **/ 
    virtual void emitSignal(KPlayerDataBase *data);
    /**
     * Loads properties from the datastream
     *
     * @param the datastream
     * @return true on success
     **/
    virtual bool load(QDataStream &stream);
    /**
     * Saves properties into the datastream
     *
     * @param the datastream
     * @return true on success
     **/
    virtual bool save(QDataStream &stream);
  private:
    int mType;
    void *mOwner;
    int mId;

};

#endif
