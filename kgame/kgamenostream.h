
#include <qstring.h>

class KMessagePrivate;
class KMessage
{
public:
	KMessage(int cookie, int msgversion, int sender, int receiver, int msgid);
	KMessage();
	~KMessage();

	void readHeader(int& cookie, int& msgversion, int& sender, int& receiver, int& msgid) const;
	void writeHeader(int  cookie, int msgversion, int sender, int receiver, int msgid);


private:
	KMessagePrivate* d;
};

class KGameNoStream
{
public:
	KGameNoStream();
	~KGameNoStream();
	
	static void toStream(const KMessage& message, QByteArray& convert);
	static void toString(const QByteArray& message, KMessage& convert);

	
};
