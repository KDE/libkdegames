/*
    This file is part of kdegames
    Copyright (c) 2006 kdegames Team

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/



#ifndef _LIBKDEGAMES_EXPORT_H
#define _LIBKDEGAMES_EXPORT_H

#include <kdemacros.h>

#ifdef Q_WS_WIN

#ifndef HIGHSCORE_EXPORT
# ifdef MAKE_HIGHSCORE_LIB
#  define HIGHSCORE_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define HIGHSCORE_EXPORT KDE_IMPORT
# else
#  define HIGHSCORE_EXPORT
# endif
#endif

#ifndef DIALOGS_EXPORT
# ifdef MAKE_DIALOGS_LIB
#  define DIALOGS_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define DIALOGS_EXPORT KDE_IMPORT
# else
#  define DIALOGS_EXPORT
# endif
#endif

#ifndef KGAME_EXPORT
# ifdef MAKE_KGAME_LIB
#  define KGAME_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KGAME_EXPORT KDE_IMPORT
# else
#  define KGAME_EXPORT
# endif
#endif



#else // not windows

#define HIGHSCORE_EXPORT KDE_EXPORT
#define DIALOGS_EXPORT KDE_EXPORT
#define KGAME_EXPORT KDE_EXPORT
#endif /* not windows */

#endif /* _LIBKDEGAMES_EXPORT_H */
