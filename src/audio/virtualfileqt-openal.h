/*
    SPDX-FileCopyrightText: 2019 Alexander Potashev <aspotashev@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

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

    static SF_VIRTUAL_IO &getSndfileVirtualIO();
    static VirtualFileQt *get(void *user_data);

private:
    QFile m_file;
};

#endif // LIBKDEGAMES_VIRTUALFILEQT_OPENAL_H
