/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001 Martin Heni (martin@heni-online.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/


#include "kgameconnectdialog.h"

#include <knuminput.h>
#include <klocale.h>

#include <qlineedit.h>
#include <qvbuttongroup.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qgrid.h>

class KGameConnectWidgetPrivate
{
 public:
	KGameConnectWidgetPrivate()
	{
		mPort = 0;
		mHost = 0;
		mButtonGroup = 0;
	}

	KIntNumInput* mPort;
	QLineEdit* mHost; //KLineEdit?
	QVButtonGroup* mButtonGroup;
};

KGameConnectWidget::KGameConnectWidget(QWidget* parent) : QWidget(parent)
{
 d = new KGameConnectWidgetPrivate;

 QVBoxLayout* vb = new QVBoxLayout(this, KDialog::spacingHint());
 d->mButtonGroup = new QVButtonGroup(this);
 vb->addWidget(d->mButtonGroup);
 connect(d->mButtonGroup, SIGNAL(clicked(int)), this, SLOT(slotTypeChanged(int)));
 (void)new QRadioButton(i18n("Create a network game"), d->mButtonGroup);
 (void)new QRadioButton(i18n("Join a network game"), d->mButtonGroup);

 QGrid* g = new QGrid(2, this);
 vb->addWidget(g);
 g->setSpacing(KDialog::spacingHint());
 (void)new QLabel(i18n("Port to connect to"), g);
 d->mPort = new KIntNumInput(g);
 (void)new QLabel(i18n("Host to connect to"), g);
 d->mHost = new QLineEdit(g); 

 QPushButton *button=new QPushButton(i18n("&Start Network"), this);
 connect(button, SIGNAL(clicked()), this, SIGNAL(signalNetworkSetup()));
 vb->addWidget(button);
}

KGameConnectWidget::~KGameConnectWidget()
{
 delete d;
}

QString KGameConnectWidget::host() const
{ 
 if (d->mHost->isEnabled()) {
	return d->mHost->text();
 } else {
	return QString::null;
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
 d->mButtonGroup->setButton(state); 
 slotTypeChanged(state); 
}

void KGameConnectWidget::slotTypeChanged(int t)
{
 if (t == 0) {
	d->mHost->setEnabled(false);
 } else if (t == 1) {
	d->mHost->setEnabled(true);
 }
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
KGameConnectDialog::KGameConnectDialog(QWidget* parent,int buttonmask) : KDialogBase(Plain,
		i18n("Network Game"),buttonmask , Ok, parent, 0, true, buttonmask!=0)
{
 d = new KGameConnectDialogPrivate;
 QVBoxLayout* vb = new QVBoxLayout(plainPage(), spacingHint());
 d->mConnect = new KGameConnectWidget(plainPage());
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

