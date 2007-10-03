/*
    This file is part of the kggzgames library.
    Copyright (c) 2007 Josef Spillner <josef@ggzgamingzone.org>

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

#include "kggzrankingsdialog.h"

#include <kggzmod/module.h>

#include <KDebug>
#include <KScoreDialog>
#include <KHighscore>

class KGGZRankingsDialogPrivate
{
	public:
		KGGZRankingsDialogPrivate(KGGZRankingsDialog* qq);

		KGGZRankingsDialog* q;

		KGGZMod::Module *m_mod;
		QWidget *m_parent;

		// slots
		void slotRankings(const KGGZMod::Event& event);
};

KGGZRankingsDialog::KGGZRankingsDialog(QWidget *parent)
: QObject(), d(new KGGZRankingsDialogPrivate(this))
{
	d->m_parent = parent;
	d->m_mod = KGGZMod::Module::instance();

	connect(d->m_mod,
		SIGNAL(signalEvent(const KGGZMod::Event&)),
		this,
		SLOT(slotRankings(const KGGZMod::Event&)));
}

KGGZRankingsDialogPrivate::KGGZRankingsDialogPrivate(KGGZRankingsDialog* qq)
: q(qq)
{
}

KGGZRankingsDialog::~KGGZRankingsDialog()
{
	delete d;
}

void KGGZRankingsDialogPrivate::slotRankings(const KGGZMod::Event& event)
{
	if(event.type() == KGGZMod::Event::rankings)
	{
		KHighscore highscore;
		highscore.setHighscoreGroup("GGZ");

		KGGZMod::RankingsEvent e(event);
		for(int i = 0; i < e.count(); i++)
		{
			QString name = e.name(i);
			int score = e.score(i);
			kDebug(11004) << "RANKINGS-DIALOG" << name << score << endl;
			highscore.writeEntry(i + 1, "Name", name);
			highscore.writeEntry(i + 1, "Score", score);
		}

		KScoreDialog ksdialog(KScoreDialog::Name, m_parent);
		ksdialog.setConfigGroup("GGZ");
		ksdialog.exec();
	}
}

#include "kggzrankingsdialog.moc"
