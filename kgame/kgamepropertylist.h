/*
    This file is part of the KDE games library
    Copyright (C) 2001 Martin Heni (kde at heni-online.de)
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

#ifndef __KGAMEPROPERTYLIST_H_
#define __KGAMEPROPERTYLIST_H_


#include <kdebug.h>

#include "kgamemessage.h"
#include "kgameproperty.h"
#include "kgamepropertyhandler.h"

// AB: also see README.LIB!

template<class type>
class KGamePropertyList : public QList<type>, public KGamePropertyBase
{
public:
     /**
     * Typedefs
     */
    typedef QListIterator<type> Iterator;

  KGamePropertyList() :QList<type>(), KGamePropertyBase()
  {
  }

  KGamePropertyList( const KGamePropertyList<type> &a ) : QList<type>(a)
  {
  }

  uint findIterator(Iterator me)
  {
    Iterator it;
    uint cnt=0;
    for( it = this->begin(); it != this->end(); ++it )
    {
      if (me==it) 
      {
        return cnt;
      }
      cnt++;
    }
    return this->count();
  }

  Iterator insert( Iterator it, const type& d )
  {
    it=QList<type>::insert(it,d);

    QByteArray b;
    QDataStream s(b, QIODevice::WriteOnly);
    KGameMessage::createPropertyCommand(s,KGamePropertyBase::IdCommand,id(),CmdInsert);
    int i=findIterator(it);
    s << i;
    s << d;
    if (policy() == PolicyClean || policy() == PolicyDirty)
    {
      if (mOwner)
      {
        mOwner->sendProperty(s);
      }
    }
    if (policy() == PolicyDirty || policy() == PolicyLocal)
    {
      extractProperty(b);
    }
    return it;
  }

  void  prepend( const type& d) { insert(this->begin(),d); }
  
  void  append( const type& d ) 
  {
    QByteArray b;
    QDataStream s(b, QIODevice::WriteOnly);
    KGameMessage::createPropertyCommand(s,KGamePropertyBase::IdCommand,id(),CmdAppend);
    s << d;
    if (policy() == PolicyClean || policy() == PolicyDirty)
    {
      if (mOwner)
      {
        mOwner->sendProperty(s);
      }
    }
    if (policy() == PolicyDirty || policy() == PolicyLocal)
    {
      extractProperty(b);
    }
  }

  Iterator erase( Iterator it )
  {
    QByteArray b;
    QDataStream s(b, QIODevice::WriteOnly);
    KGameMessage::createPropertyCommand(s,KGamePropertyBase::IdCommand,id(),CmdRemove);
    int i=findIterator(it);
    s << i;
    if (policy() == PolicyClean || policy() == PolicyDirty)
    {
      if (mOwner)
      {
        mOwner->sendProperty(s);
      }
    }
    if (policy() == PolicyDirty || policy() == PolicyLocal)
    {
      extractProperty(b);
    }
    //TODO: return value - is it correct for PolicyLocal|PolicyDirty?
//    return QValueList<type>::remove(it);
    return it;
  }

  Iterator remove( Iterator it )
  {
    return erase(it);
  }

  void remove( const type& d )
  {
    Iterator it=find(d);
    remove(it);
  }

  void clear()
  {
    QByteArray b;
    QDataStream s(b, QIODevice::WriteOnly);
    KGameMessage::createPropertyCommand(s,KGamePropertyBase::IdCommand,id(),CmdClear);
    if (policy() == PolicyClean || policy() == PolicyDirty)
    {
      if (mOwner)
      {
        mOwner->sendProperty(s);
      }
    }
    if (policy() == PolicyDirty || policy() == PolicyLocal)
    {
      extractProperty(b);
    }
  }

  void load(QDataStream& s)
  {
    kDebug(11001) << "KGamePropertyList load" << id();
    QList<type>::clear();
    uint size;
    type data;
    s >> size;

    for (unsigned int i=0;i<size;i++)
    {
      s >> data;
      QList<type>::append(data);
    }
    if (isEmittingSignal()) emitSignal();
  }

  void save(QDataStream &s)
  {
    kDebug(11001) << "KGamePropertyList save "<<id();
    type data;
    uint size=this->count();
    s << size;
    Iterator it;
    for( it = this->begin(); it != this->end(); ++it )
    {
      data=*it;
      s << data;
    }
  }

  void command(QDataStream &stream, int msgid, bool isSender)
  {
    KGamePropertyBase::command(stream, msgid);
    kDebug(11001) << "---> LIST id="<<id()<<" got command ("<<msgid<<") !!!"; 
    Iterator it;
    switch(msgid)
    {
      case CmdInsert:
      {
        uint i;
        type data;
        stream >> i >> data;
        it=this->at(i);
        QList<type>::insert(it,data);
//        kDebug(11001) << "CmdInsert:id="<<id()<<" i="<<i<<" data="<<data; 
        if (isEmittingSignal()) emitSignal();
        break;
      }
      case CmdAppend:
      {
        type data;
	stream >> data;
        QList<type>::append(data);
//        kDebug(11001) << "CmdAppend:id=" << id() << "data=" << data;
        if (isEmittingSignal()) emitSignal();
	break;
      }
      case CmdRemove:
      {
        uint i;
        stream >> i;
        it=this->at(i);
        QList<type>::remove(it);
        kDebug(11001) << "CmdRemove:id="<<id()<<" i="<<i; 
        if (isEmittingSignal()) emitSignal();
        break;
      }
      case CmdClear:
      {
        QList<type>::clear();
        kDebug(11001) << "CmdClear:id="<<id(); 
        if (isEmittingSignal()) emitSignal();
        break;
      }
      default: 
        kDebug(11001) << "Error in KPropertyList::command: Unknown command" << msgid;
    }
  }

protected:
  void extractProperty(const QByteArray& b)
  // this is called for Policy[Dirty|Local] after putting the stuff into the
  // stream
  {
    QDataStream s(b, QIODevice::ReadOnly);
    int cmd;
    int propId;
    KGameMessage::extractPropertyHeader(s, propId);
    KGameMessage::extractPropertyCommand(s, propId, cmd);
    command(s, cmd, true);
  }

};

#endif
