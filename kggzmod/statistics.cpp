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

#include "statistics.h"
#include "statistics_private.h"

using namespace KGGZMod;

Statistics::Statistics()
{
	d = new StatisticsPrivate();

	d->wins = -1;
	d->losses = -1;
	d->ties = -1;
	d->forfeits = -1;
	d->rating = -1;
	d->ranking = -1;
	d->highscore = -1;

	d->hasrecord = false;
	d->hasranking = false;
	d->hasrating = false;
	d->hashighscore = false;
}

Statistics::~Statistics()
{
	delete d;
}

int Statistics::wins() const
{
	return d->wins;
}

int Statistics::losses() const
{
	return d->losses;
}

int Statistics::ties() const
{
	return d->ties;
}

int Statistics::forfeits() const
{
	return d->forfeits;
}

int Statistics::rating() const
{
	return d->rating;
}

int Statistics::ranking() const
{
	return d->ranking;
}

int Statistics::highscore() const
{
	return d->highscore;
}

bool Statistics::hasRecord() const
{
	return d->hasrecord;
}

bool Statistics::hasRating() const
{
	return d->hasrating;
}

bool Statistics::hasRanking() const
{
	return d->hasranking;
}

bool Statistics::hasHighscore() const
{
	return d->hashighscore;
}

void Statistics::init(StatisticsPrivate *x)
{
	delete d;
	d = x;
}

