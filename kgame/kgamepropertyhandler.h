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

#ifndef __KGAMEPROPERTYHANDLER_H_
#define __KGAMEPROPERTYHANDLER_H_

#include <qdatastream.h>
#include <qintdict.h>
#include <qmap.h>

#include <kdebug.h>

#include "kgameproperty.h"

class KGame;
class KPlayer;
class KGamePropertyHandlerBase;

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
	bool addProperty(KGamePropertyBase *data,QString name=0);

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
	virtual bool sendProperty(QDataStream &s) = 0;

	/**
	 * called by a property to emit a signal 
	 * This call is simply forwarded to
	 * the parent object
	 **/ 
	virtual void emitSignal(KGamePropertyBase *data) = 0;

  QString propertyName(int id);

	void setId(int id)//AB: TODO: make this protected in KGamePropertyHandler!!
	{
		mId = id;
	}

private:
	void init();
	KGamePropertyHandlerBasePrivate* d;
	int mId;
  QMap<int,QString> mNameMap;
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
	 * @return TRUE if the message could be sent otherwise FALSE
	 **/ 
	bool sendProperty(QDataStream &s)
	{
		if (mOwner) {
			return mOwner->sendProperty(s);
		} else {
			return false;
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
