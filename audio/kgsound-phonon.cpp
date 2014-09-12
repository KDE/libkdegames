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

#include "kgsound.h"

#include <phonon/MediaObject>
#include <QDateTime>

class KgSound::Private
{
    public:
        qreal m_volume;
        bool m_valid;
        qint64 m_lastPlayedTime;
        int m_nextSource;
        Phonon::MediaObject* m_sound1;
        Phonon::MediaObject* m_sound2;

        Private() : m_volume(1.0), m_valid(false), m_lastPlayedTime(0), m_nextSource(1), m_sound1(NULL), m_sound2(NULL) {}

        ~Private()
        {
            delete m_sound1;
            delete m_sound2;
            m_sound1 = 0;
            m_sound2 = 0;
        }
};

KgSound::KgSound(const QString& file, QObject* parent)
	: QObject(parent)
	, d(new Private)
{
	d->m_sound1 = Phonon::createPlayer(Phonon::GameCategory);
	d->m_sound1->setCurrentSource(QUrl::fromLocalFile(file));
	d->m_sound2 = Phonon::createPlayer(Phonon::GameCategory);
	d->m_sound2->setCurrentSource(QUrl::fromLocalFile(file));
	d->m_valid = d->m_sound1->isValid() && d->m_sound2->isValid();
}

KgSound::~KgSound()
{
	delete d;
}

bool KgSound::isValid() const
{
	return d->m_valid;
}

KgSound::PlaybackType KgSound::playbackType() const
{
	return KgSound::AmbientPlayback;
}

void KgSound::setPlaybackType(KgSound::PlaybackType type)
{
	Q_UNUSED(type)
}

QPointF KgSound::pos() const
{
	return QPointF(0.0, 0.0);
}

void KgSound::setPos(const QPointF& pos)
{
	Q_UNUSED(pos)
}

qreal KgSound::volume() const
{
	//FIXME
	return 1.0;
}

void KgSound::setVolume(qreal volume)
{
	//FIXME
}

bool KgSound::hasError() const
{
	if (d->m_sound1 && d->m_sound1->state() == Phonon::ErrorState)
	{
		return true;
	}
	if (d->m_sound2 && d->m_sound2->state() == Phonon::ErrorState)
	{
		return true;
	}
	return false;
}

void KgSound::start()
{
	if(!d->m_sound1 || !d->m_sound2)
	{
		return;
	}
	
	QDateTime now = QDateTime::currentDateTime();
	qint64 timeNow = now.toTime_t() * 1000 + now.time().msec();
	
	if(timeNow - d->m_lastPlayedTime > 20)
	{
		if(d->m_nextSource == 1)
		{                    
			if(d->m_sound1->state() == Phonon::StoppedState)
			{
				d->m_nextSource = 2;
				d->m_sound1->play();
			}
			else
			{
				d->m_sound1->stop();
			}
		}
		else
		{
			if(d->m_sound2->state() == Phonon::StoppedState)
			{
				d->m_nextSource = 1;
				d->m_sound2->play();
			}
			else
			{
				d->m_sound2->stop();
			}
		}
		d->m_lastPlayedTime = timeNow;
	}
}

void KgSound::start(const QPointF& pos)
{
	Q_UNUSED(pos)
	//ignore parameter
	start();
}

void KgSound::stop()
{
	if(!d->m_sound1 || !d->m_sound2)
	{
		return;
	}
	
	d->m_sound1->stop();
	d->m_sound2->stop();
}

#include "moc_kgsound.cpp"
