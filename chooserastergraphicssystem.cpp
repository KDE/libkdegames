/***************************************************************************
 *   Copyright 2011 Stefan Majewsky <majewsky@gmx.net>                     *
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

#include <QtGui/QApplication>

/*
 * This file is only compiled on Q_WS_X11 (see CMakeLists.txt). It chooses the
 * raster graphicssystem in anything linking against libkdegames.
 *
 * The raster graphicssystem has a much better performance in graphics-intense
 * apps like games, compared to the X11 graphicssystem which is still the
 * default in Qt 4.7. (Rumor has it that raster will become the default as of Qt
 * 4.8.)
 *
 * This header ensures that raster is always used instead of X11 to enable a
 * more fluent graphics experience. The relevant API call
 * (QApplication::setGraphicsSystem) needs to be issued *before* the
 * application object (QApplication or KApplication) is constructed, so we do it
 * in a constructor of an anonymous global object.
 *
 * Constructors of global objects are called before main() by the C++ runtime.
 */

namespace { //don't export this struct and instance, it's internal affairs
	struct ConfigureDefaultGraphicsSystem
	{
		ConfigureDefaultGraphicsSystem() {
			QApplication::setGraphicsSystem(QLatin1String("raster"));
		}
	};
	ConfigureDefaultGraphicsSystem staticObject;
}
