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
/*
    $Id$
*/

#include "kgamepropertyhandler.h"
#include "kgameproperty.h"
#include "kgamemessage.h"

#include <qmap.h>
#include <qptrqueue.h>

#include <klocale.h>
#include <typeinfo>

#define KPLAYERHANDLER_LOAD_COOKIE 6239

//---------------------- KGamePropertyHandler -----------------------------------
class KGamePropertyHandlerPrivate
{
public:
	KGamePropertyHandlerPrivate()
	{
	}

	QMap<int, QString> mNameMap;
	QIntDict<KGamePropertyBase> mIdDict;
	int mUniqueId;
	int mId;
	KGamePropertyBase::PropertyPolicy mDefaultPolicy;
	bool mDefaultUserspace;
  int mIndirectEmit;
  QPtrQueue<KGamePropertyBase> mSignalQueue;
};

KGamePropertyHandler::KGamePropertyHandler(int id, const QObject* receiver, const char * sendf, const char *emitf, QObject* parent) : QObject(parent)
{
 init();
 registerHandler(id,receiver,sendf,emitf);
}

KGamePropertyHandler::KGamePropertyHandler(QObject* parent) : QObject(parent)
{
 init();
}

KGamePropertyHandler::~KGamePropertyHandler()
{
 clear();
 delete d;
}

void KGamePropertyHandler::init()
{
 kdDebug(11001) << k_funcinfo << ": this=" << this << endl;
 d = new KGamePropertyHandlerPrivate; // for future use - is BC important to us?
 d->mId = 0;
 d->mUniqueId=KGamePropertyBase::IdAutomatic;
 d->mDefaultPolicy=KGamePropertyBase::PolicyLocal;
 d->mDefaultUserspace=true;
 d->mIndirectEmit=0;
}


int KGamePropertyHandler::id() const
{
 return d->mId;
}

void KGamePropertyHandler::setId(int id)
{
 d->mId = id;
}

void KGamePropertyHandler::registerHandler(int id,const QObject * receiver, const char * sendf, const char *emitf)
{
 setId(id); 
 if (receiver && sendf) {
	kdDebug(11001) << "Connecting SLOT " << sendf << endl;
	connect(this, SIGNAL(signalSendMessage(int, QDataStream &, bool*)), receiver, sendf);
 }
 if (receiver && emitf) {
	kdDebug(11001) << "Connecting SLOT " << emitf << endl;
	connect(this, SIGNAL(signalPropertyChanged(KGamePropertyBase *)), receiver, emitf);
 }
}

bool KGamePropertyHandler::processMessage(QDataStream &stream, int id, bool isSender)
{
// kdDebug(11001) << k_funcinfo << ": id=" << id << " mId=" << d->mId << endl;
 if (id != d->mId) {
 	return false; // Is the message meant for us?
 }
 KGamePropertyBase* p;
 int propertyId;
 KGameMessage::extractPropertyHeader(stream, propertyId);
// kdDebug(11001) << k_funcinfo << ": Got property " << propertyId << endl;
 if (propertyId==KGamePropertyBase::IdCommand) {
	int cmd;
	KGameMessage::extractPropertyCommand(stream, propertyId, cmd);
//kdDebug(11001) << k_funcinfo << ": Got COMMAND for id= "<<propertyId <<endl;
	p = d->mIdDict.find(propertyId);
	if (p) {
		if (!isSender || p->policy()==KGamePropertyBase::PolicyClean) {
			p->command(stream, cmd, isSender);
		}
	} else {
		kdError(11001) << k_funcinfo << ": (cmd): property " << propertyId << " not found" << endl;
	}
	return true;
 }
 p = d->mIdDict.find(propertyId);
 if (p) {
	//kdDebug(11001) << k_funcinfo << ": Loading " << propertyId << endl;
	if (!isSender || p->policy()==KGamePropertyBase::PolicyClean) {
		p->load(stream);
	}
 } else {
	kdError(11001) << k_funcinfo << ": property " << propertyId << " not found" << endl;
 }
 return true;
}

bool KGamePropertyHandler::removeProperty(KGamePropertyBase* data)
{
 if (!data) {
	return false;
 }
 d->mNameMap.erase(data->id());
 return d->mIdDict.remove(data->id());
}

bool KGamePropertyHandler::addProperty(KGamePropertyBase* data, QString name)
{
 //kdDebug(11001) << k_funcinfo << ": " << data->id() << endl;
 if (d->mIdDict.find(data->id())) {
	// this id already exists
	kdError(11001) << "  -> cannot add property " << data->id() << endl;
	return false;
 } else {
	d->mIdDict.insert(data->id(), data);
  // if here is a check for "is_debug" or so we can add the strings only in debug mode
  // and save memory!!
	if (!name.isNull()) {
		d->mNameMap[data->id()] = name;
		//kdDebug(11001) << k_funcinfo << ": nid="<< (data->id()) << " inserted in Map name=" << d->mNameMap[data->id()] <<endl;
		//kdDebug(11001) << "Typeid=" << typeid(data).name() << endl;
  	//kdDebug(11001) << "Typeid call=" << data->typeinfo()->name() << endl;
	}
 }
 return true;
}

QString KGamePropertyHandler::propertyName(int id) const
{
 QString s;
 if (d->mIdDict.find(id)) {
	if (d->mNameMap.contains(id)) {
		s = i18n("%1 (%2)").arg(d->mNameMap[id]).arg(id);
	} else {
		s = i18n("Unnamed - ID: %1").arg(id);
	}
 } else {
	// Should _never_ happen
	s = i18n("%1 unregistered").arg(id);
 }
 return s;
}

bool KGamePropertyHandler::load(QDataStream &stream)
{
 // Prevent direct emmiting until all is loaded
 lockDirectEmit();
 uint count,i;
 stream >> count;
 kdDebug(11001) << k_funcinfo << ": " << count << " KGameProperty objects " << endl;
 for (i = 0; i < count; i++) {
	processMessage(stream, id(),false);
 }
 Q_INT16 cookie;
 stream >> cookie;
 if (cookie == KPLAYERHANDLER_LOAD_COOKIE) {
	kdDebug(11001) << "   KGamePropertyHandler loaded propertly"<<endl;
 } else {
	kdError(11001) << "KGamePropertyHandler loading error. probably format error"<<endl;
 }
 // Allow direct emmiting (if no other lock still holds)
 unlockDirectEmit();
 return true;
}

bool KGamePropertyHandler::save(QDataStream &stream)
{
 kdDebug(11001) << k_funcinfo << ": " << d->mIdDict.count() << " KGameProperty objects " << endl;
 stream << (uint)d->mIdDict.count();
 QIntDictIterator<KGamePropertyBase> it(d->mIdDict);
 while (it.current()) {
	KGamePropertyBase *base=it.current();
	if (base) {
		KGameMessage::createPropertyHeader(stream, base->id());
		base->save(stream);
	}
	++it;
 }
 stream << (Q_INT16)KPLAYERHANDLER_LOAD_COOKIE;
 return true;
}

KGamePropertyBase::PropertyPolicy KGamePropertyHandler::policy()
{
// kdDebug(11001) << k_funcinfo << ": " << d->mDefaultPolicy << endl;
 return d->mDefaultPolicy;
}
void KGamePropertyHandler::setPolicy(KGamePropertyBase::PropertyPolicy p,bool userspace)
{
 // kdDebug(11001) << k_funcinfo << ": " << p << endl;
 d->mDefaultPolicy=p;
 d->mDefaultUserspace=userspace;
 QIntDictIterator<KGamePropertyBase> it(d->mIdDict);
 while (it.current()) {
	if (!userspace || it.current()->id()>=KGamePropertyBase::IdUser) {
		it.current()->setPolicy((KGamePropertyBase::PropertyPolicy)p);
	}
	++it;
 }
}

void KGamePropertyHandler::unlockProperties()
{
 QIntDictIterator<KGamePropertyBase> it(d->mIdDict);
 while (it.current()) {
	it.current()->unlock();
	++it;
 }
}

void KGamePropertyHandler::lockProperties()
{
 QIntDictIterator<KGamePropertyBase> it(d->mIdDict);
 while (it.current()) {
	it.current()->lock();
	++it;
 }
}

int KGamePropertyHandler::uniquePropertyId()
{
 return d->mUniqueId++;
}

void KGamePropertyHandler::flush()
{
 QIntDictIterator<KGamePropertyBase> it(d->mIdDict);
 while (it.current()) {
	if (it.current()->isDirty()) {
		it.current()->sendProperty();
	}
	++it;
 }
}

/* Fire all property signal changed which are collected in
 * the queque
 **/
void KGamePropertyHandler::lockDirectEmit()
{
  d->mIndirectEmit++;
}

void KGamePropertyHandler::unlockDirectEmit()
{
  // If the flag is <=0 we emit the queued signals
  d->mIndirectEmit--;
  if (d->mIndirectEmit<=0)
  {
    KGamePropertyBase *prop;
    while((prop=d->mSignalQueue.dequeue()) != 0)
    {
      // kdDebug(11001) << "emmiting signal for " << prop->id() << endl;
      emit signalPropertyChanged(prop);
    }
  }
}

void KGamePropertyHandler::emitSignal(KGamePropertyBase *prop)
{
 // If the indirect flag is set (load and network transmit)
 // we cannot emit the signals directly as it can happend that
 // a sigal causes an access to a property which is e.g. not
 // yet loaded or received

 if (d->mIndirectEmit>0)
 {
  // Queque the signal
  d->mSignalQueue.enqueue(prop);
 }
 else
 {
  // directly emit
  emit signalPropertyChanged(prop);
 }
}

bool KGamePropertyHandler::sendProperty(QDataStream &s)
{
 bool sent = false;
 emit signalSendMessage(id(), s, &sent);
 return sent;
}

KGamePropertyBase *KGamePropertyHandler::find(int id)
{
 return d->mIdDict.find(id);
}

void KGamePropertyHandler::clear()
{
 kdDebug(11001) << k_funcinfo << id() << endl;
 QIntDictIterator<KGamePropertyBase> it(d->mIdDict);
 while (it.toFirst()) {
	KGamePropertyBase* p = it.toFirst();
	p->unregisterData();
	if (d->mIdDict.find(p->id())) {
		// shouldn't happen - but if mOwner in KGamePropertyBase is NULL
		// this might be possible
		removeProperty(p); 
	}
 }
}

QIntDict<KGamePropertyBase>& KGamePropertyHandler::dict() const
{ 
 return d->mIdDict; 
}

QString KGamePropertyHandler::propertyValue(KGamePropertyBase* prop)
{
 if (!prop) {
         return i18n("NULL pointer");
 }
	   
 int id = prop->id();
 QString name = propertyName(id);
 QString value;

 const type_info* t = prop->typeinfo();
 if (*t == typeid(int)) {
	value = QString::number(((KGamePropertyInt*)prop)->value());
 } else if (*t == typeid(unsigned int)) {
	value = QString::number(((KGamePropertyUInt *)prop)->value());
 } else if (*t == typeid(long int)) {
	value = QString::number(((KGameProperty<long int> *)prop)->value()); 
 } else if (*t == typeid(unsigned long int)) {
	value = QString::number(((KGameProperty<unsigned long int> *)prop)->value());
 } else if (*t == typeid(QString)) { 
	value = ((KGamePropertyQString*)prop)->value();
 } else if (*t == typeid(Q_INT8)) { 
	value = ((KGamePropertyBool*)prop)->value() ?  i18n("True") : i18n("False");
 } else {
	emit signalRequestValue(prop, value);
 }
		   
 if (value.isNull()) {
	value = i18n("Unknown");
 }
 return value;
}

void KGamePropertyHandler::Debug()
{
 kdDebug(11001) << "-----------------------------------------------------------" << endl;
 kdDebug(11001) << "KGamePropertyHandler:: Debug this=" << this << endl;

 kdDebug(11001) << "  Registered properties: (Policy,Lock,Emit,Optimized, Dirty)" << endl;
 QIntDictIterator<KGamePropertyBase> it(d->mIdDict);
 while (it.current()) {
	KGamePropertyBase *p=it.current();
	kdDebug(11001) << "  "<< p->id() << ": p=" << p->policy() 
			<< " l="<<p->isLocked()
			<< " e="<<p->isEmittingSignal() 
			<< " o=" << p->isOptimized() 
			<< " d="<<p->isDirty() 
			<< endl;
	++it;
 }
 kdDebug(11001) << "-----------------------------------------------------------" << endl;
}

#include "kgamepropertyhandler.moc"
