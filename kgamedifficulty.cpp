/*
Copyright (c) 2007, Nicolas Roffet, <nicolas-kde@roffet.com>
Copyright (c) 2007, Pino Toscano, <toscano.pino@tiscali.it>

This library is free software; you can redistribute it and/or modify it under the terms of the GNU Library General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for more details.

You should have received a copy of the GNU Library General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#include "kgamedifficulty.h"



#include <QMap>


#include <kactioncollection.h>
#include <kicon.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kselectaction.h>
#include <kxmlguiwindow.h>



class KGameDifficultyPrivate : public QObject
{
	Q_OBJECT

	public:
		KGameDifficultyPrivate(KXmlGuiWindow* window, const QObject* recvr, const char* slotStandard, const char* slotCustom);
		~KGameDifficultyPrivate();

		void rebuildActions();
		void emitStandardLevelChanged(KGameDifficulty::standardLevel level);
		void emitCustomLevelChanged(int key);
	
		/**
		* @brief Current custom difficulty level
		*/
		int m_levelCustom;

		KGameDifficulty::standardLevel m_level;
		QList<KGameDifficulty::standardLevel> m_standardLevels;
		QMap<int, QString> m_customLevels;

		KSelectAction* m_menu;
		KGameDifficulty::onChange m_restartOnChange;
		bool m_running;
		int m_oldSelection;


	public Q_SLOTS:
		/**
		 * @brief Player wants to change the difficulty level to a standard level
		 *
		 * The difference with the methode "setSelection" is that the player may have to confirm that he agrees to end the current game (if needed).
		 * @param newSelection Selected item.
		 */
		void changeSelection(int newSelection);

	Q_SIGNALS:
		/**
		 * @brief Current difficulty level changed to a standard level
		 *
		 * The game catchs this signal and restarts a game with the new standard difficulty level.
		 * @param level New standard level.
		 */
		void standardLevelChanged(KGameDifficulty::standardLevel level);

		/**
		 * @brief Current difficulty level changed to a custom level
		 *
		 * The game catchs this signal and restarts a game with the new standard difficulty level.
		 * @param key Custom level identifier.
		 */
		void customLevelChanged(int key);


	private:
		void setSelection(int newSelection);
};


KGameDifficultyPrivate::KGameDifficultyPrivate(KXmlGuiWindow* window, const QObject* recvr, const char* slotStandard, const char* slotCustom = 0)
{
	Q_ASSERT(recvr!=0);

	m_oldSelection = -1; // No valid selection
	m_level = KGameDifficulty::noLevel;
	m_restartOnChange = KGameDifficulty::restartOnChange;
	m_running = false;

	QObject::connect(this, SIGNAL(standardLevelChanged(KGameDifficulty::standardLevel)), recvr, slotStandard);
	if (slotCustom!=0)
		QObject::connect(this, SIGNAL(customLevelChanged(int)), recvr, slotCustom);

	m_menu = new KSelectAction(KIcon("games-difficult"), i18n("Game difficulty"), window);
	m_menu->setToolTip(i18n("Set the difficulty level"));
	m_menu->setWhatsThis(i18n("Set the difficulty level of the game."));
	QObject::connect(m_menu, SIGNAL(triggered(int)), this, SLOT(changeSelection(int)));
	window->actionCollection()->addAction("game_difficulty", m_menu);

	setParent(window);
}


KGameDifficultyPrivate::~KGameDifficultyPrivate()
{
	delete KGameDifficulty::self();
}


void KGameDifficultyPrivate::changeSelection(int newSelection)
{
	bool mayChange = true;

	// In the toolbar as a combobox, it's possible to select the separator!
	// TODO: Check if this is a bug in KSelectAction
	if (m_menu->action(newSelection)->isSeparator())
		mayChange = false;

	if (mayChange && (m_restartOnChange==KGameDifficulty::restartOnChange) && m_running)
		mayChange = ( KMessageBox::warningContinueCancel(0, i18n("This will be the end of the current game!"), QString(), KGuiItem(i18n("Change the difficulty level"))) == KMessageBox::Continue );

	if (mayChange) {
		setSelection(newSelection);
	}
	else {
		// restore current level selection
		setSelection(m_oldSelection);
	}
}


void KGameDifficultyPrivate::rebuildActions()
{
	m_menu->clear();
	qSort(m_standardLevels.begin(), m_standardLevels.end());

	QStringList texts;
	foreach(KGameDifficulty::standardLevel level, m_standardLevels) {
		switch (level) {
			case KGameDifficulty::ridiculouslyEasy:
				texts << i18n("Ridiculously easy");
				break;
			case KGameDifficulty::veryEasy:
				texts << i18n("Very easy");
				break;
			case KGameDifficulty::easy:
				texts << i18n("Easy");
				break;
			case KGameDifficulty::medium:
				texts << i18n("Medium");
				break;
			case KGameDifficulty::hard:
				texts << i18n("Hard");
				break;
			case KGameDifficulty::veryHard:
				texts << i18n("Very hard");
				break;
			case KGameDifficulty::extremelyHard:
				texts << i18n("Extremely hard");
				break;
			case KGameDifficulty::impossible:
				texts << i18n("Impossible");
				break;
			case KGameDifficulty::custom:
			case KGameDifficulty::configurable:
			case KGameDifficulty::noLevel:
				// Do nothing
				break;
		}
	}
	m_menu->setItems(texts);

	if (m_customLevels.count()>0) {
		foreach(QString s, m_customLevels)
			m_menu->addAction(s);
	}

	if (m_standardLevels.contains(KGameDifficulty::configurable)) {
		QAction* separator = new QAction(m_menu);
		separator->setSeparator(true);
		m_menu->addAction(separator);

		m_menu->addAction(i18n("Custom..."));
	}

	// reselect the previous selected item.
	if (m_level==KGameDifficulty::custom)
		KGameDifficulty::setLevelCustom(m_levelCustom);
	else if (m_standardLevels.contains(m_level))
		KGameDifficulty::setLevel(m_level);
}


void KGameDifficultyPrivate::emitStandardLevelChanged(KGameDifficulty::standardLevel level)
{
	emit standardLevelChanged(level);
}


void KGameDifficultyPrivate::emitCustomLevelChanged(int key)
{
	emit customLevelChanged(key);
}


void KGameDifficultyPrivate::setSelection(int newSelection)
{
	int countWithoutConfigurable = m_standardLevels.count();
	if (m_standardLevels.contains(KGameDifficulty::configurable))
		countWithoutConfigurable--;

	if ((m_standardLevels.contains(KGameDifficulty::configurable)) && (newSelection==m_menu->actions().count()-1))
		KGameDifficulty::setLevel(KGameDifficulty::configurable);
	else if(newSelection<countWithoutConfigurable)
		KGameDifficulty::setLevel(m_standardLevels[newSelection]);
	else
		KGameDifficulty::setLevelCustom((m_customLevels.uniqueKeys()).value(newSelection - countWithoutConfigurable));

	m_oldSelection = newSelection;
}


//---//


KGameDifficulty* KGameDifficulty::instance = 0;


KGameDifficulty::~KGameDifficulty()
{
	// We do not need to delete d, because d deletes us.
}


KGameDifficulty* KGameDifficulty::self()
{
	if (instance==0)
		instance = new KGameDifficulty();
	return instance;
}


void KGameDifficulty::init(KXmlGuiWindow* window, const QObject* recvr, const char* slotStandard, const char* slotCustom)
{
	Q_ASSERT(self()->d==0);

	self()->d = new KGameDifficultyPrivate(window, recvr, slotStandard, slotCustom);
}


void KGameDifficulty::setRestartOnChange(onChange restart)
{
	Q_ASSERT(self()->d);

	self()->d->m_restartOnChange = restart;
}


void KGameDifficulty::addStandardLevel(standardLevel level)
{
	Q_ASSERT(self()->d);

	self()->d->m_standardLevels.append(level);
	self()->d->rebuildActions();
}


void KGameDifficulty::removeStandardLevel(standardLevel level)
{
	Q_ASSERT(self()->d);

	self()->d->m_standardLevels.removeAll(level);
	self()->d->rebuildActions();
}


void KGameDifficulty::addCustomLevel(int key, const QString& appellation)
{
	Q_ASSERT(self()->d);

	self()->d->m_customLevels.insert(key, appellation);
	self()->d->rebuildActions();
}


void KGameDifficulty::removeCustomLevel(int key)
{
	Q_ASSERT(self()->d);

	self()->d->m_customLevels.remove(key);
	self()->d->rebuildActions();
}


void KGameDifficulty::setEnabled(bool enabled)
{
	Q_ASSERT(self()->d->m_menu);

	// TODO: Doing this never disable the combobox in the toolbar (just in the menu). It seems to be a bug in the class KSelectAction of kdelibs/kdeui/actions. To check and solve...
	self()->d->m_menu->setEnabled(enabled);
}


void KGameDifficulty::setLevel(standardLevel level)
{
	Q_ASSERT(self()->d);

	if (level==configurable)
		self()->d->m_menu->setCurrentItem(self()->d->m_menu->actions().count()-1);
	else if (level!=custom) {
		self()->d->m_menu->setCurrentItem(self()->d->m_standardLevels.indexOf(level));
	}

	if (level != self()->d->m_level)
		self()->d->emitStandardLevelChanged(level);
	self()->d->m_level = level;

	self()->d->m_oldSelection = self()->d->m_menu->currentItem();
}


KGameDifficulty::standardLevel KGameDifficulty::level()
{
	return self()->d->m_level;
}


void KGameDifficulty::setLevelCustom(int key)
{
	Q_ASSERT(self()->d);

	self()->d->m_level = custom;

	int a = self()->d->m_standardLevels.count();
	if (self()->d->m_standardLevels.contains(configurable))
		a -= 1;
	self()->d->m_menu->setCurrentItem((self()->d->m_customLevels.uniqueKeys()).indexOf(key) + a);

	if (key != self()->d->m_levelCustom)
		self()->d->emitCustomLevelChanged(key);
	self()->d->m_levelCustom = key;

	self()->d->m_oldSelection = self()->d->m_menu->currentItem();
}


int KGameDifficulty::levelCustom()
{
	return self()->d->m_levelCustom;
}


void KGameDifficulty::setRunning(bool running)
{
	Q_ASSERT(self()->d);

	self()->d->m_running = running;
}


KGameDifficulty::KGameDifficulty()
{
	d = 0;
}

#include "kgamedifficulty.moc"
