/***************************************************************************
 *   Copyright 2019 Alexander Potashev <aspotashev@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License          *
 *   version 2 as published by the Free Software Foundation                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef LIBKDEGAMES_VIRTUALFILEQT_OPENAL_H
#define LIBKDEGAMES_VIRTUALFILEQT_OPENAL_H

// Qt
#include <QFile>
// sndfile
#include <sndfile.hh>


class VirtualFileQt
{
public:
    explicit VirtualFileQt(const QString &path);
    ~VirtualFileQt() = default;

    bool open();

    int64_t getFileLen() const;
    int64_t seek(int64_t offset, int whence);
    int64_t read(void *ptr, int64_t count);
    int64_t write(const void *ptr, int64_t count);
    int64_t tell();

    static SF_VIRTUAL_IO& getSndfileVirtualIO();
    static VirtualFileQt *get(void *user_data);

private:
    QFile m_file;
};

#endif //LIBKDEGAMES_VIRTUALFILEQT_OPENAL_H
