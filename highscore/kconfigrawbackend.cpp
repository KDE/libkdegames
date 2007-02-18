/*
   This file is part of the KDE games library
   Copyright (C) 2003 Nicolas Hadacek <hadacek@kde.org>

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

#include "kconfigrawbackend.h"

#include <unistd.h>

KConfigRawBackEnd::KConfigRawBackEnd(KConfigBase *_config, int fd)
    : KConfigINIBackEnd(_config, QString(), "config", false),
      _fd(fd), _stream(0)
{
    _file.open(_fd, QIODevice::ReadOnly);
}

KConfigRawBackEnd::~KConfigRawBackEnd()
{
    if (_stream) fclose(_stream);
}

bool KConfigRawBackEnd::parseConfigFiles()
{
    _file.reset();
    parseSingleConfigFile(_file);
    return true;
}

void KConfigRawBackEnd::sync(bool bMerge)
{
  // write-sync is only necessary if there are dirty entries
  if ( !pConfig->isDirty() || pConfig->isReadOnly() ) return;

  _file.reset();
  KEntryMap aTempMap;
  getEntryMap(aTempMap, false, bMerge ? &_file : 0);

  if ( _stream==0 ) {
      _stream = fdopen(_fd, "w");
      if ( _stream==0 ) return;
  }
  _file.resize(0);
  writeEntries(_stream, aTempMap);
  fflush(_stream);
}
