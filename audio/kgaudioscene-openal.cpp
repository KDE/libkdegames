/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>                     *
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
#include "kgopenalruntime_p.h"

#include <KDE/KDebug>
#include <KDE/KGlobal>

K_GLOBAL_STATIC(KgOpenALRuntime, g_runtime)

//BEGIN KgAudioScene

QPointF KgAudioScene::listenerPos()
{
	return g_runtime->m_listenerPos;
}

void KgAudioScene::setListenerPos(const QPointF& pos)
{
	if (g_runtime->m_listenerPos != pos)
	{
		g_runtime->m_listenerPos = pos;
		g_runtime->configureListener();
	}
}

qreal KgAudioScene::volume()
{
	return g_runtime->m_volume;
}

void KgAudioScene::setVolume(qreal volume)
{
	if (g_runtime->m_volume != volume)
	{
		g_runtime->m_volume = volume;
		g_runtime->configureListener();
	}
}

//END KgAudioScene
//BEGIN KgOpenALRuntime

KgOpenALRuntime::KgOpenALRuntime()
	: m_volume(1)
	, m_context(0)
	, m_device(alcOpenDevice(""))
{
	if (!m_device)
	{
		kDebug() << "Failed to create OpenAL device";
		return;
	}
	m_context = alcCreateContext(m_device, 0);
	int error = alcGetError(m_device);
	if (error != AL_NO_ERROR)
	{
		kDebug() << "Failed to create OpenAL context: Error code" << error;
		return;
	}
	alcMakeContextCurrent(m_context);
	configureListener();
}

KgOpenALRuntime::~KgOpenALRuntime()
{
	if (m_context == alcGetCurrentContext())
	{
		alcMakeContextCurrent(0);
	}
	alcDestroyContext(m_context);
	alcCloseDevice(m_device);
}

KgOpenALRuntime* KgOpenALRuntime::instance()
{
	return g_runtime;
}

void KgOpenALRuntime::configureListener()
{
	int error; alGetError(); //clear error cache
	alListener3f(AL_POSITION, m_listenerPos.x(), m_listenerPos.y(), 0);
	alListenerf(AL_GAIN, m_volume);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		kDebug() << "Failed to setup OpenAL listener: Error code" << error;
	}
}

//END KgOpenALRuntime
