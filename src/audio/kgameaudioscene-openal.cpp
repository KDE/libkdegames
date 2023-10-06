/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgameaudioscene.h"

// own
#include "kgameopenalruntime_p.h"
#include <kdegames_audio_logging.h>

Q_GLOBAL_STATIC(KGameOpenALRuntime, g_runtime)

// BEGIN KGameAudioScene

KGameAudioScene::Capabilities KGameAudioScene::capabilities()
{
    return SupportsLowLatencyPlayback | SupportsPositionalPlayback;
}

QPointF KGameAudioScene::listenerPos()
{
    return g_runtime->m_listenerPos;
}

void KGameAudioScene::setListenerPos(QPointF pos)
{
    if (g_runtime->m_listenerPos != pos) {
        g_runtime->m_listenerPos = pos;
        g_runtime->configureListener();
    }
}

qreal KGameAudioScene::volume()
{
    return g_runtime->m_volume;
}

void KGameAudioScene::setVolume(qreal volume)
{
    if (g_runtime->m_volume != volume) {
        g_runtime->m_volume = volume;
        g_runtime->configureListener();
    }
}

bool KGameAudioScene::hasError()
{
    return g_runtime->m_error;
}

// END KGameAudioScene
// BEGIN KGameOpenALRuntime

KGameOpenALRuntime::KGameOpenALRuntime()
    : m_volume(1)
    , m_error(false)
    , m_context(nullptr)
    , m_device(alcOpenDevice(""))
{
    if (!m_device) {
        qCWarning(KDEGAMES_AUDIO_LOG) << "Failed to create OpenAL device";
        m_error = true;
        return;
    }
    m_context = alcCreateContext(m_device, nullptr);
    int error = alcGetError(m_device);
    if (error != AL_NO_ERROR) {
        qCWarning(KDEGAMES_AUDIO_LOG) << "Failed to create OpenAL context: Error code" << error;
        m_error = true;
        return;
    }
    alcMakeContextCurrent(m_context);
    configureListener();
}

KGameOpenALRuntime::~KGameOpenALRuntime()
{
    if (m_context == alcGetCurrentContext()) {
        alcMakeContextCurrent(nullptr);
    }
    alcDestroyContext(m_context);
    alcCloseDevice(m_device);
}

KGameOpenALRuntime *KGameOpenALRuntime::instance()
{
    return g_runtime;
}

void KGameOpenALRuntime::configureListener()
{
    int error;
    alGetError(); // clear error cache
    alListener3f(AL_POSITION, m_listenerPos.x(), m_listenerPos.y(), 0);
    alListenerf(AL_GAIN, m_volume);
    if ((error = alGetError()) != AL_NO_ERROR) {
        qCWarning(KDEGAMES_AUDIO_LOG) << "Failed to setup OpenAL listener: Error code" << error;
        m_error = true;
    }
}

// END KGameOpenALRuntime
