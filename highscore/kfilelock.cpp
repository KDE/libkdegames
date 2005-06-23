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

#include <config.h>

#include "kfilelock.h"

#include <unistd.h>
#include <sys/file.h>
#include <errno.h>

#include <kdebug.h>


KFileLock::KFileLock(int fd)
    : _fd(fd), _locked(false)
{}

int KFileLock::lock()
{
    kdDebug(11002) << "lock fd=" << _fd << endl;
#ifdef F_SETLK
# ifndef SEEK_SET
#  define SEEK_SET 0
# endif
    struct flock lock_data;
    lock_data.l_type = F_WRLCK;
    lock_data.l_whence = SEEK_SET;
    lock_data.l_start = lock_data.l_len = 0;
    if ( fcntl(_fd, F_SETLK, &lock_data)==-1 ) {
        if ( errno==EAGAIN ) return -2;
        return -1;
    }
#else
# ifdef LOCK_EX
    if ( flock (_fd, LOCK_EX|LOCK_NB)==-1 ) {
        if ( errno==EWOULDBLOCK ) return -2;
        return -1;
    }
# else
    if ( lockf(_fd, F_TLOCK, 0)==-1 ) {
        if ( errno==EACCES ) return -2;
        return -1;
    }
# endif
#endif
    _locked = true;
    return 0;
}

KFileLock::~KFileLock()
{
    unlock();
}

void KFileLock::unlock()
{
    if ( !_locked ) return;
    kdDebug(11002) << "unlock" << endl;
# ifdef F_SETLK
    struct flock lock_data;
    lock_data.l_type = F_UNLCK;
    lock_data.l_whence = SEEK_SET;
    lock_data.l_start = lock_data.l_len = 0;
    (void)fcntl(_fd, F_SETLK, &lock_data);
# else
#  ifdef F_ULOCK
    lockf(_fd, F_ULOCK, 0);
#  else
    flock(_fd, LOCK_UN);
#  endif
# endif
    _locked = false;
}
