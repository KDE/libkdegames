/* **************************************************************************
                           KGamePropertyArray Class
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
#ifndef __KGAMEPROPERTYARRAY_H_
#define __KGAMEPROPERTYARRAY_H_

#include <qdatastream.h>
#include <kdebug.h>
#include "kgamemessage.h"
#include "kgameproperty.h"


template<class type>
class KGamePropertyArray : public QArray<type>, public KGamePropertyBase
{
public:
  KGamePropertyArray() :QArray<type>(), KGamePropertyBase()
  {
    //kdDebug() << "KGamePropertyArray init" << endl;
  }
  KGamePropertyArray( int size )
  {
    resize(size);
  }
  KGamePropertyArray( const KGamePropertyArray<type> &a ) : QArray<type>(a)
  {
    send();
  }
  bool  resize( uint size )
  {
    if (size!=QArray<type>::size())
    {
      QByteArray b;
      QDataStream s(b, IO_WriteOnly);
      KGameMessage::createPropertyCommand(s,KGamePropertyBase::IdCommand,id(),CmdResize);
      s << size ;
      if (mOwner)  mOwner->sendProperty(s);
      bool a=QArray<type>::resize(size);
      return a;
    }
    else return true;
  }

  void setAt(uint i,type data)
  {
    QByteArray b;
    QDataStream s(b, IO_WriteOnly);
    KGameMessage::createPropertyCommand(s,KGamePropertyBase::IdCommand,id(),CmdAt);
    s << i ;
    s << data;
    if (mOwner)  mOwner->sendProperty(s);
    QArray<type>::at(i)=data;
    //kdDebug() << "KGamePropertyArray setAt send COMMAND for id="<<id() << " type=" << 1 << " at(" << i<<")="<<data  << endl;
  }

  type at( uint i ) const
  {
    return QArray<type>::at(i);
  }
  type operator[]( int i ) const
  {
    return QArray<type>::at(i);
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
    bool r;
    r=QArray<type>::fill(data,size);
    QByteArray b;
    QDataStream s(b, IO_WriteOnly);
    KGameMessage::createPropertyCommand(s,KGamePropertyBase::IdCommand,id(),CmdFill);
    s << data;
    s << size ;
    if (mOwner)  mOwner->sendProperty(s);
    return r;
  }
  KGamePropertyArray<type>& assign( const KGamePropertyArray<type>& a )
  {
    QArray<type>::assign(a);
    send();
    return *this;
  }
  KGamePropertyArray<type>& assign( const type *a, uint n )
  {
    QArray<type>::assign(a,n);
    send();
    return *this;
  }
  KGamePropertyArray<type>& duplicate( const KGamePropertyArray<type>& a )
  {
    QArray<type>::duplicate(a);
    send();
    return *this;
  }
  KGamePropertyArray<type>& duplicate( const type *a, uint n )
  {
    QArray<type>::duplicate(a,n);
    send();
    return *this;
  }
  KGamePropertyArray<type>& setRawData( const type *a, uint n )
  {
    QArray<type>::setRawData(a,n);
    send();
    return *this;
  }
  void sort()
  {
    QArray<type>::sort();
    QByteArray b;
    QDataStream s(b, IO_WriteOnly);
    KGameMessage::createPropertyCommand(s,KGamePropertyBase::IdCommand,id(),CmdSort);
    if (mOwner)  mOwner->sendProperty(s);
  }

	void load(QDataStream& s)
	{
    kdDebug() << "KGamePropertyArray load " << id() << endl;
    type data;
    for (unsigned int i=0;i<QArray<type>::size();i++) {s >> data;  QArray<type>::at(i)=data;}
		if (isEmittingSignal()) emitSignal();
  }
	void save(QDataStream &s)
	{
    kdDebug() << "KGamePropertyArray save "<<id() << endl;
    for (unsigned int i=0;i<QArray<type>::size();i++) s << at(i);
	}

  void command(QDataStream &s,int cmd)
  {
    kdDebug() << "Array id="<<id()<<" got command ("<<cmd<<") !!!" <<endl; 
    switch(cmd)
    {
      case CmdAt:
      {
        uint i;
        type data;
        s >> i >> data;
        QArray<type>::at(i)=data;
        kdDebug() << "CmdAt:id="<<id()<<" i="<<i<<" data="<<data <<endl; 
        if (isEmittingSignal()) emitSignal();
        break;
      }
      case CmdResize:
      {
        uint size;
        s >> size;
        kdDebug() << "CmdResize:id="<<id()<<" oldsize="<<QArray<type>::size()<<" newsize="<<size <<endl; 
        if (QArray<type>::size()!=size) resize(size);
        break;
      }
      case CmdFill:
      {
        int size;
        type data;
        s >> data >> size;
        kdDebug() << "CmdFill:id="<<id()<<"size="<<size <<endl; 
        QArray<type>::fill(data,size);
        if (isEmittingSignal()) emitSignal();
        break;
      }
      case CmdSort:
      {
        kdDebug() << "CmdSort:id="<<id()<<endl; 
        QArray<type>::sort();
        break;
      }
      default: 
        kdDebug() << "Error in KPropertyArray::command: Unknown command " << cmd << endl;
    }
  }


};

#endif
