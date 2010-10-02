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

#include "event.h"

using namespace KGGZMod;

Event::Event(Type type)
{
	m_type = type;
	m_player = 0;
}

Event::Type Event::type() const
{
	return m_type;
}

Player *Event::player() const
{
	return m_player;
}

LaunchEvent::LaunchEvent(const Event& event) : Event(Event::launch)
{
	data = event.data;
}

ServerEvent::ServerEvent(const Event& event) : Event(Event::server)
{
	data = event.data;
}

int ServerEvent::fd() const
{
	QString f = data[QLatin1String( "fd" )];
	return f.toInt();
}

SelfEvent::SelfEvent(const Event& event) : Event(Event::self)
{
	data = event.data;
	m_player = event.player();
}

Player *SelfEvent::self() const
{
	return m_player;
}

SeatEvent::SeatEvent(const Event& event) : Event(Event::seat)
{
	data = event.data;
	m_player = event.player();
}

Player *SeatEvent::player() const
{
	return m_player;
}

ChatEvent::ChatEvent(const Event& event) : Event(Event::chat)
{
	data = event.data;
	m_player = event.player();
}

Player *ChatEvent::player() const
{
	return m_player;
}

QString ChatEvent::message() const
{
	return data[QLatin1String( "message" )];
}

StatsEvent::StatsEvent(const Event& event) : Event(Event::stats)
{
	data = event.data;
	m_player = event.player();
}

Player *StatsEvent::player() const
{
	return m_player;
}

InfoEvent::InfoEvent(const Event& event) : Event(Event::info)
{
	data = event.data;
	m_player = event.player();
}

Player *InfoEvent::player() const
{
	return m_player;
}

RankingsEvent::RankingsEvent(const Event& event) : Event(Event::rankings)
{
	data = event.data;
}

int RankingsEvent::count() const
{
	QString num = data[QLatin1String( "num" )];
	return num.toInt();
}

QString RankingsEvent::name(int i) const
{
	QString name = data[QLatin1String( "name" ) + QString::number(i)];
	return name;
}

int RankingsEvent::score(int i) const
{
	QString score = data[QLatin1String( "score" ) + QString::number(i)];
	return score.toInt();
}

