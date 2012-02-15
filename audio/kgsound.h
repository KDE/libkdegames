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

#ifndef KGSOUND_H
#define KGSOUND_H

#include <QtCore/QObject>
#include <QtCore/QPointF>

#include <libkdegames_export.h>

class PlaybackEvent;

/**
 * @class KgSound sound.h <KgSound>
 *
 * This class models a sound file. Because it is implicitly added to this
 * application's KgAudioScene, it can be played at different positions (if
 * positional playback is supported, see KgAudioScene::capabilities()).
 *
 * Compared to many other media playback classes, the notable difference of
 * KgSound is that one sound instance can be played multiple times at the
 * same point in time, by calling start() multiple times (possibly with
 * different playback positions). This behavior can be suppressed by calling
 * stop() before start().
 *
 * @note WAV files and Ogg/Vorbis files are guaranteed to work. Other audio
 *       files may also work, depending on the KgAudio backend and its
 *       configuration.
 */
class KDEGAMES_EXPORT KgSound : public QObject
{
	Q_OBJECT
	Q_PROPERTY(KgSound::PlaybackType playbackType READ playbackType WRITE setPlaybackType)
	Q_PROPERTY(QPointF pos READ pos WRITE setPos)
	Q_PROPERTY(qreal volume READ volume WRITE setVolume)
	public:
		///This enumeration describes how a sound can be played back
		enum PlaybackType
		{
			///Positional playback disabled. The sound will appear at the same
			///volume from every listener position, and will not appear to be
			///coming from any direction. The pos() of this sound is ignored.
			AmbientPlayback = 1,
			///Positional playback enabled. That means that the sound comes from
			///a certain direction with a distance-depending volume. The pos()
			///of this sound is given in absolute coordinates: Both direction
			///and volume can change when the listener is moved.
			AbsolutePlayback,
			///Positional playback enabled. That means that the sound comes from
			///a certain direction with a distance-depending volume. The pos()
			///of this sound is given in relative coordinates: The direction
			///and volume do not depend on the listener's position. (In these
			///relative coordinates, the listener is at the point of origin.)
			RelativePlayback
		};

		///Loads a new sound from the given @a file. Note that this is an
		///expensive operation which you might want to do during application
		///startup. However, you can reuse the same Sound instance for multiple
		///playback events.
		explicit KgSound(const QString& file, QObject* parent = 0);
		///Destroys this KgSound instance.
		virtual ~KgSound();

		///@return whether the sound file could be loaded successfully
		bool isValid() const;
		///@return the playback type for this sound
		KgSound::PlaybackType playbackType() const;
		///Sets the playback type for this sound. This affects how the sound
		///will be perceived by the listener. The default is AmbientPlayback.
		///
		///@note Changes to this property will not be propagated to running
		///      playbacks of this sound.
		void setPlaybackType(KgSound::PlaybackType type);
		///@return the position of this sound
		QPointF pos() const;
		///Sets the position of this sound. It depends on the playbackType() how
		///this is position interpreted. See the KgSound::PlaybackType
		///enumeration documentation for details.
		///
		///@note Changes to this property will not be propagated to running
		///      playbacks of this sound.
		void setPos(const QPointF& pos);
		///@return the volume of this sound
		qreal volume() const;
		///Sets the volume of this sound. The default is 1.0, which means no
		///volume change, compared to the original sound file. 0.0 means that
		///the sound is inaudible.
		///
		///If you think of the KgSound as a loudspeaker, the
		///volume which is controlled by this method is what you regulate at its
		///volume control. If positional playback is enabled (see
		///playbackType()), this will not be the actual volume which the
		///listener will perceive, because the playback volume decreases with
		///increasing playback-listener distances.
		///
		///@note Changes to this property will not be propagated to running
		///      playbacks of this sound.
		void setVolume(qreal volume);
	public Q_SLOTS:
		///Starts a new playback instance of this sound. This will not interrupt
		///running playbacks of the same sound or any other sounds.
		void start();
		///@overload
		///This overload takes an additional position argument which overrides
		///the sound's pos() property.
		void start(const QPointF& pos);
		///Stops any playbacks of this sounds.
		void stop();
	private:
		friend class KgPlaybackEvent;
		class Private;
		Private* const d;
};

#endif // KGSOUND_H
