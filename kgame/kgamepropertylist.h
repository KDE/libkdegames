/* **************************************************************************
                           KGamePropertyList Class
                           -------------------
    begin                : 21 June 2001
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
#ifndef __KGAMEPROPERTYLIST_H_
#define __KGAMEPROPERTYLIST_H_

#include <qdatastream.h>
#include <qvaluelist.h>
#include <kdebug.h>
#include "kgamemessage.h"
#include "kgameproperty.h"


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
      if (me==it) return cnt;
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
    if (mOwner)  mOwner->sendProperty(s);
    return it;
  }
  void  prepend( const type& d) { insert(begin(),d); }
  void  append( const type& d ) { insert(end(),d); }

  Iterator remove( Iterator it )
  {
    QByteArray b;
    QDataStream s(b, IO_WriteOnly);
    KGameMessage::createPropertyCommand(s,KGamePropertyBase::IdCommand,id(),CmdRemove);
    int i=findIterator(it);
    s << i;
    if (mOwner)  mOwner->sendProperty(s);
    QValueList<type>::remove(it);
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
    QValueList<type>::clear();
  }

	void load(QDataStream& s)
	{
    kdDebug() << "KGamePropertyList load " << id() << endl;
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
    kdDebug() << "KGamePropertyList save "<<id() << endl;
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

  void command(QDataStream &s,int cmd)
  {
    kdDebug() << "---> LIST id="<<id()<<" got command ("<<cmd<<") !!!" <<endl; 
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
        kdDebug() << "CmdInsert:id="<<id()<<" i="<<i<<" data="<<data <<endl; 
        if (isEmittingSignal()) emitSignal();
        break;
      }
      case CmdRemove:
      {
        uint i;
        s >> i;
        it=at(i);
        QValueList<type>::remove(it);
        kdDebug() << "CmdRemove:id="<<id()<<" i="<<i <<endl; 
        if (isEmittingSignal()) emitSignal();
        break;
      }
      case CmdClear:
      {
        QValueList<type>::clear();
        kdDebug() << "CmdClear:id="<<id()<<endl; 
        if (isEmittingSignal()) emitSignal();
        break;
      }
      default: 
        kdDebug() << "Error in KPropertyList::command: Unknown command " << cmd << endl;
    }
  }


};

#endif
