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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KCONFIGRAWBACKEND_H
#define _KCONFIGRAWBACKEND_H

#include <qfile.h>

#include <kconfigbackend.h>
#include <ksimpleconfig.h>


class KConfigRawBackEnd : public KConfigINIBackEnd
{
public:
    KConfigRawBackEnd(KConfigBase *_config, int fd);
    ~KConfigRawBackEnd();

    bool parseConfigFiles();

    void sync(bool bMerge = true);

private:
    int   _fd;
    FILE *_stream;
    QFile _file;

    class KConfigRawBackEndPrivate;
    KConfigRawBackEndPrivate *d;
};

class KRawConfig : public KSimpleConfig
{
    Q_OBJECT
public:
    KRawConfig(int fd, bool readOnly)
        : KSimpleConfig(new KConfigRawBackEnd(this, fd), readOnly) {}
};


#endif
