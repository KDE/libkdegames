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

#include "module.h"
#include "module_private.h"

#include <kggzmod/statistics.h>
#include "player_private.h"
#include "statistics_private.h"
#include "misc_private.h"

#include <kggznet/kggzraw.h>

#include <kdebug.h>

#include <stdlib.h> // for getenv()

using namespace KGGZMod;

// Implementation notes:
// 1) all classes have private counterparts to keep the API clean
//  this is a bit clumsy in the source but also offers ABI compatibility
// 2) classes always take ownership of their private objects
//  i.e. after calling obj->init(opriv) the caller mustn't delete opriv
//  and certainly not do anything else including calling init() again

static Module *s_module = NULL;

Module::Module(const QString &name)
: QObject()
{
	s_module = this;

	d = new ModulePrivate();

	d->m_name = name;

	d->m_fd = -1;

	d->m_net = 0;
	d->m_notifier = 0;
	d->m_gnotifier = 0;

	d->m_state = created;

	d->m_playerseats = 0;
	d->m_spectatorseats = 0;

	d->m_myseat = -1;

	connect(d, SIGNAL(signalEvent(const KGGZMod::Event&)), this, SIGNAL(signalEvent(const KGGZMod::Event&)));
	connect(d, SIGNAL(signalError()), this, SIGNAL(signalError()));
	connect(d, SIGNAL(signalNetwork(int)), this, SIGNAL(signalNetwork(int)));

	d->connect();
}

Module::~Module()
{
	d->disconnect();
	delete d;

	s_module = NULL;
}

void Module::sendRequest(Request request)
{
	d->sendRequest(request);
}

QList<Player*> Module::players() const
{
	return d->m_players;
}

QList<Player*> Module::spectators() const
{
	return d->m_spectators;
}

Module::State Module::state() const
{
	return d->m_state;
}

void ModulePrivate::sendRequest(Request request)
{
	if(!m_net)
	{
		kDebug(11003) << "[kggzmod] error: not connected";
		return;
	}

	kDebug(11003) << "[kggzmod] debug: send a request";
	kDebug(11003) << "[kggzmod] info: send request" << requestString(request.type());

	Request::Type opcode = request.type();
	// FIXME: in networking we always assume sizeof(opcode) = 4!

	if(opcode == Request::state)
	{
		*m_net << opcode;
		*m_net << (qint8)request.data["state"].toInt();
	}
	if(opcode == Request::stand)
	{
		*m_net << opcode;
	}
	if(opcode == Request::sit)
	{
		*m_net << opcode;
		*m_net << request.data["seat"].toInt();
	}
	if(opcode == Request::boot)
	{
		*m_net << opcode;
		*m_net << request.data["player"];
	}
	if(opcode == Request::bot)
	{
		*m_net << opcode;
		*m_net << request.data["seat"].toInt();
	}
	if(opcode == Request::open)
	{
		*m_net << opcode;
		*m_net << request.data["seat"].toInt();
	}
	if(opcode == Request::chat)
	{
		*m_net << opcode;
		*m_net << request.data["message"];
	}
	if(opcode == Request::info)
	{
		*m_net << opcode;
		*m_net << request.data["seat"].toInt();
	}
	if(opcode == Request::rankings)
	{
		*m_net << opcode;
	}
}

QString ModulePrivate::requestString(Request::Type requestcode)
{
	QString str;
	QMap<Request::Type, QString> requestcodes;

	requestcodes[Request::state] = "Request::state";
	requestcodes[Request::stand] = "Request::stand";
	requestcodes[Request::sit] = "Request::sit";
	requestcodes[Request::boot] = "Request::boot";
	requestcodes[Request::bot] = "Request::bot";
	requestcodes[Request::open] = "Request::open";
	requestcodes[Request::chat] = "Request::chat";
	requestcodes[Request::info] = "Request::info";
	requestcodes[Request::rankings] = "Request::rankings";

	if(requestcodes.contains(requestcode))
	{
		str = requestcodes[requestcode];
	}
	else
	{
		str = "??unknown??";
	}

	str += " (" + QString::number(requestcode) + ')';

	return str;
}

QString ModulePrivate::opcodeString(GGZEvents opcode)
{
	QString str;
	QMap<GGZEvents, QString> opcodes;

	opcodes[msglaunch] = "msglaunch";
	opcodes[msgserver] = "msgserver";
	opcodes[msgserverfd] = "msgserverfd";
	opcodes[msgplayer] = "msgplayer";
	opcodes[msgseat] = "msgseat";
	opcodes[msgspectatorseat] = "msgspectatorseat";
	opcodes[msgchat] = "msgchat";
	opcodes[msgstats] = "msgstats";
	opcodes[msginfo] = "msginfo";
	opcodes[msgrankings] = "msgrankings";

	if(opcodes.contains(opcode))
	{
		str = opcodes[opcode];
	}
	else
	{
		str = "??unknown??";
	}

	str += " (" + QString::number(opcode) + ')';

	return str;
}

void ModulePrivate::slotGGZEvent()
{
	int opcodetmp, ret;
	GGZEvents opcode;
	QString _host;
	int _port;
	QString _player, _message;
	int _fd;
	int _isspectator, _seat;
	int _seattype;
	int _hasrecord, _hasrating, _hasranking, _hashighscore;
	int _wins, _losses, _ties, _forfeits, _rating, _ranking, _highscore;
	int _num;
	QString _realname, _photo;
	QList<Player*>::Iterator it;

	kDebug(11003) << "[kggzmod] debug: input from GGZ has arrived";
	*m_net >> opcodetmp;
	opcode = (GGZEvents)opcodetmp;

	kDebug(11003) << "[kggzmod] info: got GGZ input" << opcodeString(opcode);

	if((opcode < msglaunch) || (opcode > msgrankings))
	{
		kDebug(11003) << "[kggzmod] error: unknown opcode";
		disconnect();
		emit signalError();
		return;
	}

	// FIXME: also treat ggzmod 'error' events as signalError()!

	if(opcode == msglaunch)
	{
		Event e(Event::launch);
		emit signalEvent(e);

		// FIXME: ggzmod doesn't provide a "launch" event

		sendRequest(StateRequest(Module::connected));
	}
	if(opcode == msgserver)
	{
		//Event e(Event::server);
		*m_net >> _host;
		*m_net >> _port;
		*m_net >> _player;
		//e.data["host"] = _host;
		//e.data["player"] = _player;
		//e.data["port"] = QString::number(_port);
		//emit signalEvent(e);

		// FIXME: we don't handle this variant

		kDebug(11003) << "[kggzmod] error: we don't handle msgserver";
		disconnect();
		emit signalError();
	}
	if(opcode == msgserverfd)
	{
		sendRequest(StateRequest(Module::waiting));

		Event e(Event::server);
		// FIXME: this is a send_fd operation, might not be portable!
		kDebug(11003) << "[kggzmod] debug: go read fd with ancillary data";
		ret = readfiledescriptor(m_fd, &_fd);
		if(!ret)
		{
			kDebug(11003) << "[kggzmod] error: socket reading failed";
			disconnect();
			emit signalError();
			return;
		}

		kDebug(11003) << "[kggzmod] debug: server fd =" << _fd;
		e.data["fd"] = QString::number(_fd);
		emit signalEvent(e);

		m_gnotifier = new QSocketNotifier(_fd, QSocketNotifier::Read, this);
		QObject::connect(m_gnotifier, SIGNAL(activated(int)), SIGNAL(signalNetwork(int)));
	}
	if(opcode == msgplayer)
	{
		Event e(Event::self);
		*m_net >> _player;
		*m_net >> _isspectator;
		*m_net >> _seat;
		e.data["player"] = _player;

		m_myseat = _seat;
		m_myspectator = (_isspectator != 0);

		insertPlayer((_isspectator ? Player::spectator : Player::player),
			e.data["player"], _seat);
		e.m_player = findPlayer((_isspectator ? Player::spectator : Player::player),
			e.data["player"]);

		emit signalEvent(e);
	}
	if(opcode == msgseat)
	{
		Event e(Event::seat);
		*m_net >> _seat;
		*m_net >> _seattype;
		*m_net >> _player;
		e.data["player"] = _player;

		insertPlayer((Player::Type)_seattype, e.data["player"], _seat);
		e.m_player = findPlayer((Player::Type)_seattype, e.data["player"]);

		if(_seat >= m_playerseats) m_playerseats = _seat + 1;

		emit signalEvent(e);
	}
	if(opcode == msgspectatorseat)
	{
		Event e(Event::seat);
		*m_net >> _seat;
		*m_net >> _player;
		e.data["player"] = _player;

		insertPlayer(Player::spectator, e.data["player"], _seat);
		e.m_player = findPlayer(Player::spectator, e.data["player"]);

		if(_seat >= m_spectatorseats) m_spectatorseats = _seat + 1;

		emit signalEvent(e);
	}
	if(opcode == msgchat)
	{
		Event e(Event::chat);
		*m_net >> _player;
		*m_net >> _message;
		e.data["player"] = _player;
		e.data["message"] = _message;

		e.m_player = findPlayer(Player::player, e.data["player"]);

		emit signalEvent(e);
	}
	if(opcode == msgstats)
	{
		Event e(Event::stats);

		for(int i = 0; i < m_playerseats + m_spectatorseats; i++)
		{
			*m_net >> _hasrecord;
			*m_net >> _hasrating;
			*m_net >> _hasranking;
			*m_net >> _hashighscore;
			*m_net >> _wins;
			*m_net >> _losses;
			*m_net >> _ties;
			*m_net >> _forfeits;
			*m_net >> _rating;
			*m_net >> _ranking;
			*m_net >> _highscore;

			Statistics *stat = new Statistics();
			StatisticsPrivate *statpriv = new StatisticsPrivate();
			statpriv->hasrecord = false;
			statpriv->hasrating = false;
			statpriv->hasranking = false;
			statpriv->hashighscore = false;

			if(_hasrecord)
			{
				statpriv->wins = _wins;
				statpriv->losses = _wins;
				statpriv->ties = _ties;
				statpriv->forfeits = _forfeits;
				statpriv->hasrecord = true;
			}
			if(_hasrating)
			{
				statpriv->rating = _rating;
				statpriv->hasrating = true;
			}
			if(_hasranking)
			{
				statpriv->ranking = _ranking;
				statpriv->hasranking = true;
			}
			if(_hashighscore)
			{
				statpriv->highscore = _highscore;
				statpriv->hashighscore = true;
			}
			stat->init(statpriv);

			Player *p;
			if(i < m_playerseats) p = m_players.at(i);
			else p = m_spectators.at(i);
			p->d->m_stats = stat;
		}

		emit signalEvent(e);
	}
	if(opcode == msginfo)
	{
		Event e(Event::info);

		*m_net >> _num;

		for(int i = 0; i < _num; i++)
		{
			*m_net >> _seat;
			*m_net >> _realname;
			*m_net >> _photo;
			*m_net >> _host;
			//e.data["realname"] = QString(_realname);
			//e.data["photo"] = QString(_photo);
			//e.data["host"] = QString(_host);

			for(it = m_players.begin(); it != m_players.end(); it++)
			{
				if((*it)->seat() == _seat)
				{
					Player *p = (*it);
					p->d->m_realname = _realname;
					p->d->m_hostname = _host;
					p->d->m_photo = _photo;
					break;
				}
			}
		}

		emit signalEvent(e);
	}
	if(opcode == msgrankings)
	{
		Event e(Event::rankings);
		kDebug(11003) << "[kggzmod] debug: rankings message" << endl;

		*m_net >> _num;
		e.data["num"] = QString::number(_num);

		for(int i = 0; i < _num; i++)
		{
			kDebug(11003) << " ~~rankings: iterate~~ " << i << endl;
			*m_net >> _realname;
			*m_net >> _ranking;
			*m_net >> _highscore;
			e.data["name" + QString::number(i)] = _realname;
			e.data["position" + QString::number(i)] = QString::number(_ranking);
			e.data["score" + QString::number(i)] = QString::number(_highscore);
		}

		emit signalEvent(e);
	}
}

void ModulePrivate::connect()
{
	kDebug(11003) << "[kggzmod] debug: connect() to GGZ";

	if(!Module::isGGZ())
	{
		kDebug(11003) << "[kggzmod] info: GGZMODE not set, ignore";
		// FIXME: alternatively throw error as well?
		return;
	}

	QString ggzsocket = getenv("GGZSOCKET");
	if(ggzsocket.isNull())
	{
		kDebug(11003) << "[kggzmod] error: GGZSOCKET not set";
		emit signalError();
		return;
	}

	m_fd = ggzsocket.toInt();
	kDebug(11003) << "[kggzmod] debug: use socket" << ggzsocket;
	kDebug(11003) << "[kggzmod] debug: numeric socket" << m_fd;

	m_net = new KGGZRaw();
	m_net->setNetwork(m_fd);

	m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
	QObject::connect(m_notifier, SIGNAL(activated(int)), SLOT(slotGGZEvent()));

	kDebug(11003) << "[kggzmod] debug: connect() is finished";
}

void ModulePrivate::disconnect()
{
	delete m_gnotifier;
	delete m_notifier;
	delete m_net;

	m_net = 0;
	m_notifier = 0;
	m_gnotifier = 0;
}

Player *ModulePrivate::self() const
{
	if(m_myseat == -1)
	{
		return 0;
	}

	if(m_myspectator)
	{
		return m_spectators.at(m_myseat);
	}
	else
	{
		return m_players.at(m_myseat);
	}

	return 0;
}

Player* ModulePrivate::findPlayer(Player::Type seattype, const QString &name)
{
	QList<Player*>::Iterator it;

	if(seattype == Player::spectator)
	{
		for(it = m_spectators.begin(); it != m_spectators.end(); it++)
		{
			if((*it)->name() == name)
			{
				return (*it);
			}
		}
	}
	else
	{
		for(it = m_players.begin(); it != m_players.end(); it++)
		{
			if((*it)->name() == name)
			{
				return (*it);
			}
		}
	}

	return 0;
}

void ModulePrivate::insertPlayer(Player::Type seattype, const QString &name, int seat)
{
	QList<Player*>::Iterator it;

	if(seat == -1)
	{
		return;
	}

	Player *p = new Player();
	PlayerPrivate *ppriv = new PlayerPrivate();
	ppriv->m_type = seattype;
	ppriv->m_name = name;
	ppriv->m_seat = seat;
	ppriv->m_stats = 0;
	p->init(ppriv);

	if(seattype == Player::spectator)
	{
		if(seat < m_spectators.count())
		{
			m_spectators.replace(seat, p);
		}
		else
		{
			m_spectators.append(p);
		}
	}
	else
	{
		if(seat < m_players.count())
		{
			m_players.replace(seat, p);
		}
		else
		{
			m_players.append(p);
		}
	}
}

bool Module::isGGZ()
{
	if(getenv("GGZMODE")) return true;
	else return false;
}

Player *Module::self() const
{
	return d->self();
}

Module *Module::instance()
{
	return s_module;
}

#include "module.moc"
#include "module_private.moc"
