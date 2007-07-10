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



KGameDifficulty::KGameDifficulty(KXmlGuiWindow* window, bool restartByChange, int number)
{
	Q_ASSERT((number>0) && (number<9));

	// Create level list
	if (number>=7)
		m_texts << i18n("Ridiculously easy");
	if (number>=5)
		m_texts << i18n("Very easy");
	if (number>=3)
		m_texts << i18n("Easy");
	m_texts << i18n("Medium");
	if (number>=2)
		m_texts << i18n("Hard");
	if (number>=4)
		m_texts << i18n("Very hard");
	if (number>=6)
		m_texts << i18n("Almost impossible");
	if (number>=8)
		m_texts << i18n("Impossible");

	createActionsAndMore(window, restartByChange);
}


KGameDifficulty::KGameDifficulty(KXmlGuiWindow* window, bool restartByChange, QStringList& texts)
{
	Q_ASSERT(texts.count()>0);

	m_texts = texts;

	createActionsAndMore(window, restartByChange);
}


KGameDifficulty::~KGameDifficulty()
{
	delete m_menu;
}


void KGameDifficulty::addCustomLevel()
{
	QAction* separator = new QAction(m_menu);
	separator->setSeparator(true);
	m_menu->addAction(separator);

	m_menu->addAction(i18n("Custom..."));
}


void KGameDifficulty::changeLevel(int level)
{
	bool mayAbort = true;

	// In the toolbar as a combobox, it's possible to select the separator!
	if (m_menu->action(level)->isSeparator())
		mayAbort = false;

	if (mayAbort && m_restartByChange && m_running)
		mayAbort = ( KMessageBox::warningContinueCancel(0, i18n("This will be the end of the current game!"), QString(), KGuiItem(i18n("Change the difficulty level"))) == KMessageBox::Continue );

	if (mayAbort)
		setLevel(level);
	else
		// restore current level selection
		setLevel(m_level);
}


void KGameDifficulty::setEnabled(bool enabled)
{
	Q_ASSERT(m_menu);

	// TODO: Doing this never disable the combobox in the toolbar (just in the menu). It seems to be a bug in the class KSelectAction of kdelibs/kdeui/actions. To check and solve...
	m_menu->setEnabled(enabled);
}


void KGameDifficulty::setLevel(int level)
{
	m_menu->setCurrentItem(level);
	if (level!=m_level)
		emit levelChanged(level);
	m_level = level;
}


void KGameDifficulty::setRunning(bool running)
{
	m_running = running;
}


void KGameDifficulty::createActionsAndMore(KXmlGuiWindow* window, bool restartByChange)
{
	m_restartByChange = restartByChange;
	setParent(window);
	setRunning(true);

	m_menu = new KSelectAction(KIcon("games-difficult"), i18n("Game difficulty"), window);
	window->actionCollection()->addAction("game_difficulty", m_menu);
	connect(m_menu, SIGNAL(triggered(int)), this, SLOT(changeLevel(int)));
	m_menu->setItems(m_texts);
	m_menu->setToolTip(i18n("Set the difficulty level"));
	m_menu->setWhatsThis(i18n("Set the difficulty level of the game."));

	setEnabled(true);
}

#include "kgamedifficulty.moc"
