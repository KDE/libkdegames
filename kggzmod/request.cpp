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

#include "request.h"

using namespace KGGZMod;

Request::Request(Type type)
{
	m_type = type;
}

Request::Type Request::type() const
{
	return m_type;
}

StateRequest::StateRequest(/*Module::State*/int state)
: Request(Request::state)
{
	data["state"] = QString::number(state);
}

StandRequest::StandRequest()
: Request(Request::stand)
{
}

SitRequest::SitRequest(int seat)
: Request(Request::sit)
{
	data["seat"] = QString::number(seat);
}

BootRequest::BootRequest(const QString &playername)
: Request(Request::boot)
{
	data["player"] = playername;
}

BotRequest::BotRequest(int seat)
: Request(Request::bot)
{
	data["seat"] = QString::number(seat);
}

OpenRequest::OpenRequest(int seat)
: Request(Request::open)
{
	data["seat"] = QString::number(seat);
}

ChatRequest::ChatRequest(const QString &message)
: Request(Request::chat)
{
	data["message"] = message;
}

InfoRequest::InfoRequest(int seat)
: Request(Request::info)
{
	data["seat"] = QString::number(seat);
}

InfoRequest::InfoRequest()
: Request(Request::info)
{
	data["seat"] = QString::number(-1);
}

