/***************************************************************************
 *   Copyright 2007 Nicolas Roffet <nicolas-kde@roffet.com>                *
 *   Copyright 2007 Pino Toscano <toscano.pino@tiscali.it>                 *
 *   Copyright 2011-2012 Stefan Majewsky <majewsky@gmx.net>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License          *
 *   version 2 as published by the Free Software Foundation                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "kgdifficulty.h"

#include <QVector>
#include <KConfigGroup>
#include <KGuiItem>
#include <KMessageBox>
//the following only used by KgDifficultyGUI
#include <KActionCollection>
#include <KComboBox>
#include <KSelectAction>
#include <KXmlGuiWindow>

#include <QCoreApplication>
#include <QIcon>
#include <QGlobalStatic>
#include <QStatusBar>

#include <KLocalizedString>
#include <KSharedConfig>

//BEGIN KgDifficultyLevel

class Q_DECL_HIDDEN KgDifficultyLevel::Private
{
    public:
        bool m_isDefault;
        int m_hardness;
        StandardLevel m_level;
        QByteArray m_key;
        QString m_title;

        Private(int hardness, const QByteArray& key, const QString& title, StandardLevel level, bool isDefault);
        static Private* fromStandardLevel(StandardLevel level, bool isDefault);
};

KgDifficultyLevel::KgDifficultyLevel(int hardness, const QByteArray& key, const QString& title, bool isDefault)
	: d(new Private(hardness, key, title, Custom, isDefault))
{
}

KgDifficultyLevel::Private::Private(int hardness, const QByteArray& key, const QString& title, StandardLevel level, bool isDefault)
	: m_isDefault(isDefault)
	, m_hardness(hardness)
	, m_level(level)
	, m_key(key)
	, m_title(title)
{
}

KgDifficultyLevel::KgDifficultyLevel(StandardLevel level, bool isDefault)
	: d(Private::fromStandardLevel(level, isDefault))
{
}

KgDifficultyLevel::Private* KgDifficultyLevel::Private::fromStandardLevel(KgDifficultyLevel::StandardLevel level, bool isDefault)
{
	Q_ASSERT_X(level != Custom,
		"KgDifficultyLevel(StandardLevel) constructor",
		"Custom level not allowed here"
	);
	//The first entry in the pair is to be used as a key so don't change it. It doesn't have to match the string to be translated
	QPair<QByteArray, QString> data;
	switch (level)
	{
		case RidiculouslyEasy:
			data = qMakePair(QByteArray("Ridiculously Easy"), i18nc("Game difficulty level 1 out of 8", "Ridiculously Easy"));
			break;
		case VeryEasy:
			data = qMakePair(QByteArray("Very Easy"), i18nc("Game difficulty level 2 out of 8", "Very Easy"));
			break;
		case Easy:
			data = qMakePair(QByteArray("Easy"), i18nc("Game difficulty level 3 out of 8", "Easy"));
			break;
		case Medium:
			data = qMakePair(QByteArray("Medium"), i18nc("Game difficulty level 4 out of 8", "Medium"));
			break;
		case Hard:
			data = qMakePair(QByteArray("Hard"), i18nc("Game difficulty level 5 out of 8", "Hard"));
			break;
		case VeryHard:
			data = qMakePair(QByteArray("Very Hard"), i18nc("Game difficulty level 6 out of 8", "Very Hard"));
			break;
		case ExtremelyHard:
			data = qMakePair(QByteArray("Extremely Hard"), i18nc("Game difficulty level 7 out of 8", "Extremely Hard"));
			break;
		case Impossible:
			data = qMakePair(QByteArray("Impossible"), i18nc("Game difficulty level 8 out of 8", "Impossible"));
			break;
		case Custom:
			return nullptr;
	}
	return new KgDifficultyLevel::Private(level, data.first, data.second, level, isDefault);
}

KgDifficultyLevel::~KgDifficultyLevel()
{
	delete d;
}

bool KgDifficultyLevel::isDefault() const
{
	return d->m_isDefault;
}

int KgDifficultyLevel::hardness() const
{
	return d->m_hardness;
}

QByteArray KgDifficultyLevel::key() const
{
	return d->m_key;
}

QString KgDifficultyLevel::title() const
{
	return d->m_title;
}

KgDifficultyLevel::StandardLevel KgDifficultyLevel::standardLevel() const
{
	return d->m_level;
}

//END KgDifficultyLevel
//BEGIN KgDifficulty

class Q_DECL_HIDDEN KgDifficulty::Private
{
    public:
        QList<const KgDifficultyLevel*> m_levels;
        const KgDifficultyLevel* m_currentLevel;
        bool m_editable, m_gameRunning;

        Private() : m_currentLevel(nullptr), m_editable(true), m_gameRunning(false) {}
};

static void saveLevel()
{
	//save current difficulty level in config file (no sync() call here; this
	//will most likely be called at application shutdown when others are also
	//writing to KGlobal::config(); also KConfig's dtor will sync automatically)
	KConfigGroup cg(KSharedConfig::openConfig(), "KgDifficulty");
	cg.writeEntry("Level", Kg::difficulty()->currentLevel()->key());
}

KgDifficulty::KgDifficulty(QObject* parent)
	: QObject(parent)
	, d(new Private)
{
	qRegisterMetaType<const KgDifficultyLevel*>();
	qAddPostRoutine(saveLevel);
}

KgDifficulty::~KgDifficulty()
{
	qDeleteAll(d->m_levels);
}

void KgDifficulty::addLevel(KgDifficultyLevel* level)
{
	//The intended use is to create the KgDifficulty object, add levels, *then*
	//start to work with the currentLevel(). The first call to currentLevel()
	//will load the previous selection from the config, and the level list will
	//be considered immutable from this point.
	Q_ASSERT_X(d->m_currentLevel == nullptr,
		"KgDifficulty::addLevel",
		"Only allowed before currentLevel() is called."
	);
	//ensure that list stays sorted
	QList<const KgDifficultyLevel*>::iterator it = d->m_levels.begin();
	while (it != d->m_levels.end() && (*it)->hardness() < level->hardness())
	{
		++it;
	}
	d->m_levels.insert(it, level);
	level->setParent(this);
}

typedef KgDifficultyLevel::StandardLevel DS;

void KgDifficulty::addStandardLevel(DS level, bool isDefault)
{
	addLevel(new KgDifficultyLevel(level, isDefault));
}

void KgDifficulty::addStandardLevelRange(DS from, DS to)
{
	//every level in range != Custom, therefore no level is default
	addStandardLevelRange(from, to, KgDifficultyLevel::Custom);
}

void KgDifficulty::addStandardLevelRange(DS from, DS to, DS defaultLevel)
{
	const QVector<DS> levels = QVector<DS>()
		<< KgDifficultyLevel::RidiculouslyEasy
		<< KgDifficultyLevel::VeryEasy
		<< KgDifficultyLevel::Easy
		<< KgDifficultyLevel::Medium
		<< KgDifficultyLevel::Hard
		<< KgDifficultyLevel::VeryHard
		<< KgDifficultyLevel::ExtremelyHard
		<< KgDifficultyLevel::Impossible
	;
	const int fromIndex = levels.indexOf(from);
	const int toIndex = levels.indexOf(to);
	const int defaultLevelIndex = levels.indexOf(defaultLevel);
	Q_ASSERT_X(fromIndex >= 0 && toIndex > fromIndex && (defaultLevelIndex == KgDifficultyLevel::Custom || (defaultLevelIndex >= fromIndex && defaultLevelIndex <= toIndex)),
		"KgDifficulty::addStandardLevelRange",
		"No argument may be KgDifficultyLevel::Custom."
	);
	for (int i = fromIndex; i <= toIndex; ++i)
	{
		addLevel(new KgDifficultyLevel(levels[i], levels[i] == defaultLevel));
	}
}

QList<const KgDifficultyLevel*> KgDifficulty::levels() const
{
	return d->m_levels;
}

const KgDifficultyLevel* KgDifficulty::currentLevel() const
{
	if (d->m_currentLevel)
	{
		return d->m_currentLevel;
	}
	Q_ASSERT(!d->m_levels.isEmpty());
	//check configuration file for saved difficulty level
	KConfigGroup cg(KSharedConfig::openConfig(), "KgDifficulty");
	const QByteArray key = cg.readEntry("Level", QByteArray());
	for (const KgDifficultyLevel* level : qAsConst(d->m_levels)) {
		if (level->key() == key)
		{
			return d->m_currentLevel = level;
		}
	}
	//no level predefined - look for a default level
	for (const KgDifficultyLevel* level : qAsConst(d->m_levels)) {
		if (level->isDefault())
		{
			return d->m_currentLevel = level;
		}
	}
	//no default level predefined - easiest level is probably a sane default
	return d->m_currentLevel = d->m_levels[0];
}

bool KgDifficulty::isEditable() const
{
	return d->m_editable;
}

void KgDifficulty::setEditable(bool editable)
{
	if (d->m_editable == editable)
	{
		return;
	}
	d->m_editable = editable;
	Q_EMIT editableChanged(editable);
}

bool KgDifficulty::isGameRunning() const
{
	return d->m_gameRunning;
}

void KgDifficulty::setGameRunning(bool gameRunning)
{
	if (d->m_gameRunning == gameRunning)
	{
		return;
	}
	d->m_gameRunning = gameRunning;
	Q_EMIT gameRunningChanged(gameRunning);
}

void KgDifficulty::select(const KgDifficultyLevel* level)
{
	Q_ASSERT(d->m_levels.contains(level));
	if (d->m_currentLevel == level)
	{
		return;
	}
	//ask for confirmation if necessary
	if (d->m_gameRunning)
	{
		const int result = KMessageBox::warningContinueCancel(nullptr,
			i18n("Changing the difficulty level will end the current game!"),
			QString(), KGuiItem(i18n("Change the difficulty level"))
		);
		if (result != KMessageBox::Continue)
		{
			Q_EMIT selectedLevelChanged(d->m_currentLevel);
			return;
		}
	}
	d->m_currentLevel = level;
	Q_EMIT selectedLevelChanged(level);
	Q_EMIT currentLevelChanged(level);
}

//END KgDifficulty

Q_GLOBAL_STATIC(KgDifficulty, g_difficulty)

KgDifficulty* Kg::difficulty()
{
	return g_difficulty;
}

KgDifficultyLevel::StandardLevel Kg::difficultyLevel()
{
	return g_difficulty->currentLevel()->standardLevel();
}

//BEGIN KgDifficultyGUI

namespace KgDifficultyGUI
{
	class Selector : public KComboBox
	{
		Q_OBJECT
		private:
			KgDifficulty* d;
		public:
			Selector(KgDifficulty* difficulty, QWidget* parent = nullptr)
				: KComboBox(parent), d(difficulty) {}
		Q_SIGNALS:
			void signalSelected(int levelIndex);
		public Q_SLOTS:
			void slotActivated(int levelIndex)
			{
				d->select(d->levels().value(levelIndex));
			}
			void slotSelected(const KgDifficultyLevel* level)
			{
				Q_EMIT signalSelected(d->levels().indexOf(level));
			}
	};
	class Menu : public KSelectAction
	{
		Q_OBJECT
		public:
			Menu(const QIcon& i, const QString& s, QWidget* p) : KSelectAction(i,s,p){}
		public Q_SLOTS:
			//this whole class just because the following is not a slot
			void setCurrentItem(int index) { KSelectAction::setCurrentItem(index); }
	};
}

void KgDifficultyGUI::init(KXmlGuiWindow* window, KgDifficulty* difficulty)
{
	const bool useSingleton = !difficulty;
	if (useSingleton)
		difficulty = Kg::difficulty();

	//create selector (resides in status bar)
	KgDifficultyGUI::Selector* selector = new KgDifficultyGUI::Selector(difficulty, window);
	selector->setToolTip(i18nc("Game difficulty level", "Difficulty"));
	QObject::connect(selector, SIGNAL(activated(int)), selector, SLOT(slotActivated(int)));
	QObject::connect(difficulty, &KgDifficulty::editableChanged, selector, &QWidget::setEnabled);
	QObject::connect(difficulty, &KgDifficulty::selectedLevelChanged,
		selector, &Selector::slotSelected);
	QObject::connect(selector, &Selector::signalSelected, selector, &QComboBox::setCurrentIndex);

	//create menu action
	const QIcon icon = QIcon::fromTheme(QStringLiteral("games-difficult"));
	KSelectAction* menu = new KgDifficultyGUI::Menu(icon, i18nc("Game difficulty level", "Difficulty"), window);
	menu->setToolTip(i18n("Set the difficulty level"));
	menu->setWhatsThis(i18n("Set the difficulty level of the game."));
	QObject::connect(menu, SIGNAL(triggered(int)), selector, SLOT(slotActivated(int)));
	QObject::connect(difficulty, &KgDifficulty::editableChanged, menu, &QAction::setEnabled);
	QObject::connect(selector, &Selector::signalSelected, menu, &KSelectAction::setCurrentItem);

	//fill menu and selector
	const auto levels = difficulty->levels();
	for (const KgDifficultyLevel* level : levels) {
		selector->addItem(icon, level->title());
		menu->addAction(level->title());
	}
	//initialize selection in selector
	selector->slotSelected(difficulty->currentLevel());

	//add selector to statusbar
	window->statusBar()->addPermanentWidget(selector);
	//add menu action to window
	menu->setObjectName(QStringLiteral("options_game_difficulty"));
	window->actionCollection()->addAction(menu->objectName(), menu);

	//ensure that the KgDifficulty instance gets deleted
	if (!useSingleton && !difficulty->parent())
	{
		difficulty->setParent(window);
	}
}

//END KgDifficultyGUI

#include "kgdifficulty.moc"
#include "moc_kgdifficulty.cpp"
