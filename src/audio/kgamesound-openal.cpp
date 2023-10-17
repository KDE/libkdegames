/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgamesound.h"

// own
#include "kgameopenalruntime_p.h"
#include "virtualfileqt-openal.h"
#include <kdegames_audio_logging.h>
// sndfile
#include <sndfile.hh>

class KGameSoundPrivate
{
public:
    KGameSound::PlaybackType m_type;
    qreal m_volume;
    QPointF m_pos;

    bool m_valid;
    ALuint m_buffer;

    KGameSoundPrivate()
        : m_type(KGameSound::AmbientPlayback)
        , m_volume(1.0)
        , m_valid(false)
        , m_buffer(AL_NONE)
    {
    }
};

// BEGIN KGameSound

KGameSound::KGameSound(const QString &file, QObject *parent)
    : QObject(parent)
    , d_ptr(new KGameSoundPrivate)
{
    Q_D(KGameSound);

    VirtualFileQt fileInterface(file);
    if (!fileInterface.open()) {
        qCWarning(KDEGAMES_AUDIO_LOG) << "Failed to open sound file" << file;
        return;
    }

    // open sound file
    SndfileHandle handle(VirtualFileQt::getSndfileVirtualIO(), &fileInterface);
    if (handle.error()) {
        qCWarning(KDEGAMES_AUDIO_LOG) << "Failed to load sound file" << file << ". Error message from libsndfile follows.";
        qCWarning(KDEGAMES_AUDIO_LOG) << handle.strError();
        return;
    }
    const int channelCount = handle.channels();
    const int sampleCount = channelCount * handle.frames();
    const int sampleRate = handle.samplerate();
    // load data from sound file
    QList<ALshort> samples(sampleCount);
    if (handle.read(samples.data(), sampleCount) < sampleCount) {
        qCWarning(KDEGAMES_AUDIO_LOG) << "Failed to read sound file" << file;
        qCWarning(KDEGAMES_AUDIO_LOG) << "File ended unexpectedly.";
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
        qCWarning(KDEGAMES_AUDIO_LOG) << "Failed to read sound file" << file;
        qCWarning(KDEGAMES_AUDIO_LOG) << "More than two channels are not supported.";
        return;
    }
    // make sure OpenAL is initialized; clear OpenAL error storage
    KGameOpenALRuntime::instance();
    int error;
    alGetError();
    // create OpenAL buffer
    alGenBuffers(1, &d->m_buffer);
    if ((error = alGetError()) != AL_NO_ERROR) {
        qCWarning(KDEGAMES_AUDIO_LOG) << "Failed to create OpenAL buffer: Error code" << error;
        return;
    }
    alBufferData(d->m_buffer, format, samples.data(), sampleCount * sizeof(ALshort), sampleRate);
    if ((error = alGetError()) != AL_NO_ERROR) {
        qCWarning(KDEGAMES_AUDIO_LOG) << "Failed to fill OpenAL buffer: Error code" << error;
        alDeleteBuffers(1, &d->m_buffer);
        return;
    }
    // loading finished
    d->m_valid = true;
}

KGameSound::~KGameSound()
{
    Q_D(KGameSound);

    if (d->m_valid) {
        stop();
        KGameOpenALRuntime::instance()->m_soundsEvents.remove(this);
        alDeleteBuffers(1, &d->m_buffer);
    }
}

bool KGameSound::isValid() const
{
    Q_D(const KGameSound);

    return d->m_valid;
}

KGameSound::PlaybackType KGameSound::playbackType() const
{
    Q_D(const KGameSound);

    return d->m_type;
}

void KGameSound::setPlaybackType(KGameSound::PlaybackType type)
{
    Q_D(KGameSound);

    if (d->m_type == type)
        return;
    d->m_type = type;
    Q_EMIT playbackTypeChanged(type);
}

QPointF KGameSound::pos() const
{
    Q_D(const KGameSound);

    return d->m_pos;
}

void KGameSound::setPos(QPointF pos)
{
    Q_D(KGameSound);

    if (d->m_pos == pos)
        return;
    d->m_pos = pos;
    Q_EMIT posChanged(pos);
}

qreal KGameSound::volume() const
{
    Q_D(const KGameSound);

    return d->m_volume;
}

void KGameSound::setVolume(qreal volume)
{
    Q_D(KGameSound);

    if (d->m_volume == volume)
        return;
    d->m_volume = volume;
    Q_EMIT volumeChanged(volume);
}

bool KGameSound::hasError() const
{
    Q_D(const KGameSound);

    return !d->m_valid;
}

void KGameSound::start()
{
    Q_D(KGameSound);

    start(d->m_pos);
}

void KGameSound::start(QPointF pos)
{
    Q_D(KGameSound);

    if (d->m_valid) {
        KGameOpenALRuntime *runtime = KGameOpenALRuntime::instance();
        if (!runtime->instance()->m_soundsEvents[this].isEmpty()) {
            if (runtime->instance()->m_soundsEvents[this].last()->replay(pos) == false) {
                new KGamePlaybackEvent(this, pos);
            }
        } else {
            new KGamePlaybackEvent(this, pos);
        }
    }
}

void KGameSound::stop()
{
    qDeleteAll(KGameOpenALRuntime::instance()->m_soundsEvents.take(this));
}

// END KGameSound
// BEGIN KGamePlaybackEvent

KGamePlaybackEvent::KGamePlaybackEvent(KGameSound *sound, QPointF pos)
    : m_valid(false)
{
    // make sure OpenAL is initialized
    KGameOpenALRuntime *runtime = KGameOpenALRuntime::instance();
    // clear OpenAL error storage
    int error;
    alGetError();
    // create source for playback
    alGenSources(1, &m_source);
    if ((error = alGetError()) != AL_NO_ERROR) {
        qCWarning(KDEGAMES_AUDIO_LOG) << "Failed to create OpenAL source: Error code" << error;
        return;
    }
    // store in OpenALRuntime
    runtime->m_soundsEvents[sound] << this;
    m_valid = true;
    // connect to sound (buffer)
    alSource3f(m_source, AL_POSITION, pos.x(), pos.y(), 0);
    alSourcef(m_source, AL_PITCH, 1.0); // TODO: debug
    alSourcef(m_source, AL_GAIN, sound->volume());
    alSourcei(m_source, AL_BUFFER, sound->d_ptr->m_buffer);
    const KGameSound::PlaybackType type = sound->playbackType();
    alSourcef(m_source, AL_ROLLOFF_FACTOR, type == KGameSound::AmbientPlayback ? 0.0 : 1.0);
    alSourcei(m_source, AL_SOURCE_RELATIVE, type == KGameSound::RelativePlayback ? AL_TRUE : AL_FALSE);
    if ((error = alGetError()) != AL_NO_ERROR) {
        qCWarning(KDEGAMES_AUDIO_LOG) << "Failed to setup OpenAL source: Error code" << error;
        return;
    }
    // start playback
    alSourcePlay(m_source);
}

KGamePlaybackEvent::~KGamePlaybackEvent()
{
    if (alIsSource(m_source) == AL_TRUE) {
        alSourceStop(m_source);
        alDeleteSources(1, &m_source);
    }
}

bool KGamePlaybackEvent::isRunning() const
{
    ALint state;
    alGetSourcei(m_source, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

bool KGamePlaybackEvent::replay(QPointF pos) const
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

// END KGamePlaybackEvent

#include "moc_kgamesound.cpp"
