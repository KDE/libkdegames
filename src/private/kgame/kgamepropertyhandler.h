/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>
    SPDX-FileCopyrightText: 2001 Martin Heni <kde at heni-online.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef __KGAMEPROPERTYHANDLER_H_
#define __KGAMEPROPERTYHANDLER_H_

// own
#include "kgameproperty.h"
#include "libkdegamesprivate_export.h"
// Qt
#include <QMultiHash>
#include <QObject>
// Std
#include <memory>

class QDataStream;
class KGame;
// class KGamePropertyBase;

class KGamePropertyHandlerPrivate; // wow - what a name ;-)

/**
 * \class KGamePropertyHandler kgamepropertyhandler.h <KGame/KGamePropertyHandler>
 *
 * @short A collection class for KGameProperty objects
 *
 * The KGamePropertyHandler class is some kind of a collection class for
 * KGameProperty. You usually don't have to create one yourself, as both
 * KPlayer and KGame provide a handler. In most cases you do not even have
 * to care about the KGamePropertHandler. KGame and KPlayer implement
 * all features of KGamePropertyHandler so you will rather use it there.
 *
 * You have to use the KGamePropertyHandler as parent for all KGameProperty
 * objects but you can also use KPlayer or KGame as parent - then
 * KPlayer::dataHandler or KGame::dataHandler will be used.
 *
 * Every KGamePropertyHandler must have - just like every KGameProperty -
 * a unique ID. This ID is provided either in the constructor or in
 * registerHandler. The ID is used to assign an incoming message (e.g. a changed
 * property) to the correct handler. Inside the handler the property ID is used
 * to change the correct property.
 *
 * The constructor or registerHandler takes 3 additional arguments: a
 * receiver and two slots. The first slot is connected to
 * signalSendMessage, the second to signalPropertyChanged. You must provide
 * these in order to use the KGamePropertyHandler.
 *
 * The most important function of KGamePropertyHandler is processMessage
 * which assigns an incoming value to the correct property.
 *
 * A KGamePropertyHandler is also used - indirectly using emitSignal - to
 * emit a signal when the value of a property changes. This is done this way
 * because a KGameProperty does not inherit QObject because of memory
 * advantages. Many games can have dozens or even hundreds of KGameProperty
 * objects so every additional variable in KGameProperty would be
 * multiplied.
 */
class KDEGAMESPRIVATE_EXPORT KGamePropertyHandler : public QObject
{
    Q_OBJECT

public:
    /**
     * Construct an unregistered KGamePropertyHandler
     *
     * You have to call registerHandler before you can use this
     * handler!
     */
    explicit KGamePropertyHandler(QObject *parent = nullptr);

    /**
     * Construct a registered handler.
     *
     * @see registerHandler
     */
    KGamePropertyHandler(int id, const QObject *receiver, const char *sendf, const char *emitf, QObject *parent = nullptr);
    ~KGamePropertyHandler() override;

    /**
     * Register the handler with a parent. This is to use
     * if the constructor without arguments has been chosen.
     * Otherwise you need not call this.
     *
     * @param id The id of the message to listen for
     * @param receiver The object that will receive the signals of
     * KGamePropertyHandler
     * @param send A slot that is being connected to signalSendMessage
     * @param emit A slot that is being connected to signalPropertyChanged
     */
    void registerHandler(int id, const QObject *receiver, const char *send, const char *emit);

    /**
     * Main message process function. This has to be called by
     * the parent's message event handler. If the id of the message
     * agrees with the id of the handler, the message is extracted
     * and processed. Otherwise false is returned.
     * Example:
     * \code
     *   if (mProperties.processMessage(stream,msgid,sender==gameId())) return ;
     * \endcode
     *
     * @param stream The data stream containing the message
     * @param id the message id of the message
     * @param isSender Whether the receiver is also the sender
     * @return true on message processed otherwise false
     */
    bool processMessage(QDataStream &stream, int id, bool isSender);

    /**
     * @return the id of the handler
     */
    int id() const;

    /**
     * Adds a KGameProperty property to the handler
     * @param data the property
     * @param name A description of the property, which will be returned by
     * propertyName. This is used for debugging
     * @return true on success
     */
    bool addProperty(KGamePropertyBase *data, const QString &name = QString());

    /**
     * Removes a property from the handler
     * @param data the property
     * @return true on success
     */
    bool removeProperty(KGamePropertyBase *data);

    /**
     * returns a unique property ID starting called usually with a base of
     * KGamePropertyBase::IdAutomatic. This is used internally by
     * the property base to assign automatic id's. Not much need to
     * call this yourself.
     */
    int uniquePropertyId();

    /**
     * Loads properties from the datastream
     *
     * @param stream the datastream to load from
     * @return true on success otherwise false
     */
    virtual bool load(QDataStream &stream);

    /**
     * Saves properties into the datastream
     *
     * @param stream the datastream to save to
     * @return true on success otherwise false
     */
    virtual bool save(QDataStream &stream);

    /**
     * called by a property to send itself into the
     * datastream. This call is simply forwarded to
     * the parent object
     */
    bool sendProperty(QDataStream &s);

    void sendLocked(bool l);

    /**
     * called by a property to emit a signal
     * This call is simply forwarded to
     * the parent object
     */
    void emitSignal(KGamePropertyBase *data);

    /**
     * @param id The ID of the property
     * @return A name of the property which can be used for debugging. Don't
     * depend on this function! It it possible not to provide a name or to
     * provide the same name for multiple properties!
     */
    QString propertyName(int id) const;

    /**
     * @param id The ID of the property. See KGamePropertyBase::id
     * @return The KGameProperty this ID is assigned to
     */
    KGamePropertyBase *find(int id);

    /**
     * Clear the KGamePropertyHandler. Note that the properties are
     * <em>not</em> deleted so if you created your KGameProperty
     * objects dynamically like
     * \code
     * KGamePropertyInt* myProperty = new KGamePropertyInt(id, dataHandler());
     * \endcode
     * you also have to delete it:
     * \code
     * dataHandler()->clear();
     * delete myProperty;
     * \endcode
     */
    void clear();

    /**
     * Use id as new ID for this KGamePropertyHandler. This is used
     * internally only.
     */
    void setId(int id); // AB: TODO: make this protected in KGamePropertyHandler!!

    /**
     * Calls KGamePropertyBase::setReadOnly(false) for all properties of this
     * player. See also lockProperties
     */
    void unlockProperties();

    /**
     * Set the policy for all kgame variables which are currently registered in
     * the KGame property handler. See KGamePropertyBase::setPolicy
     *
     * @param p is the new policy for all properties of this handler
     * @param userspace if userspace is true (default) only user properties are changed.
     * Otherwise the system properties are also changed.
     */
    void setPolicy(KGamePropertyBase::PropertyPolicy p, bool userspace = true);

    /**
     * Called by the KGame or KPlayer object or the handler itself to delay
     * emitting of signals. Locking keeps a counter and unlock is only achieved
     * when every lock is canceled by an unlock.
     * While this is set signals are queued and only emitted after this
     * is reset. Its deeper meaning is to prevent inconsistencies in a game
     * load or network transfer where a emit could access a property not
     * yet loaded or transmitted. Calling this by yourself you better know
     * what your are doing.
     */
    void lockDirectEmit();

    /**
     * Removes the lock from the emitting of property signals. Corresponds to
     * the lockIndirectEmits
     */
    void unlockDirectEmit();

    /**
     * Returns the default policy for this property handler. All properties
     * registered newly, will have this property.
     */
    KGamePropertyBase::PropertyPolicy policy();

    /**
     * Calls KGamePropertyBase::setReadOnly(true) for all properties of this
     * handler
     *
     * Use with care! This will even lock the core properties, like name,
     * group and myTurn!!
     *
     * @see unlockProperties
     */
    void lockProperties();

    /**
     * Sends all properties which are marked dirty over the network. This will
     * make a forced synchronization of the properties and mark them all not dirty.
     */
    void flush();

    /**
     * Reference to the internal dictionary
     */
    QMultiHash<int, KGamePropertyBase *> &dict() const;

    /**
     * In several situations you just want to have a QString of a
     * KGameProperty object. This
     * function will provide you with such a QString for all the types
     * used inside of all KGame classes. If you have a non-standard
     * property (probably a self defined class or something like this) you
     * also need to connect to signalRequestValue to make this function
     * useful.
     * @param property Return the value of this KGameProperty
     * @return The value of a KGameProperty
     */
    QString propertyValue(KGamePropertyBase *property);

    /**
     * Writes some debug output to the console.
     */
    void Debug();

Q_SIGNALS:
    /**
     * This is emitted by a property. KGamePropertyBase::emitSignal
     * calls emitSignal which emits this signal.
     *
     * This signal is emitted whenever the property is changed. Note that
     * you can switch off this behaviour using
     * KGamePropertyBase::setEmittingSignal in favor of performance. Note
     * that you won't experience any performance loss using signals unless
     * you use dozens or hundreds of properties which change very often.
     */
    void signalPropertyChanged(KGamePropertyBase *);

    /**
     * This signal is emitted when a property needs to be sent. Only the
     * parent has to react to this.
     * @param msgid The id of the handler
     * @param sent set this to true if the property was sent successfully -
     * otherwise don't touch
     */
    void signalSendMessage(int msgid, QDataStream &, bool *sent); // AB shall we change bool* into bool& again?

    /**
     * If you call propertyValue with a non-standard KGameProperty
     * it is possible that the value cannot automatically be converted into a
     * QString. Then this signal is emitted and asks you to provide the
     * correct value. You probably want to use something like this to achieve
     * this:
     * \code
     * #include <typeinfo>
     * void slotRequestValue(KGamePropertyBase* p, QString& value)
     * {
     * 	if (*(p->typeinfo()) == typeid(MyType) {
     * 		value = QString(((KGameProperty<MyType>*)p)->value());
     * 	}
     * }
     * \endcode
     *
     * @param property The KGamePropertyBase the value is requested for
     * @param value The value of this property. You have to set this.
     */
    void signalRequestValue(KGamePropertyBase *property, QString &value);

private:
    friend class KGamePropertyHandlerPrivate;
    std::unique_ptr<KGamePropertyHandlerPrivate> const d;

    Q_DISABLE_COPY(KGamePropertyHandler)
};

#endif
