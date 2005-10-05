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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef __KGAMEPROPERTYARRAY_H_
#define __KGAMEPROPERTYARRAY_H_

#include <qdatastream.h>
//Added by qt3to4:
#include <QVector>
#include <kdebug.h>

#include "kgamemessage.h"
#include "kgameproperty.h"
#include "kgamepropertyhandler.h"


template<class type>
class KGamePropertyArray : public QVector<type>, public KGamePropertyBase
{
public:
  KGamePropertyArray() :QVector<type>(), KGamePropertyBase()
  {
    //kdDebug(11001) << "KGamePropertyArray init" << endl;
  }
  
  KGamePropertyArray( int size )
  {
    resize(size);
  }
  
  KGamePropertyArray( const KGamePropertyArray<type> &a ) : QVector<type>(a)
  {
  }
  
  bool  resize( uint size )
  {
    if (size!=QVector<type>::size())
    {
      bool a=true;
      QByteArray b;
      QDataStream s(&b, QIODevice::WriteOnly);
      KGameMessage::createPropertyCommand(s,KGamePropertyBase::IdCommand,id(),CmdResize);
      s << size ;
      if (policy()==PolicyClean || policy()==PolicyDirty)
      {
        if (mOwner)
        {
          mOwner->sendProperty(s);
        }
      }
      if (policy()==PolicyLocal || policy()==PolicyDirty)
      {
        extractProperty(b);
//        a=QMemArray<type>::resize(size);// FIXME: return value!
      }
      return a;
    }
    else return true;
  }

  void setAt(uint i,type data)
  {
    QByteArray b;
    QDataStream s(&b, QIODevice::WriteOnly);
    KGameMessage::createPropertyCommand(s,KGamePropertyBase::IdCommand,id(),CmdAt);
    s << i ;
    s << data;
    if (policy()==PolicyClean || policy()==PolicyDirty)
    {
      if (mOwner)
      {
        mOwner->sendProperty(s);
      }
    }
    if (policy()==PolicyLocal || policy()==PolicyDirty)
    {
      extractProperty(b);
    }
    //kdDebug(11001) << "KGamePropertyArray setAt send COMMAND for id="<<id() << " type=" << 1 << " at(" << i<<")="<<data  << endl;
  }

  type at( uint i ) const
  {
    return QVector<type>::at(i);
  }
  
  type operator[]( int i ) const
  {
    return QVector<type>::at(i);
  }

  KGamePropertyArray<type> &operator=(const KGamePropertyArray<type> &a)
  {
    return assign(a);
  }

  bool  truncate( uint pos )
  {
    return resize(pos);
  }
  
  bool  fill( const type &data, int size = -1 )
  {
    bool r=true;
    QByteArray b;
    QDataStream s(&b, QIODevice::WriteOnly);
    KGameMessage::createPropertyCommand(s,KGamePropertyBase::IdCommand,id(),CmdFill);
    s << data;
    s << size ;
    if (policy()==PolicyClean || policy()==PolicyDirty)
    {
      if (mOwner)
      {
        mOwner->sendProperty(s);
      }
    }
    if (policy()==PolicyLocal || policy()==PolicyDirty)
    {
      extractProperty(b);
//      r=QMemArray<type>::fill(data,size);//FIXME: return value!
    }
    return r;
  }

  KGamePropertyArray<type>& assign( const KGamePropertyArray<type>& a )
  {
// note: send() has been replaced by sendProperty so it might be broken now!
    if (policy()==PolicyClean || policy()==PolicyDirty)
    {
      sendProperty();
    }
    if (policy()==PolicyLocal || policy()==PolicyDirty)
    {
      QVector<type>::assign(a);
    }
    return *this;
  }
  KGamePropertyArray<type>& assign( const type *a, uint n )
  {
    if (policy()==PolicyClean || policy()==PolicyDirty)
    {
      sendProperty();
    }
    if (policy()==PolicyLocal || policy()==PolicyDirty)
    {
      QVector<type>::assign(a,n);
    }
    return *this;
  }
  KGamePropertyArray<type>& duplicate( const KGamePropertyArray<type>& a )
  {
    if (policy()==PolicyClean || policy()==PolicyDirty)
    {
      sendProperty();
    }
    if (policy()==PolicyLocal || policy()==PolicyDirty)
    {
      QVector<type>::duplicate(a);
    }
    return *this;
  }
  KGamePropertyArray<type>& duplicate( const type *a, uint n )
  {
    if (policy()==PolicyClean || policy()==PolicyDirty)
    {
      sendProperty();
    }
    if (policy()==PolicyLocal || policy()==PolicyDirty)
    {
      QVector<type>::duplicate(a,n);
    }
    return *this;
  }
  KGamePropertyArray<type>& setRawData( const type *a, uint n )
  {
    if (policy()==PolicyClean || policy()==PolicyDirty)
    {
      sendProperty();
    }
    if (policy()==PolicyLocal || policy()==PolicyDirty)
    {
      QVector<type>::setRawData(a,n);
    }
    return *this;
  }
  void sort()
  {
    QByteArray b;
    QDataStream s(b, QIODevice::WriteOnly);
    KGameMessage::createPropertyCommand(s,KGamePropertyBase::IdCommand,id(),CmdSort);
    if (policy()==PolicyLocal || policy()==PolicyDirty)
    {
      if (mOwner)
      {
        mOwner->sendProperty(s);
      }
    }
    if (policy()==PolicyLocal || policy()==PolicyDirty)
    {
      extractProperty(b);
    }
  }

  void load(QDataStream& s)
  {
    //kdDebug(11001) << "KGamePropertyArray load " << id() << endl;
    type data;
    for (unsigned int i=0; i<QVector<type>::size(); i++) 
    {
      s >> data;
      QVector<type>::replace(i,data);
    }
    if (isEmittingSignal())
    {
      emitSignal();
    }
  }
  void save(QDataStream &s)
  {
    //kdDebug(11001) << "KGamePropertyArray save "<<id() << endl;
    for (unsigned int i=0; i<QVector<type>::size(); i++) 
    {
      s << at(i);
    }
  }

  void command(QDataStream &s,int cmd,bool)
  {
    KGamePropertyBase::command(s, cmd);
    //kdDebug(11001) << "Array id="<<id()<<" got command ("<<cmd<<") !!!" <<endl; 
    switch(cmd)
    {
      case CmdAt:
      {
        uint i;
        type data;
        s >> i >> data;
        QVector<type>::at(i)=data;
        //kdDebug(11001) << "CmdAt:id="<<id()<<" i="<<i<<" data="<<data <<endl; 
        if (isEmittingSignal()) 
        {
          emitSignal();
        }
        break;
      }
      case CmdResize:
      {
        uint size;
        s >> size;
        //kdDebug(11001) << "CmdResize:id="<<id()<<" oldsize="<<QMemArray<type>::size()<<" newsize="<<size <<endl; 
        if (QVector<type>::size() != size)
        {
          QVector<type>::resize(size);
        }
        break;
      }
      case CmdFill:
      {
        int size;
        type data;
        s >> data >> size;
        //kdDebug(11001) << "CmdFill:id="<<id()<<"size="<<size <<endl; 
        QVector<type>::fill(data,size);
        if (isEmittingSignal()) 
        {
          emitSignal();
        }
        break;
      }
      case CmdSort:
      {
        //kdDebug(11001) << "CmdSort:id="<<id()<<endl; 
        QVector<type>::sort();
        break;
      }
      default: 
        kdError(11001) << "Error in KPropertyArray::command: Unknown command " << cmd << endl;
        break;
    }
  }
protected:
  void extractProperty(const QByteArray& b)
  {
	QByteArray _b(b);
    QDataStream s(&_b, QIODevice::ReadOnly);
    int cmd;
    int propId;
    KGameMessage::extractPropertyHeader(s, propId);
    KGameMessage::extractPropertyCommand(s, propId, cmd);
    command(s, cmd, true);
  }

};

#endif
