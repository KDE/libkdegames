/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KGOPENALRUNTIME_P_H
#define KGOPENALRUNTIME_P_H

// Qt
#include <QHash>
#include <QPointF>
// OpenAL includes (without "AL/" include directory; see FindOpenAL.cmake)
#include <al.h>
#include <alc.h>

class KGameSound;

/// @internal
class KgPlaybackEvent
{
public:
    // Creates and starts the playback. Also registers with the OpenALRuntime.
    KgPlaybackEvent(KGameSound *sound, const QPointF &pos);
    // Stops playback if it is still running.
    ~KgPlaybackEvent();

    // Is playback still running?
    bool isRunning() const;
    bool replay(const QPointF &pos) const;

private:
    ALuint m_source;
    bool m_valid;
};

typedef QList<KgPlaybackEvent *> KgPlaybackEventList;

/// @internal
class KgOpenALRuntime
{
public:
    KgOpenALRuntime();
    ~KgOpenALRuntime();

    static KgOpenALRuntime *instance();

    void configureListener();
    void cleanupUnusedSources();

    // global properties
    QPointF m_listenerPos;
    qreal m_volume;
    bool m_error;
    // active sound and playback instances
    QHash<KGameSound *, KgPlaybackEventList> m_soundsEvents;

private:
    ALCcontext *m_context;
    ALCdevice *m_device;
};

#endif // KGOPENALRUNTIME_P_H
