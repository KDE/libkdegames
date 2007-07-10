/*
Copyright (c) 2007, Nicolas Roffet, <nicolas-kde@roffet.com>

This library is free software; you can redistribute it and/or modify it under the terms of the GNU Library General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for more details.

You should have received a copy of the GNU Library General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#include "kgamedifficulty.h"

#include <kactioncollection.h>
#include <kicon.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kselectaction.h>
#include <kxmlguiwindow.h>

class KGameDifficultyPrivate
{
public:
	KGameDifficultyPrivate(KGameDifficulty* qq);
	~KGameDifficultyPrivate();

	void createActionsAndMore(KXmlGuiWindow* window, bool restartByChange);

	KGameDifficulty* q;

	/**
	 * @brief Current difficulty level
	 */
	int m_level;
	KSelectAction* m_menu;
	bool m_restartByChange;
	bool m_running;
	QStringList m_texts;
};

KGameDifficultyPrivate::KGameDifficultyPrivate(KGameDifficulty* qq)
    : q(qq)
{
}

KGameDifficultyPrivate::~KGameDifficultyPrivate()
{
	delete m_menu;
}

KGameDifficulty::KGameDifficulty(KXmlGuiWindow* window, bool restartByChange, int number)
    : d(new KGameDifficultyPrivate(this))
{
	Q_ASSERT((number>0) && (number<9));

	// Create level list
	if (number>=7)
		d->m_texts << i18n("Ridiculously easy");
	if (number>=5)
		d->m_texts << i18n("Very easy");
	if (number>=3)
		d->m_texts << i18n("Easy");
	d->m_texts << i18n("Medium");
	if (number>=2)
		d->m_texts << i18n("Hard");
	if (number>=4)
		d->m_texts << i18n("Very hard");
	if (number>=6)
		d->m_texts << i18n("Almost impossible");
	if (number>=8)
		d->m_texts << i18n("Impossible");

	d->createActionsAndMore(window, restartByChange);
}


KGameDifficulty::KGameDifficulty(KXmlGuiWindow* window, bool restartByChange, QStringList& texts)
    : d(new KGameDifficultyPrivate(this))
{
	Q_ASSERT(texts.count()>0);

	d->m_texts = texts;

	d->createActionsAndMore(window, restartByChange);
}


KGameDifficulty::~KGameDifficulty()
{
	delete d;
}


void KGameDifficulty::addCustomLevel()
{
	QAction* separator = new QAction(d->m_menu);
	separator->setSeparator(true);
	d->m_menu->addAction(separator);

	d->m_menu->addAction(i18n("Custom..."));
}


void KGameDifficulty::changeLevel(int level)
{
	bool mayAbort = true;

	// In the toolbar as a combobox, it's possible to select the separator!
	if (d->m_menu->action(level)->isSeparator())
		mayAbort = false;

	if (mayAbort && d->m_restartByChange && d->m_running)
		mayAbort = ( KMessageBox::warningContinueCancel(0, i18n("This will be the end of the current game!"), QString(), KGuiItem(i18n("Change the difficulty level"))) == KMessageBox::Continue );

	if (mayAbort)
		setLevel(level);
	else
		// restore current level selection
		setLevel(d->m_level);
}


void KGameDifficulty::setEnabled(bool enabled)
{
	Q_ASSERT(d->m_menu);

	// TODO: Doing this never disable the combobox in the toolbar (just in the menu). It seems to be a bug in the class KSelectAction of kdelibs/kdeui/actions. To check and solve...
	d->m_menu->setEnabled(enabled);
}


void KGameDifficulty::setLevel(int level)
{
	d->m_menu->setCurrentItem(level);
	if (level != d->m_level)
		emit levelChanged(level);
	d->m_level = level;
}


void KGameDifficulty::setRunning(bool running)
{
	d->m_running = running;
}


void KGameDifficultyPrivate::createActionsAndMore(KXmlGuiWindow* window, bool restartByChange)
{
	m_restartByChange = restartByChange;
	q->setParent(window);
	q->setRunning(true);

	m_menu = new KSelectAction(KIcon("games-difficult"), i18n("Game difficulty"), window);
	window->actionCollection()->addAction("game_difficulty", m_menu);
	QObject::connect(m_menu, SIGNAL(triggered(int)), q, SLOT(changeLevel(int)));
	m_menu->setItems(m_texts);
	m_menu->setToolTip(i18n("Set the difficulty level"));
	m_menu->setWhatsThis(i18n("Set the difficulty level of the game."));

	q->setEnabled(true);
}

#include "kgamedifficulty.moc"
