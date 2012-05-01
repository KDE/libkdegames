/*  This file is part of the KDE project
    Copyright (C) 2007 David Faure <faure@kde.org>
    Copyright (C) 2012 Stefan Majewsky <majewsky@gmx.net>

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
    Boston, MA 02110-1301, USA.
*/

#ifndef LIBKDEGAMESPRIVATE_EXPORT_H
#define LIBKDEGAMESPRIVATE_EXPORT_H

#ifdef MAKE_KDEGAMESPRIVATE_LIB
# define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#endif
/* error out if a special macro is not defined; we want to be very clear that
 * these are private headers */
#ifndef USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
# error "You must not use libkdegamesprivate outside the kdegames source tree."
# error "There is no stability promise for this API. If you need something from"
# error "libkdegamesprivate for compatibility reasons, you're probably better"
# error "off by copying the relevant code from the libkdegamesprivate sources"
# error "into your own source tree."
#endif

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef KDEGAMESPRIVATE_EXPORT
# if defined(MAKE_KDEGAMESPRIVATE_LIB)
   /* We are building this library */ 
#  define KDEGAMESPRIVATE_EXPORT KDE_EXPORT
# else
   /* We are using this library */ 
#  define KDEGAMESPRIVATE_EXPORT KDE_IMPORT
# endif
#endif

// # ifndef KDEGAMESPRIVATE_EXPORT_DEPRECATED
// #  define KDEGAMESPRIVATE_EXPORT_DEPRECATED KDE_DEPRECATED KDEGAMESPRIVATE_EXPORT
// # endif

#endif
