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

/**
 * The KGamePropertyHandler class is some kind of a collection class for
 * KGameProperty. You usually don't have to create one yourself, as both @ref
 * KPlayer and @ref KGame provide a handler. In most cases you do not even have
 * to care about the KGamePropertHandler. @ref KGame and @ref KPlayer implement
 * all features of KGamePropertyHandler so you will rather use it there.
 *
 * You have to use the KGamePropertyHandler as parent for all @ref KGameProperty
 * objects but you can also use @ref KPlayer or @ref KGame as parent - then @ref
 * KPlayer::dataHandler or @ref KGame::dataHandler will be used. 
 *
 * Every KGamePropertyHandler must have - just like every @ref KGameProperty -
 * a unique ID. This ID is provided either in the constructor or in @ref
 * registerHandler. The ID is used to assign an incoming message (e.g. a changed
 * property) to the correct handler. Inside the handler the property ID is used
 * to change the correct property. 
 *
 * The constructor or @ref registerHandler takes 3 addittional arguments: a
 * receiver and two slots. The first slot is connected to @ref
 * signalSendMessage, the second to @ref signalPropertyChanged. You must provide
 * these in order to use the KGamePropertyHandler. 
 *
 * The most important function of KGamePropertyHandler is @ref processMessage
 * which assigns an incoming value to the correct property. 
 *
 * A KGamePropertyHandler is also used - indirectly using @ref emitSignal - to
 * emit a signal when the value of a property changes. This is done this way
 * because a @ref KGameProperty does not inherit @ref QObject because of memory
 * advantages. Many games can have dozens or even hundreds of @ref KGameProperty
 * objects so every additional variable in @ref KGameProperty would be
 * multiplied. 
 *
 * @short A collection class for @ref KGameProperty objects
 **/
class KGamePropertyHandler : public QObject
{
  Q_OBJECT

public:
	/**
	 * Construct an unregistered KGamePropertyHandler
	 *
	 * You have to call @ref registerHandler before you can use this
	 * handler!
	 **/
	KGamePropertyHandler(QObject* parent = 0);

	/**
	 * Construct a registered handler. 
	 *
	 * See also @ref registerHandler
	 **/
	KGamePropertyHandler(int id, const QObject* receiver, const char* sendf, const char* emitf, QObject* parent = 0);
	~KGamePropertyHandler();

	/**
	 * Register the handler with a parent. This is to use
	 * if the constructor without arguments has been choosen.
	 * Otherwise you need not call this.
	 *
	 * @param id The id of the message to listen for
	 * @param owner the parent object
	 * @param receiver The object that will receive the signals of
	 * KGamePropertyHandler
	 * @param send A slot that is being connected to @ref signalSendMessage
	 * @param emit A slot that is being connected to @ref
	 * signalPropertyChanged
	 **/
	void registerHandler(int id, const QObject *receiver, const char * send, const char *emit); 

	/**
	 * Main message process function. This has to be called by
	 * the parent's message event handler. If the id of the message
	 * agrees with the id of the handler, the message is extracted 
	 * and processed. Otherwise false is returned.
	 * Example:
	 * <pre>
	 *   if (mProperties.processMessage(stream,msgid,sender==gameId())) return ;
	 * </pre>
	 * 
	 * @param stream The data stream containing the message
	 * @param id the message id of the message
	 * @param isSender Whether the receiver is also the sender
	 * @return true on message processed otherwise false
	 **/
	bool processMessage(QDataStream &stream, int id, bool isSender );
	
	/**
	 * @return the id of the handler
	 **/
	int id() const;
	
	/**
	 * Adds a @ref KGameProperty property to the handler
	 * @param data the property
	 * @param name A description of the property, which will be returned by
	 * @ref propertyName. This is used for debugging, e.g. in @ref
	 * KGameDebugDialog
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
   * returns a unique property ID starting called usually with a base of
   * @ref KGamePropertyBase::IdAutomatic. This is used internally by
   * the property base to assign automtic id's. Not much need to
   * call this yourself.
   *
   */
  int uniquePropertyId();


	/**
	 * Loads properties from the datastream
	 *
	 * @param stream the datastream to load from
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
	bool sendProperty(QDataStream &s);

	void sendLocked(bool l);

	/**
	 * called by a property to emit a signal 
	 * This call is simply forwarded to
	 * the parent object
	 **/ 
	void emitSignal(KGamePropertyBase *data);

	/**
	 * @param id The ID of the property
	 * @return A name of the property which can be used for debugging. Don't
	 * depend on this function! It it possible not to provide a name or to
	 * provide the same name for multiple properties!
	 **/
	QString propertyName(int id) const;

	/**
	 * @param id The ID of the property. See @ref KGamePropertyBase::id
	 * @return The @ref KGameProperty this ID is assigned to
	 **/
	KGamePropertyBase *find(int id);

	/**
	 * Clear the KGamePropertyHandler. Note that the properties are
	 * <em>not</em> deleted so if you created your @ref KGameProperty
	 * objects dynamically like
	 * <pre>
	 * KGamePropertyInt* myProperty = new KGamePropertyInt(id, dataHandler());
	 * </pre>
	 * you also have to delete it:
	 * <pre>
	 * dataHandler()->clear();
	 * delete myProperty;
	 * </pre>
	 **/
	void clear();

	/**
	 * Use id as new ID for this KGamePropertyHandler. This is used
	 * internally only.
	 **/
	void setId(int id);//AB: TODO: make this protected in KGamePropertyHandler!!

	/**
	 * Calls @ref KGamePropertyBase::setReadOnly(false) for all properties of this
	 * player. See also @ref lockProperties
	 **/
	void unlockProperties();

	/**
     * Set the policy for all kgame variables which are currently registerd in
     * the KGame proeprty handler. See @ref KGamePropertyBase::setPolicy
     *
     * @param p is the new policy for all properties of this handler
     * @param is userspace=true (default) only user properties are changed.
     * Otherwise also the system properties
	 **/
	void setPolicy(int p, bool userspace=true);

	/**
	 * Calls @ref KGamePropertyBase::setReadOnly(true) for all properties of this
	 * handler
	 *
	 * Use with care! This will even lock the core properties, like name,
	 * group and myTurn!!
	 *
	 * See also @ref unlockProperties
	 **/
	void lockProperties();

  /**
   * Sends all properties which are marked dirty over the network. This will
   * make a forced synchornisation of the properties and mark them all not dirty.
   **/
  void flush();

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
	/**
	 * This is emitted by a property. @ref KGamePropertyBase::emitSignal
	 * calls @ref emitSignal which emits this signal. 
	 *
	 * This signal is emitted whenever the property is changed. Note that
	 * you can switch off this behaviour using @ref
	 * KGamePropertyBase::setEmittingSignal in favor of performance. Note
	 * that you won't experience any performance loss using signals unless
	 * you use dozens or hundreds of properties which change very often.
	 **/
	void signalPropertyChanged(KGamePropertyBase *);

	/**
	 * This signal is emitted when a property needs to be sent. Only the
	 * parent has to react to this.
	 * @param sent set this to true if the property was sent successfully -
	 * otherwise don't touch
	 **/
	void signalSendMessage(QDataStream &, bool& sent);

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
};

#endif
