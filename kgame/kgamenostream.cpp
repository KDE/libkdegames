
#include "kgamenostream.h"
#include "kgamemessage.h"

#include <qdatastream.h>
#include <qvariant.h>

#include <kdebug.h>

class KMessagePrivate
{
public:
	KMessagePrivate()
	{
		mCookie = 0;
		mMsgversion = 0;
		mSender = 0;
		mReceiver = 0;
		mMsgid = 0;
	}
	
	int mCookie;
	int mMsgversion;
	int mSender;
	int mReceiver;
	int mMsgid;
};

KMessage::KMessage()
{
 d = new KMessagePrivate;
}

KMessage::KMessage(int cookie, int msgversion, int sender, int receiver, int msgid)
{
 d = new KMessagePrivate;
 writeHeader(cookie, msgversion, sender, receiver, msgid);
}

KMessage::~KMessage()
{
 delete d;

}

void KMessage::readHeader(int& cookie, int& msgversion, int& sender, int& receiver, int& msgid) const
{
 cookie = d->mCookie;
 msgversion = d->mMsgversion;
 sender = d->mSender;
 receiver = d->mReceiver;
 msgid = d->mMsgid;
}

void KMessage::writeHeader(int cookie, int msgversion, int sender, int receiver, int msgid)
{
 d->mCookie = cookie;
 d->mMsgversion = msgversion;
 d->mSender = sender;
 d->mReceiver = receiver;
 d->mMsgid = msgid;
}



KGameNoStream::KGameNoStream()
{
}
KGameNoStream::~KGameNoStream()
{
}

void KGameNoStream::toStream(const KMessage& message, QByteArray& convert)
{
 QDataStream stream(convert, IO_WriteOnly);
 int c, v, s, r, m;
 message.readHeader(c, v, s, r, m);
 KGameMessage::createHeader(stream, c, v, s, r, m);

}

void KGameNoStream::toString(const QByteArray& message, KMessage& convert)
{
kdDebug() << "converting stream to string" << endl;
 QDataStream stream(message, IO_ReadOnly);
 int c, v, s, r, m;
 KGameMessage::extractHeader(stream, c, v, s, r, m);
 convert.writeHeader(c, v, s, r, m);
 kdDebug() << c << " " << v << " " << s << " " << r << " " << m << endl;

 for (int i = 0;  i < 5; i++) {
	QVariant data;
	stream >> data;
	kdDebug() << data.typeName() << endl;
	kdDebug() << data.asString() << endl;
 }

kdDebug() << "converting stream to string end" << endl;
}

