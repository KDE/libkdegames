/*
    This file is part of the kggzmod library.
    Copyright (c) 2005 - 2007 Josef Spillner <josef@ggzgamingzone.org>

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

#ifndef KGGZMOD_PLAYER_PRIVATE_H
#define KGGZMOD_PLAYER_PRIVATE_H

namespace KGGZMod
{

class PlayerPrivate
{
	public:
		PlayerPrivate(){}
		Player::Type m_type;
		QString m_name;
		int m_seat;
		Statistics *m_stats;
		QString m_hostname;
		QString m_photo;
		QString m_realname;
};

}

#endif

