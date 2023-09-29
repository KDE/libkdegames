/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgsound.h"

// own
#include "kgopenalruntime_p.h"
#include "virtualfileqt-openal.h"
// Qt
#include <QDebug>
// sndfile
#include <sndfile.hh>

class KgSoundPrivate
{
public:
    KgSound::PlaybackType m_type;
    qreal m_volume;
    QPointF m_pos;

    bool m_valid;
    ALuint m_buffer;

    KgSoundPrivate()
        : m_type(KgSound::AmbientPlayback)
        , m_volume(1.0)
        , m_valid(false)
        , m_buffer(AL_NONE)
    {
    }
};

// BEGIN KgSound

KgSound::KgSound(const QString &file, QObject *parent)
    : QObject(parent)
    , d(new KgSoundPrivate)
{
    VirtualFileQt fileInterface(file);
    if (!fileInterface.open()) {
        qWarning() << "Failed to open sound file" << file;
        return;
    }

    // open sound file
    SndfileHandle handle(VirtualFileQt::getSndfileVirtualIO(), &fileInterface);
    if (handle.error()) {
        qWarning() << "Failed to load sound file" << file << ". Error message from libsndfile follows.";
        qWarning() << handle.strError();
        return;
    }
    const int channelCount = handle.channels();
    const int sampleCount = channelCount * handle.frames();
    const int sampleRate = handle.samplerate();
    // load data from sound file
    QList<ALshort> samples(sampleCount);
    if (handle.read(samples.data(), sampleCount) < sampleCount) {
        qWarning() << "Failed to read sound file" << file;
        qWarning() << "File ended unexpectedly.";
        return;
    }
    // determine file format from number of channels
    ALenum format;
    switch (channelCount) {
    case 1:
        format = AL_FORMAT_MONO16;
        break;
    case 2:
        format = AL_FORMAT_STEREO16;
        break;
    default:
        qWarning() << "Failed to read sound file" << file;
        qWarning() << "More than two channels are not supported.";
        return;
    }
    // make sure OpenAL is initialized; clear OpenAL error storage
    KgOpenALRuntime::instance();
    int error;
    alGetError();
    // create OpenAL buffer
    alGenBuffers(1, &d->m_buffer);
    if ((error = alGetError()) != AL_NO_ERROR) {
        qWarning() << "Failed to create OpenAL buffer: Error code" << error;
        return;
    }
    alBufferData(d->m_buffer, format, samples.data(), sampleCount * sizeof(ALshort), sampleRate);
    if ((error = alGetError()) != AL_NO_ERROR) {
        qWarning() << "Failed to fill OpenAL buffer: Error code" << error;
        alDeleteBuffers(1, &d->m_buffer);
        return;
    }
    // loading finished
    d->m_valid = true;
}

KgSound::~KgSound()
{
    if (d->m_valid) {
        stop();
        KgOpenALRuntime::instance()->m_soundsEvents.remove(this);
        alDeleteBuffers(1, &d->m_buffer);
    }
}

bool KgSound::isValid() const
{
    return d->m_valid;
}

KgSound::PlaybackType KgSound::playbackType() const
{
    return d->m_type;
}

void KgSound::setPlaybackType(KgSound::PlaybackType type)
{
    if (d->m_type == type)
        return;
    d->m_type = type;
    Q_EMIT playbackTypeChanged(type);
}

QPointF KgSound::pos() const
{
    return d->m_pos;
}

void KgSound::setPos(const QPointF &pos)
{
    if (d->m_pos == pos)
        return;
    d->m_pos = pos;
    Q_EMIT posChanged(pos);
}

qreal KgSound::volume() const
{
    return d->m_volume;
}

void KgSound::setVolume(qreal volume)
{
    if (d->m_volume == volume)
        return;
    d->m_volume = volume;
    Q_EMIT volumeChanged(volume);
}

bool KgSound::hasError() const
{
    return !d->m_valid;
}

void KgSound::start()
{
    start(d->m_pos);
}

void KgSound::start(const QPointF &pos)
{
    if (d->m_valid) {
        KgOpenALRuntime *runtime = KgOpenALRuntime::instance();
        if (!runtime->instance()->m_soundsEvents[this].isEmpty()) {
            if (runtime->instance()->m_soundsEvents[this].last()->replay(pos) == false) {
                new KgPlaybackEvent(this, pos);
            }
        } else {
            new KgPlaybackEvent(this, pos);
        }
    }
}

void KgSound::stop()
{
    qDeleteAll(KgOpenALRuntime::instance()->m_soundsEvents.take(this));
}

// END KgSound
// BEGIN KgPlaybackEvent

KgPlaybackEvent::KgPlaybackEvent(KgSound *sound, const QPointF &pos)
    : m_valid(false)
{
    // make sure OpenAL is initialized
    KgOpenALRuntime *runtime = KgOpenALRuntime::instance();
    // clear OpenAL error storage
    int error;
    alGetError();
    // create source for playback
    alGenSources(1, &m_source);
    if ((error = alGetError()) != AL_NO_ERROR) {
        qWarning() << "Failed to create OpenAL source: Error code" << error;
        return;
    }
    // store in OpenALRuntime
    runtime->m_soundsEvents[sound] << this;
    m_valid = true;
    // connect to sound (buffer)
    alSource3f(m_source, AL_POSITION, pos.x(), pos.y(), 0);
    alSourcef(m_source, AL_PITCH, 1.0); // TODO: debug
    alSourcef(m_source, AL_GAIN, sound->volume());
    alSourcei(m_source, AL_BUFFER, sound->d->m_buffer);
    const KgSound::PlaybackType type = sound->playbackType();
    alSourcef(m_source, AL_ROLLOFF_FACTOR, type == KgSound::AmbientPlayback ? 0.0 : 1.0);
    alSourcei(m_source, AL_SOURCE_RELATIVE, type == KgSound::RelativePlayback ? AL_TRUE : AL_FALSE);
    if ((error = alGetError()) != AL_NO_ERROR) {
        qWarning() << "Failed to setup OpenAL source: Error code" << error;
        return;
    }
    // start playback
    alSourcePlay(m_source);
}

KgPlaybackEvent::~KgPlaybackEvent()
{
    if (alIsSource(m_source) == AL_TRUE) {
        alSourceStop(m_source);
        alDeleteSources(1, &m_source);
    }
}

bool KgPlaybackEvent::isRunning() const
{
    ALint state;
    alGetSourcei(m_source, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

bool KgPlaybackEvent::replay(const QPointF &pos) const
{
    if (alIsSource(m_source) == AL_TRUE) {
        alSourceStop(m_source);
        alSource3f(m_source, AL_POSITION, pos.x(), pos.y(), 0);
        alSourcePlay(m_source);
        return true;
    } else {
        return false;
    }
}

// END KgPlaybackEvent

#include "moc_kgsound.cpp"
