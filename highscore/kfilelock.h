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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KFILELOCK_H
#define KFILELOCK_H


class KFileLock
{
public:
    KFileLock(int fd);

    /** Call unlock(). */
    ~KFileLock();

    /** @return the file descriptor. */
    int fd() const { return _fd; }

    /*
     * Lock the file.
     * @return 0 on success, -1 on failure (no permission) and -2 if another
     * process is currently locking the file.
     */
    int lock();

    /** Unlock the file. */
    void unlock();

    /** @return true if we currently lock the file. */
    bool isLocked() const { return _locked; }

private:
    int   _fd;
    bool  _locked;
};


#endif
