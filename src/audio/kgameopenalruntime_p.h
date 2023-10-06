/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KGAMEOPENALRUNTIME_P_H
#define KGAMEOPENALRUNTIME_P_H

// Qt
#include <QHash>
#include <QPointF>
// OpenAL includes (without "AL/" include directory; see FindOpenAL.cmake)
#include <al.h>
#include <alc.h>

class KGameSound;

/// @internal
class KGamePlaybackEvent
{
public:
    // Creates and starts the playback. Also registers with the OpenALRuntime.
    KGamePlaybackEvent(KGameSound *sound, QPointF pos);
    // Stops playback if it is still running.
    ~KGamePlaybackEvent();

    // Is playback still running?
    bool isRunning() const;
    bool replay(QPointF pos) const;

private:
    ALuint m_source;
    bool m_valid;
};

typedef QList<KGamePlaybackEvent *> KGamePlaybackEventList;

/// @internal
class KGameOpenALRuntime
{
public:
    KGameOpenALRuntime();
    ~KGameOpenALRuntime();

    static KGameOpenALRuntime *instance();

    void configureListener();
    void cleanupUnusedSources();

    // global properties
    QPointF m_listenerPos;
    qreal m_volume;
    bool m_error;
    // active sound and playback instances
    QHash<KGameSound *, KGamePlaybackEventList> m_soundsEvents;

private:
    ALCcontext *m_context;
    ALCdevice *m_device;
};

#endif // KGAMEOPENALRUNTIME_P_H
