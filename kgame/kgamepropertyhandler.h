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

#include <qobject.h>
#include <qintdict.h>

class QDataStream;
class KGame;
class KPlayer;
class KGamePropertyBase;

class KGamePropertyHandlerPrivate; // wow - what a name ;-)

class KGamePropertyHandler : public QObject
{
  Q_OBJECT

public:
	KGamePropertyHandler(QObject* parent = 0);
	KGamePropertyHandler(int id,const QObject * receiver, const char * sendf, const char *emitf, QObject* parent = 0);
	~KGamePropertyHandler();

	/**
	 * Register the handler with a parent. This is to use
	 * if the constructor without arguments has been choosen.
	 * Otherwise you need not call this.
	 *
	 * @param id The id of the message to listen for
	 * @param owner the parent object
	 **/
	void registerHandler(int id,const QObject *receiver, const char * send, const char *emit); 

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
	bool addProperty(KGamePropertyBase *data, QString name=0);

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
	void sendProperty(QDataStream &s);

	/**
	 * called by a property to emit a signal 
	 * This call is simply forwarded to
	 * the parent object
	 **/ 
	void emitSignal(KGamePropertyBase *data);

	QString propertyName(int id) const;

	KGamePropertyBase *find(int id);
	void clear();

	void setId(int id)//AB: TODO: make this protected in KGamePropertyHandler!!
	{
		mId = id;
	}

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
	 * Reference to the internal dictionary
	 **/
	QIntDict<KGamePropertyBase> &dict() const;

	/**
	 * In several situations you just want to have a @ref QString of a @ref
	 * KGameProperty object. This is e.g. the case in the @ref
	 * KGameDebugDialog where the value of all properties is displayed. This
	 * function will provide you with such a @ref QString for all the types
	 * used inside of all @ref KGame classes. If you have a non-standard
	 * property (probably a self defined class or something like this) you
	 * also need to connect to @ref signalRequestValue to make this function
	 * useful.
	 * @ref property Return the value of this @ref KGameProperty 
	 * @return The value of a @ref KGameProperty
	 **/
	QString propertyValue(KGamePropertyBase* property);

signals:
      void signalPropertyChanged(KGamePropertyBase *);
      void signalSendMessage(QDataStream &);

      /**
       * If you call @ref propertyValue with a non-standard @ref KGameProperty
       * it is possible that the value cannot automatically be converted into a
       * @ref QString. Then this signal is emitted and asks you to provide the
       * correct value. You probably want to use something like this to achieve
       * this:
       * <pre>
       * #include <typeinfo>
       * void slotRequestValue(KGamePropertyBase* p, QString& value)
       * {
       * 	if (*(p->typeinfo()) == typeid(MyType) {
       * 		value = QString(((KGameProperty<MyType>*)p)->value());
       * 	}
       * }
       * </pre>
       *
       * @ref property The @ref KGamePropertyBase the value is requested for
       * @ref value The value of this property. You have to set this.
       **/
      void signalRequestValue(KGamePropertyBase* property, QString& value);

private:
	void init();

private:
	KGamePropertyHandlerPrivate* d;
	int mId;
};

#endif
