/*
    This file is part of the KDE games library
    Copyright (C) 2001 Martin Heni (martin@heni-online.de)
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __KGAMEPROPERTYLIST_H_
#define __KGAMEPROPERTYLIST_H_

#include <qvaluelist.h>

#include <kdebug.h>

#include "kgamemessage.h"
#include "kgameproperty.h"
#include "kgamepropertyhandler.h"

// AB: also see README.LIB!

template<class type>
class KGamePropertyList : public QValueList<type>, public KGamePropertyBase
{
public:
     /**
     * Typedefs
     */
    typedef QValueListIterator<type> Iterator;
    typedef QValueListConstIterator<type> ConstIterator;

  KGamePropertyList() :QValueList<type>(), KGamePropertyBase()
  {
  }

  KGamePropertyList( const KGamePropertyList<type> &a ) : QValueList<type>(a)
  {
    send();
  }

  uint findIterator(Iterator me)
  {
    Iterator it;
    uint cnt=0;
    for( it = begin(); it != end(); ++it )
    {
      if (me==it) 
      {
        return cnt;
      }
      cnt++;
    }
    return count();
  }

  Iterator insert( Iterator it, const type& d )
  {
    it=QValueList<type>::insert(it,d);

    QByteArray b;
    QDataStream s(b, IO_WriteOnly);
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
      command(s, CmdInsert, true);
    }
    return it;
  }

  void  prepend( const type& d) { insert(begin(),d); }
  
  void  append( const type& d ) 
  {
    QByteArray b;
    QDataStream s(b, IO_WriteOnly);
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
      command(s, CmdAppend, true);
    }
  }

  Iterator erase( Iterator it )
  {
    QByteArray b;
    QDataStream s(b, IO_WriteOnly);
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
      command(s, CmdRemove, true);
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
    QDataStream s(b, IO_WriteOnly);
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
      command(s, CmdClear, true);
    }
  }

  void load(QDataStream& s)
  {
    kdDebug(11001) << "KGamePropertyList load " << id() << endl;
    QValueList<type>::clear();
    uint size;
    type data;
    s >> size;

    for (unsigned int i=0;i<size;i++)
    {
      s >> data;
      QValueList<type>::append(data);
    }
    if (isEmittingSignal()) emitSignal();
  }

  void save(QDataStream &s)
  {
    kdDebug(11001) << "KGamePropertyList save "<<id() << endl;
    type data;
    uint size=count();
    s << size;
    Iterator it;
    for( it = begin(); it != end(); ++it )
    {
      data=*it;
      s << data;
    }
  }

  void command(QDataStream &s,int cmd,bool)
  {
    KGamePropertyBase::command(s, cmd);
    kdDebug(11001) << "---> LIST id="<<id()<<" got command ("<<cmd<<") !!!" <<endl; 
    Iterator it;
    switch(cmd)
    {
      case CmdInsert:
      {
        uint i;
        type data;
        s >> i >> data;
        it=at(i);
        QValueList<type>::insert(it,data);
//        kdDebug(11001) << "CmdInsert:id="<<id()<<" i="<<i<<" data="<<data <<endl; 
        if (isEmittingSignal()) emitSignal();
        break;
      }
      case CmdAppend:
      {
        type data;
	s >> data;
        QValueList<type>::append(data);
//        kdDebug(11001) << "CmdAppend:id=" << id() << " data=" << data << endl; 
        if (isEmittingSignal()) emitSignal();
	break;
      }
      case CmdRemove:
      {
        uint i;
        s >> i;
        it=at(i);
        QValueList<type>::remove(it);
        kdDebug(11001) << "CmdRemove:id="<<id()<<" i="<<i <<endl; 
        if (isEmittingSignal()) emitSignal();
        break;
      }
      case CmdClear:
      {
        QValueList<type>::clear();
        kdDebug(11001) << "CmdClear:id="<<id()<<endl; 
        if (isEmittingSignal()) emitSignal();
        break;
      }
      default: 
        kdDebug(11001) << "Error in KPropertyList::command: Unknown command " << cmd << endl;
    }
  }


};

#endif
