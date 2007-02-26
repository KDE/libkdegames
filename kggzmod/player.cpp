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

#include "player.h"
#include "player_private.h"

using namespace KGGZMod;

Player::Player()
{
	d = new PlayerPrivate();

	d->m_type = Player::unknown;
	d->m_seat = -1;
	d->m_stats = 0;
}

Player::Type Player::type() const
{
	return d->m_type;
}

QString Player::name() const
{
	return d->m_name;
}

int Player::seat() const
{
	return d->m_seat;
}

Statistics *Player::stats() const
{
	return d->m_stats;
}

QString Player::hostname() const
{
	return d->m_hostname;
}

QString Player::photo() const
{
	return d->m_photo;
}

QString Player::realname() const
{
	return d->m_realname;
}

void Player::init(PlayerPrivate *x)
{
	delete d;
	d = x;
}

