/* **************************************************************************
                           KGameConnectDialog Class
                           ------------------------
    begin                : 1 January 2001
    copyright            : (C) 2001 by Andreas Beckermann and Martin Heni
    email                : b_mann@gmx.de and martin@heni-online.de
 ***************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Additional license: Any of the above copyright holders can add an     *
 *   enhanced license which complies with the license of the KDE core      *
 *   libraries so that this file resp. this library is compatible with     *
 *   the KDE core libraries.                                               *
 *   The user of this program shall have the choice which license to use   *
 *                                                                         *
 ***************************************************************************/

#include <qlineedit.h>
#include <qvbuttongroup.h>
#include <qvgroupbox.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qlabel.h>

#include <knuminput.h>
#include <klocale.h>

#include "kgameconnectdialog.h"

class KGameConnectDialogPrivate
{
public:
	KGameConnectDialogPrivate()
	{
		mPort = 0;
		mHost = 0;
		mButtonGroup = 0;
	}

	KIntNumInput* mPort;
	QLineEdit* mHost; //KLineEdit?
	QVButtonGroup* mButtonGroup;
};

KGameConnectDialog::KGameConnectDialog(QWidget* parent) : KDialogBase(Plain,
		i18n("Network Game"), Ok|Cancel, Ok, parent, 0, true, true)
{
 d = new KGameConnectDialogPrivate;
 QVBoxLayout* vb = new QVBoxLayout(plainPage(), spacingHint());
 d->mButtonGroup = new QVButtonGroup(plainPage());
 vb->addWidget(d->mButtonGroup);
 connect(d->mButtonGroup, SIGNAL(clicked(int)), this, SLOT(slotTypeChanged(int)));
 (void)new QRadioButton(i18n("Create a network game"), d->mButtonGroup);
 (void)new QRadioButton(i18n("Join a network game"), d->mButtonGroup);

 QGrid* g = new QGrid(2, plainPage());
 vb->addWidget(g);
 g->setSpacing(spacingHint());
 (void)new QLabel(i18n("Port to connect to"), g);
 d->mPort = new KIntNumInput(g);
 (void)new QLabel(i18n("Host to connect to"), g);
 d->mHost = new QLineEdit(g); 
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
 if (d->mHost->isEnabled()) {
	return d->mHost->text();
 } else {
	return QString::null;
 }
}

unsigned short int KGameConnectDialog::port() const
{ return d->mPort->value(); }
void KGameConnectDialog::setHost(const QString& host)
{ d->mHost->setText(host); }
void KGameConnectDialog::setPort(unsigned short int port)
{ d->mPort->setValue(port); }
void KGameConnectDialog::setDefault(int state)
{ d->mButtonGroup->setButton(state); slotTypeChanged(state); }

void KGameConnectDialog::slotTypeChanged(int t)
{
 if (t == 0) {
	d->mHost->setEnabled(false);
 } else if (t == 1) {
	d->mHost->setEnabled(true);
 }
}
#include "kgameconnectdialog.moc"
