/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001 Martin Heni (kde at heni-online.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kgameconnectdialog.h"

#include <knuminput.h>
#include <klocale.h>

#include <QLineEdit>
#include <QComboBox>
#include <QLayout>
#include <qradiobutton.h>
#include <QLabel>
#include <QVBoxLayout>
#include <QList>
#include <QButtonGroup>
#include <dnssd/servicebrowser.h>
#include <QPushButton>

class KGameConnectWidgetPrivate
{
 public:
	KGameConnectWidgetPrivate()
	{
		mPort = 0;
		mHost = 0;
		mButtonGroup = 0;
		mBrowser = 0;
	}

	KIntNumInput* mPort;
	QLineEdit* mHost; //KLineEdit?
	QButtonGroup* mButtonGroup;
	QComboBox *mClientName;
	QLabel *mClientNameLabel;
	DNSSD::ServiceBrowser *mBrowser;
	QLabel *mServerNameLabel;
	QLineEdit *mServerName;
	QString mType;
};

KGameConnectWidget::KGameConnectWidget(QWidget* parent) : QWidget(parent)
{
 d = new KGameConnectWidgetPrivate;

 QVBoxLayout* vb = new QVBoxLayout(this);
 vb->setSpacing( KDialog::spacingHint() );
 d->mButtonGroup = new QButtonGroup(this);
 d->mButtonGroup->setExclusive(true);
//  vb->addWidget(d->mButtonGroup);
 connect(d->mButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotTypeChanged(int)));
 QRadioButton* buttonCreate = new QRadioButton(i18n("Create a network game"), this);
 d->mButtonGroup->addButton(buttonCreate,0);
 QRadioButton* buttonJoin = new QRadioButton(i18n("Join a network game"), this);
 d->mButtonGroup->addButton(buttonJoin,1);

 QWidget *g = new QWidget(this);
 vb->addWidget(g);
 d->mServerNameLabel = new QLabel(i18n("Game name:"), g);
 d->mServerName = new QLineEdit(g);
 d->mClientNameLabel = new QLabel(i18n("Network games:"), g);
 d->mClientName = new QComboBox(g);

 QGridLayout* layout = new QGridLayout;
 layout->addWidget(d->mServerNameLabel, 0, 0);
 layout->addWidget(d->mServerName, 0, 1);
 layout->addWidget(d->mClientNameLabel, 1, 0);
 layout->addWidget(d->mClientName, 1, 1);
 layout->setSpacing(KDialog::spacingHint());
 g->setLayout(layout);
 connect(d->mClientName,SIGNAL(activated(int)),SLOT(slotGameSelected(int)));
 (void)new QLabel(i18n("Port to connect to:"), g);
 d->mPort = new KIntNumInput(g);
 (void)new QLabel(i18n("Host to connect to:"), g);
 d->mHost = new QLineEdit(g); 

 QPushButton *button=new QPushButton(i18n("&Start Network"), this);
 connect(button, SIGNAL(clicked()), this, SIGNAL(signalNetworkSetup()));
 vb->addWidget(button);
 // Hide until type is set
 d->mClientName->hide();
 d->mClientNameLabel->hide();
 d->mServerName->hide();
 d->mServerNameLabel->hide();
}

void KGameConnectWidget::showDnssdControls() 
{
 if (!d->mBrowser) return;
 if (d->mHost->isEnabled()) {      // client	
  d->mClientName->show();
  d->mClientNameLabel->show();
  d->mServerName->hide();
  d->mServerNameLabel->hide();
  slotGameSelected(d->mClientName->currentIndex());
 } else {
  d->mClientName->hide();
  d->mClientNameLabel->hide();
  d->mServerName->show();
  d->mServerNameLabel->show();
 }
}

void KGameConnectWidget::setType(const QString& type) 
{
 d->mType = type;
 delete d->mBrowser;
 d->mBrowser = new DNSSD::ServiceBrowser(type);
 connect(d->mBrowser,SIGNAL(finished()),SLOT(slotGamesFound()));
 d->mBrowser->startBrowse();
 showDnssdControls();
}

void KGameConnectWidget::slotGamesFound()
{
 bool autoselect=false;
 if (!d->mClientName->count()) autoselect=true;
 d->mClientName->clear();
 QStringList names;
 
 QListIterator<DNSSD::RemoteService::Ptr> it(d->mBrowser->services());
 while (it.hasNext())
  names << it.next()->serviceName();
 d->mClientName->addItems(names);
 if (autoselect && d->mClientName->count()) slotGameSelected(0);
}

void KGameConnectWidget::setName(const QString& name) 
{
 d->mServerName->setText(name);
}

QString KGameConnectWidget::gameName() const
{
 return d->mServerName->text();
}

QString KGameConnectWidget::type() const
{
 return d->mType;
}

void KGameConnectWidget::slotGameSelected(int nr)
{
 if (nr>=(d->mBrowser->services().count()) || nr<0) return;
 if (!d->mHost->isEnabled()) return; // this is server mode, do not overwrite host and port controls
 DNSSD::RemoteService::Ptr srv = d->mBrowser->services()[nr];
 if (!srv->isResolved() && !srv->resolve()) return;
 d->mHost->setText(srv->hostName());
 d->mPort->setValue(srv->port());
}
KGameConnectWidget::~KGameConnectWidget()
{
 delete d->mBrowser;
 delete d;
}

QString KGameConnectWidget::host() const
{ 
 if (d->mHost->isEnabled()) {
	return d->mHost->text();
 } else {
	return QString();
 }
}

unsigned short int KGameConnectWidget::port() const
{
 return d->mPort->value(); 
}

void KGameConnectWidget::setHost(const QString& host)
{ 
 d->mHost->setText(host); 
}

void KGameConnectWidget::setPort(unsigned short int port)
{
 d->mPort->setValue(port); 
}

void KGameConnectWidget::setDefault(int state)
{
 if (d->mButtonGroup->button(state) == 0) {
  kError(11001) << "KGameConnectWidget::setDefault" << state;
  return;
 } 
 d->mButtonGroup->button(state)->setChecked(true);
 slotTypeChanged(state); 
}

void KGameConnectWidget::slotTypeChanged(int t)
{
 if (t == 0) {
	d->mHost->setEnabled(false);
 } else if (t == 1) {
	d->mHost->setEnabled(true);
 }
 showDnssdControls();
 emit signalServerTypeChanged(t);
}

class KGameConnectDialogPrivate
{
 public:
	KGameConnectDialogPrivate()
	{
		mConnect = 0;
	}

	KGameConnectWidget* mConnect;
};

// buttonmask =Ok|Cancel
KGameConnectDialog::KGameConnectDialog(QWidget* parent, ButtonCodes buttonmask) : 
		KDialog(parent)
		//KDialog(Plain,
		//i18n("Network Game"),buttonmask , Ok, parent, 0, true, buttonmask!=0)
{
 setCaption(i18n("Network Game"));
 setButtons(buttonmask);
 setModal(true);
 d = new KGameConnectDialogPrivate;
 QFrame *frame = new QFrame(this);
 setMainWidget(frame);
 QVBoxLayout* vb = new QVBoxLayout(frame);
 vb->setSpacing( spacingHint() );
 d->mConnect = new KGameConnectWidget(frame);
 vb->addWidget(d->mConnect);
}

KGameConnectDialog::~KGameConnectDialog()
{
 delete d;
}

int KGameConnectDialog::initConnection( unsigned short int& port,
		QString& host, QWidget* parent, bool server)
{
 KGameConnectDialog d(parent);
 d.setHost(host);
 d.setPort(port);
 if (server) {
	d.setDefault(0);
 } else {
	d.setDefault(1);
 }

 int result = d.exec();
 if (result == QDialog::Accepted) {
	host = d.host();
	port = d.port();
 }
 return result;
}

QString KGameConnectDialog::host() const
{
 return d->mConnect->host();
}

unsigned short int KGameConnectDialog::port() const
{
 return d->mConnect->port();
}

void KGameConnectDialog::setHost(const QString& host)
{
 d->mConnect->setHost(host);
}

void KGameConnectDialog::setPort(unsigned short int port)
{
 d->mConnect->setPort(port);
}

void KGameConnectDialog::setDefault(int state)
{
 d->mConnect->setDefault(state);
}



#include "kgameconnectdialog.moc"

