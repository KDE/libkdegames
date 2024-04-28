/*
    SPDX-FileCopyrightText: 2007 Nicolas Roffet <nicolas-kde@roffet.com>
    SPDX-FileCopyrightText: 2007 Pino Toscano <toscano.pino@tiscali.it>
    SPDX-FileCopyrightText: 2011-2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgamedifficulty.h"

// KF
#include <KActionCollection>
#include <KComboBox>
#include <KConfigGroup>
#include <KGuiItem>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSelectAction>
#include <KSharedConfig>
#include <KXmlGuiWindow>
// Qt
#include <QCoreApplication>
#include <QIcon>
#include <QList>
#include <QStatusBar>
// Std
#include <algorithm>
#include <utility>

// BEGIN KGameDifficultyLevel

class KGameDifficultyLevelPrivate
{
public:
    bool m_isDefault;
    int m_hardness;
    KGameDifficultyLevel::StandardLevel m_level;
    QByteArray m_key;
    QString m_title;

public:
    KGameDifficultyLevelPrivate(int hardness, const QByteArray &key, const QString &title, KGameDifficultyLevel::StandardLevel level, bool isDefault);
    static KGameDifficultyLevelPrivate *fromStandardLevel(KGameDifficultyLevel::StandardLevel level, bool isDefault);
};

KGameDifficultyLevel::KGameDifficultyLevel(int hardness, const QByteArray &key, const QString &title, bool isDefault)
    : d_ptr(new KGameDifficultyLevelPrivate(hardness, key, title, Custom, isDefault))
{
}

KGameDifficultyLevelPrivate::KGameDifficultyLevelPrivate(int hardness,
                                                         const QByteArray &key,
                                                         const QString &title,
                                                         KGameDifficultyLevel::StandardLevel level,
                                                         bool isDefault)
    : m_isDefault(isDefault)
    , m_hardness(hardness)
    , m_level(level)
    , m_key(key)
    , m_title(title)
{
}

KGameDifficultyLevel::KGameDifficultyLevel(StandardLevel level, bool isDefault)
    : d_ptr(KGameDifficultyLevelPrivate::fromStandardLevel(level, isDefault))
{
}

KGameDifficultyLevelPrivate *KGameDifficultyLevelPrivate::fromStandardLevel(KGameDifficultyLevel::StandardLevel level, bool isDefault)
{
    Q_ASSERT_X(level != KGameDifficultyLevel::Custom, "KGameDifficultyLevel(StandardLevel) constructor", "Custom level not allowed here");
    // The first entry in the pair is to be used as a key so don't change it. It doesn't have to match the string to be translated
    QPair<QByteArray, QString> data;
    switch (level) {
    case KGameDifficultyLevel::RidiculouslyEasy:
        data = qMakePair(QByteArrayLiteral("Ridiculously Easy"), i18nc("Game difficulty level 1 out of 8", "Ridiculously Easy"));
        break;
    case KGameDifficultyLevel::VeryEasy:
        data = qMakePair(QByteArrayLiteral("Very Easy"), i18nc("Game difficulty level 2 out of 8", "Very Easy"));
        break;
    case KGameDifficultyLevel::Easy:
        data = qMakePair(QByteArrayLiteral("Easy"), i18nc("Game difficulty level 3 out of 8", "Easy"));
        break;
    case KGameDifficultyLevel::Medium:
        data = qMakePair(QByteArrayLiteral("Medium"), i18nc("Game difficulty level 4 out of 8", "Medium"));
        break;
    case KGameDifficultyLevel::Hard:
        data = qMakePair(QByteArrayLiteral("Hard"), i18nc("Game difficulty level 5 out of 8", "Hard"));
        break;
    case KGameDifficultyLevel::VeryHard:
        data = qMakePair(QByteArrayLiteral("Very Hard"), i18nc("Game difficulty level 6 out of 8", "Very Hard"));
        break;
    case KGameDifficultyLevel::ExtremelyHard:
        data = qMakePair(QByteArrayLiteral("Extremely Hard"), i18nc("Game difficulty level 7 out of 8", "Extremely Hard"));
        break;
    case KGameDifficultyLevel::Impossible:
        data = qMakePair(QByteArrayLiteral("Impossible"), i18nc("Game difficulty level 8 out of 8", "Impossible"));
        break;
    case KGameDifficultyLevel::Custom:
        return nullptr;
    }
    return new KGameDifficultyLevelPrivate(level, data.first, data.second, level, isDefault);
}

KGameDifficultyLevel::~KGameDifficultyLevel() = default;

bool KGameDifficultyLevel::isDefault() const
{
    Q_D(const KGameDifficultyLevel);

    return d->m_isDefault;
}

int KGameDifficultyLevel::hardness() const
{
    Q_D(const KGameDifficultyLevel);

    return d->m_hardness;
}

QByteArray KGameDifficultyLevel::key() const
{
    Q_D(const KGameDifficultyLevel);

    return d->m_key;
}

QString KGameDifficultyLevel::title() const
{
    Q_D(const KGameDifficultyLevel);

    return d->m_title;
}

KGameDifficultyLevel::StandardLevel KGameDifficultyLevel::standardLevel() const
{
    Q_D(const KGameDifficultyLevel);

    return d->m_level;
}

// END KGameDifficultyLevel
// BEGIN KGameDifficulty

class KGameDifficultyPrivate
{
public:
    QList<const KGameDifficultyLevel *> m_levels;
    mutable const KGameDifficultyLevel *m_currentLevel = nullptr;
    bool m_editable = true;
    bool m_gameRunning = false;

public:
    KGameDifficultyPrivate() = default;
};

static void saveLevel()
{
    // save current difficulty level in config file (no sync() call here; this
    // will most likely be called at application shutdown when others are also
    // writing to KGlobal::config(); also KConfig's dtor will sync automatically)
    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KgDifficulty"));
    cg.writeEntry("Level", KGameDifficulty::global()->currentLevel()->key());
}

KGameDifficulty::KGameDifficulty(QObject *parent)
    : QObject(parent)
    , d_ptr(new KGameDifficultyPrivate)
{
    qRegisterMetaType<const KGameDifficultyLevel *>();
    qAddPostRoutine(saveLevel);
}

KGameDifficulty::~KGameDifficulty()
{
    Q_D(KGameDifficulty);

    qDeleteAll(d->m_levels);
}

void KGameDifficulty::addLevel(KGameDifficultyLevel *level)
{
    Q_D(KGameDifficulty);

    // The intended use is to create the KGameDifficulty object, add levels, *then*
    // start to work with the currentLevel(). The first call to currentLevel()
    // will load the previous selection from the config, and the level list will
    // be considered immutable from this point.
    Q_ASSERT_X(d->m_currentLevel == nullptr, "KGameDifficulty::addLevel", "Only allowed before currentLevel() is called.");
    // ensure that list stays sorted
    const int newLevelHardness = level->hardness();
    auto it = std::find_if(d->m_levels.begin(), d->m_levels.end(), [newLevelHardness](const KGameDifficultyLevel *l) {
        return l->hardness() >= newLevelHardness;
    });
    d->m_levels.insert(it, level);
    level->setParent(this);
}

typedef KGameDifficultyLevel::StandardLevel DS;

void KGameDifficulty::addStandardLevel(DS level, bool isDefault)
{
    addLevel(new KGameDifficultyLevel(level, isDefault));
}

void KGameDifficulty::addStandardLevelRange(DS from, DS to)
{
    // every level in range != Custom, therefore no level is default
    addStandardLevelRange(from, to, KGameDifficultyLevel::Custom);
}

void KGameDifficulty::addStandardLevelRange(DS from, DS to, DS defaultLevel)
{
    const QList<DS> levels{
        KGameDifficultyLevel::RidiculouslyEasy,
        KGameDifficultyLevel::VeryEasy,
        KGameDifficultyLevel::Easy,
        KGameDifficultyLevel::Medium,
        KGameDifficultyLevel::Hard,
        KGameDifficultyLevel::VeryHard,
        KGameDifficultyLevel::ExtremelyHard,
        KGameDifficultyLevel::Impossible,
    };
    const int fromIndex = levels.indexOf(from);
    const int toIndex = levels.indexOf(to);
    const int defaultLevelIndex = levels.indexOf(defaultLevel);
    Q_ASSERT_X(fromIndex >= 0 && toIndex > fromIndex
                   && (defaultLevelIndex == KGameDifficultyLevel::Custom || (defaultLevelIndex >= fromIndex && defaultLevelIndex <= toIndex)),
               "KGameDifficulty::addStandardLevelRange",
               "No argument may be KGameDifficultyLevel::Custom.");
    for (int i = fromIndex; i <= toIndex; ++i) {
        addLevel(new KGameDifficultyLevel(levels[i], levels[i] == defaultLevel));
    }
}

QList<const KGameDifficultyLevel *> KGameDifficulty::levels() const
{
    Q_D(const KGameDifficulty);

    return d->m_levels;
}

const KGameDifficultyLevel *KGameDifficulty::currentLevel() const
{
    Q_D(const KGameDifficulty);

    if (d->m_currentLevel) {
        return d->m_currentLevel;
    }
    Q_ASSERT(!d->m_levels.isEmpty());
    // check configuration file for saved difficulty level
    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("KgDifficulty"));
    const QByteArray key = cg.readEntry("Level", QByteArray());
    for (const KGameDifficultyLevel *level : std::as_const(d->m_levels)) {
        if (level->key() == key) {
            return d->m_currentLevel = level;
        }
    }
    // no level predefined - look for a default level
    for (const KGameDifficultyLevel *level : std::as_const(d->m_levels)) {
        if (level->isDefault()) {
            return d->m_currentLevel = level;
        }
    }
    // no default level predefined - easiest level is probably a sane default
    return d->m_currentLevel = d->m_levels[0];
}

bool KGameDifficulty::isEditable() const
{
    Q_D(const KGameDifficulty);

    return d->m_editable;
}

void KGameDifficulty::setEditable(bool editable)
{
    Q_D(KGameDifficulty);

    if (d->m_editable == editable) {
        return;
    }
    d->m_editable = editable;
    Q_EMIT editableChanged(editable);
}

bool KGameDifficulty::isGameRunning() const
{
    Q_D(const KGameDifficulty);

    return d->m_gameRunning;
}

void KGameDifficulty::setGameRunning(bool gameRunning)
{
    Q_D(KGameDifficulty);

    if (d->m_gameRunning == gameRunning) {
        return;
    }
    d->m_gameRunning = gameRunning;
    Q_EMIT gameRunningChanged(gameRunning);
}

void KGameDifficulty::select(const KGameDifficultyLevel *level)
{
    Q_D(KGameDifficulty);

    Q_ASSERT(d->m_levels.contains(level));
    if (d->m_currentLevel == level) {
        return;
    }
    // ask for confirmation if necessary
    if (d->m_gameRunning) {
        const int result = KMessageBox::warningContinueCancel(nullptr,
                                                              i18n("Changing the difficulty level will end the current game!"),
                                                              QString(),
                                                              KGuiItem(i18nc("@action:button", "Change the Difficulty Level")));
        if (result != KMessageBox::Continue) {
            Q_EMIT selectedLevelChanged(d->m_currentLevel);
            return;
        }
    }
    d->m_currentLevel = level;
    Q_EMIT selectedLevelChanged(level);
    Q_EMIT currentLevelChanged(level);
}

// END KGameDifficulty

Q_GLOBAL_STATIC(KGameDifficulty, g_difficulty)

KGameDifficulty *KGameDifficulty::global()
{
    return g_difficulty;
}

KGameDifficultyLevel::StandardLevel KGameDifficulty::globalLevel()
{
    return g_difficulty->currentLevel()->standardLevel();
}

// BEGIN KGameDifficultyGUI

namespace KGameDifficultyGUI
{
class Selector : public KComboBox
{
    Q_OBJECT

private:
    KGameDifficulty *d;

public:
    Selector(KGameDifficulty *difficulty, QWidget *parent = nullptr)
        : KComboBox(parent)
        , d(difficulty)
    {
    }

Q_SIGNALS:
    void signalSelected(int levelIndex);

public Q_SLOTS:
    void slotActivated(int levelIndex)
    {
        d->select(d->levels().value(levelIndex));
    }
    void slotSelected(const KGameDifficultyLevel *level)
    {
        Q_EMIT signalSelected(d->levels().indexOf(level));
    }
};

class Menu : public KSelectAction
{
    Q_OBJECT

public:
    Menu(const QIcon &i, const QString &s, QWidget *p)
        : KSelectAction(i, s, p)
    {
    }

public Q_SLOTS:
    // this whole class just because the following is not a slot
    void setCurrentItem(int index)
    {
        KSelectAction::setCurrentItem(index);
    }
};
}

void KGameDifficultyGUI::init(KXmlGuiWindow *window, KGameDifficulty *difficulty)
{
    const bool useSingleton = !difficulty;
    if (useSingleton)
        difficulty = KGameDifficulty::global();

    // create selector (resides in status bar)
    KGameDifficultyGUI::Selector *selector = new KGameDifficultyGUI::Selector(difficulty, window);
    selector->setToolTip(i18nc("@info:tooltip Game difficulty level", "Difficulty"));
    QObject::connect(selector, &QComboBox::activated, selector, &Selector::slotActivated);
    QObject::connect(difficulty, &KGameDifficulty::editableChanged, selector, &QWidget::setEnabled);
    QObject::connect(difficulty, &KGameDifficulty::selectedLevelChanged, selector, &Selector::slotSelected);
    QObject::connect(selector, &Selector::signalSelected, selector, &QComboBox::setCurrentIndex);

    // create menu action
    const QIcon icon = QIcon::fromTheme(QStringLiteral("games-difficult"));
    KSelectAction *menu = new KGameDifficultyGUI::Menu(icon, i18nc("@title:menu Game difficulty level", "Difficulty"), window);
    menu->setToolTip(i18nc("@info:tooltip", "Set the difficulty level"));
    menu->setWhatsThis(i18nc("@info:whatsthis", "Sets the difficulty level of the game."));
    QObject::connect(menu, &KSelectAction::indexTriggered, selector, &Selector::slotActivated);
    QObject::connect(difficulty, &KGameDifficulty::editableChanged, menu, &QAction::setEnabled);
    QObject::connect(selector, &Selector::signalSelected, menu, &KSelectAction::setCurrentItem);

    // fill menu and selector
    const auto levels = difficulty->levels();
    for (const KGameDifficultyLevel *level : levels) {
        selector->addItem(icon, level->title());
        menu->addAction(level->title());
    }
    // initialize selection in selector
    selector->slotSelected(difficulty->currentLevel());

    // add selector to statusbar
    window->statusBar()->addPermanentWidget(selector);
    // add menu action to window
    menu->setObjectName(QStringLiteral("options_game_difficulty"));
    window->actionCollection()->addAction(menu->objectName(), menu);

    // ensure that the KGameDifficulty instance gets deleted
    if (!useSingleton && !difficulty->parent()) {
        difficulty->setParent(window);
    }
}

// END KGameDifficultyGUI

#include "kgamedifficulty.moc"
#include "moc_kgamedifficulty.cpp"
