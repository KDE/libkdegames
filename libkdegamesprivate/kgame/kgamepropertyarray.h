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

#ifndef __KGAMEPROPERTYARRAY_H_
#define __KGAMEPROPERTYARRAY_H_

#include <QtCore/QDataStream>
//Added by qt3to4:
#include <QtCore/QVector>
#include <kdebug.h>

#include "kgamemessage.h"
#include "kgameproperty.h"
#include "kgamepropertyhandler.h"

/**
 * \class KGamePropertyArray kgamepropertyarray.h <KGamePropertyArray>
 */
template<class type>
class KGamePropertyArray : public QVector<type>, public KGamePropertyBase
{
public:
  KGamePropertyArray() :QVector<type>(), KGamePropertyBase()
  {
    //kDebug(11001) << "KGamePropertyArray init";
  }

  KGamePropertyArray( int size )
  {
    resize(size);
  }

  KGamePropertyArray( const KGamePropertyArray<type> &a ) : QVector<type>(a)
  {
  }

  bool  resize( int size )
  {
    if (size!= QVector<type>::size())
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

  void setAt(int i,type data)
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
    //kDebug(11001) << "KGamePropertyArray setAt send COMMAND for id="<<id() << "type=" << 1 << "at(" << i<<")="<<data;
  }

  const type& at( int i ) const
  {
    return QVector<type>::at(i);
  }

  const type& operator[]( int i ) const
  {
    return QVector<type>::operator[](i);
  }

  type& operator[]( int i )
  {
    return QVector<type>::operator[](i);
  }

  KGamePropertyArray<type> &operator=(const KGamePropertyArray<type> &a)
  {
    return assign(a);
  }

  bool  truncate( int pos )
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
  KGamePropertyArray<type>& assign( const type *a, int n )
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
  KGamePropertyArray<type>& duplicate( const type *a, int n )
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
  KGamePropertyArray<type>& setRawData( const type *a, int n )
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

  void load(QDataStream& s)
  {
    //kDebug(11001) << "KGamePropertyArray load" << id();
    type data;
    for (int i=0; i<QVector<type>::size(); i++)
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
    //kDebug(11001) << "KGamePropertyArray save "<<id();
    for (int i=0; i<QVector<type>::size(); i++)
    {
      s << at(i);
    }
  }

  void command(QDataStream &stream,int msgid, bool isSender)
  {
    Q_UNUSED(isSender);
    KGamePropertyBase::command(stream, msgid);
    //kDebug(11001) << "Array id="<<id()<<" got command ("<<msgid<<") !!!";
    switch(msgid)
    {
      case CmdAt:
      {
        uint i;
        type data;
        stream >> i >> data;
        QVector<type>::replace( i, data );
        //kDebug(11001) << "CmdAt:id="<<id()<<" i="<<i<<" data="<<data;
        if (isEmittingSignal())
        {
          emitSignal();
        }
        break;
      }
      case CmdResize:
      {
        uint size;
        stream >> size;
        //kDebug(11001) << "CmdResize:id="<<id()<<" oldsize="<<QMemArray<type>::size()<<" newsize="<<size;
        if (( uint )QVector<type>::size() != size)
        {
          QVector<type>::resize(size);
        }
        break;
      }
      case CmdFill:
      {
        int size;
        type data;
        stream >> data >> size;
        //kDebug(11001) << "CmdFill:id="<<id()<<"size="<<size;
        QVector<type>::fill(data,size);
        if (isEmittingSignal())
        {
          emitSignal();
        }
        break;
      }
      case CmdSort:
      {
        //kDebug(11001) << "CmdSort:id="<<id();
        qSort( *this );
        break;
      }
      default:
        kError(11001) << "Error in KPropertyArray::command: Unknown command" << msgid;
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
