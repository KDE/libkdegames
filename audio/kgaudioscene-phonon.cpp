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

#include "kgaudioscene.h"

#include <Phonon/AudioOutput>

QPointF KgAudioScene::listenerPos()
{
	return QPointF(0.0, 0.0);
}

void KgAudioScene::setListenerPos(const QPointF& pos)
{
	Q_UNUSED(pos)
	static bool onlyOnce = true;
	if (onlyOnce)
	{
		onlyOnce = false;
		kDebug() << "Not supported by Phonon.";
	}
}

qreal KgAudioScene::volume()
{
	return Phonon::AudioOutput(Phonon::GameCategory).volume();
}

void KgAudioScene::setVolume(qreal volume)
{
	Phonon::AudioOutput(Phonon::GameCategory).setVolume(volume);
}
