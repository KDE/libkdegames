/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>
    SPDX-FileCopyrightText: 2007 Simon Hürlimann <simon.huerlimann@huerlisi.ch>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kstandardgameaction.h"

// KF
#include <KActionCollection>
#include <KLazyLocalizedString>
#include <KLocalizedString>
#include <KStandardShortcut>
// Qt
#include <QIcon>

Q_LOGGING_CATEGORY(GAMES_UI, "org.kde.games.ui", QtWarningMsg)

struct KStandardGameActionInfo {
    KStandardGameAction::StandardGameAction id;
    KStandardShortcut::StandardShortcut globalAccel; // if we reuse a global accel
    int shortcut; // specific shortcut (NH: should be configurable)
    const char *psName;
    const KLazyLocalizedString psLabel;
    const KLazyLocalizedString psWhatsThis;
    const char *psIconName;
    const KLazyLocalizedString psToolTip;
};

const KStandardGameActionInfo g_rgActionInfo[] = {
    // clang-format off
    // "game" menu
    { KStandardGameAction::New,             KStandardShortcut::New,       0,                    "game_new",
      kli18nc("new game", "&New"),  kli18n("Start a new game."), "document-new",        kli18n("Start a new game") },
    { KStandardGameAction::Load,            KStandardShortcut::Open,      0,                    "game_load",
      kli18n("&Load..."),           {},                          "document-open",       kli18n("Open a saved game...") },
    { KStandardGameAction::LoadRecent,      KStandardShortcut::AccelNone, 0,                    "game_load_recent",
      kli18n("Load &Recent"),       {},                         nullptr,                kli18n("Open a recently saved game...") },
    { KStandardGameAction::Restart,         KStandardShortcut::Reload,    0,                    "game_restart",
      kli18n("Restart &Game"),      {},                         "view-refresh",         kli18n("Restart the game") },
    { KStandardGameAction::Save,            KStandardShortcut::Save,      0,                    "game_save",
      kli18n("&Save"),              {},                         "document-save",        kli18n("Save the current game") },
    { KStandardGameAction::SaveAs,          KStandardShortcut::AccelNone, 0,                    "game_save_as",
      kli18n("Save &As..."),        {},                         "document-save-as",     kli18n("Save the current game to another file") },
    { KStandardGameAction::End,             KStandardShortcut::End,       0,                    "game_end",
      kli18n("&End Game"),          {},                         "window-close",         kli18n("End the current game") },
    { KStandardGameAction::Pause,           KStandardShortcut::AccelNone, Qt::Key_P,            "game_pause",
      kli18n("Pa&use"),             {},                         "media-playback-pause", kli18n("Pause the game") },
    { KStandardGameAction::Highscores,      KStandardShortcut::AccelNone, Qt::CTRL | Qt::Key_H, "game_highscores",
      kli18n("Show &High Scores"),  {},                         "games-highscores",     kli18n("Show high scores") },
    { KStandardGameAction::ClearHighscores, KStandardShortcut::AccelNone, 0,                    "game_clear_highscores",
      kli18n("&Clear High Scores"), {},                         "clear_highscore",      kli18n("Clear high scores") },
    { KStandardGameAction::Statistics,      KStandardShortcut::AccelNone, 0,                    "game_statistics",
      kli18n("Show Statistics"),    {},                         "highscore",            kli18n("Show statistics") },
    { KStandardGameAction::ClearStatistics, KStandardShortcut::AccelNone, 0,                    "game_clear_statistics",
      kli18n("&Clear Statistics"),  {},                         "flag",                 kli18n("Delete all-time statistics.") },
    { KStandardGameAction::Print,           KStandardShortcut::Print,     0,                    "game_print",
      kli18n("&Print..."),          {},                         "document-print",       {} },
    { KStandardGameAction::Quit,            KStandardShortcut::Quit,      0,                    "game_quit",
      kli18n("&Quit"),              {},                         "application-exit",     kli18n("Quit the program") },

    // "move" menu
    { KStandardGameAction::Repeat,  KStandardShortcut::AccelNone, 0,                    "move_repeat",
      kli18n("Repeat"),     {}, nullptr,                kli18n("Repeat the last move") },
    { KStandardGameAction::Undo,    KStandardShortcut::Undo,      0,                    "move_undo",
      kli18n("Und&o"),      {}, "edit-undo",            kli18n("Undo the last move") },
    { KStandardGameAction::Redo,    KStandardShortcut::Redo,      0,                    "move_redo",
      kli18n("Re&do"),      {}, "edit-redo",            kli18n("Redo the latest move") },
    { KStandardGameAction::Roll,    KStandardShortcut::AccelNone, Qt::CTRL | Qt::Key_R, "move_roll",
      kli18n("&Roll Dice"), {}, "roll",                 kli18n("Roll the dice") },
    { KStandardGameAction::EndTurn, KStandardShortcut::AccelNone, 0,                    "move_end_turn",
      kli18n("End Turn"),   {}, "games-endturn",        {}  },
    { KStandardGameAction::Hint,    KStandardShortcut::AccelNone, Qt::Key_H,            "move_hint",
      kli18n("&Hint"),      {}, "games-hint",           kli18n("Give a hint") },
    { KStandardGameAction::Demo,    KStandardShortcut::AccelNone, Qt::Key_D,            "move_demo",
      kli18n("&Demo"),      {}, "media-playback-start", kli18n("Play a demo") },
    { KStandardGameAction::Solve,   KStandardShortcut::AccelNone, 0,                    "move_solve",
      kli18n("&Solve"),     {}, "games-solve",          kli18n("Solve the game") },

    // "settings" menu
    { KStandardGameAction::ChooseGameType,      KStandardShortcut::AccelNone, 0, "options_choose_game_type",
      kli18n("Choose Game &Type"),         {}, nullptr, {} },
    { KStandardGameAction::Carddecks,           KStandardShortcut::AccelNone, 0, "options_configure_carddecks",
      kli18n("Configure &Carddecks..."),   {}, nullptr, {} },
    { KStandardGameAction::ConfigureHighscores, KStandardShortcut::AccelNone, 0, "options_configure_highscores",
      kli18n("Configure &High Scores..."), {}, nullptr, {} },

    { KStandardGameAction::ActionNone, KStandardShortcut::AccelNone, 0, nullptr, {}, {}, nullptr, {} }
    // clang-format on
};

static const KStandardGameActionInfo *infoPtr(KStandardGameAction::StandardGameAction id)
{
    for (uint i = 0; g_rgActionInfo[i].id != KStandardGameAction::ActionNone; i++) {
        if (g_rgActionInfo[i].id == id)
            return &g_rgActionInfo[i];
    }
    return nullptr;
}

QAction *KStandardGameAction::_k_createInternal(KStandardGameAction::StandardGameAction id, QObject *parent)
{
    QAction *pAction = nullptr;
    const KStandardGameActionInfo *pInfo = infoPtr(id);
    // qCDebug(GAMES_UI) << "KStandardGameAction::create( " << id << "=" << (pInfo ? pInfo->psName : (const char*)nullptr) << "," << parent << " )";
    if (pInfo) {
        const QString sLabel = pInfo->psLabel.toString();
        switch (id) {
        case LoadRecent:
            pAction = new KRecentFilesAction(sLabel, parent);
            break;
        case Pause:
        case Demo:
            pAction = new KToggleAction(QIcon::fromTheme(QString::fromLatin1(pInfo->psIconName)), sLabel, parent);
            break;
        case ChooseGameType:
            pAction = new KSelectAction(QIcon::fromTheme(QString::fromLatin1(pInfo->psIconName)), sLabel, parent);
            break;
        default:
            pAction = new QAction(QIcon::fromTheme(QString::fromLatin1(pInfo->psIconName)), sLabel, parent);
            break;
        }

        // clang-format off
        const QList<QKeySequence> cut =
            (pInfo->globalAccel != KStandardShortcut::AccelNone) ? KStandardShortcut::shortcut(pInfo->globalAccel) :
            (pInfo->shortcut != 0)                               ? QList<QKeySequence>{QKeySequence(pInfo->shortcut)} :
            /* else */ QList<QKeySequence>();
        // clang-format on

        if (!cut.isEmpty()) {
            pAction->setShortcuts(cut);
            pAction->setProperty("defaultShortcuts", QVariant::fromValue(cut));
        }
        if (!pInfo->psToolTip.isEmpty())
            pAction->setToolTip(pInfo->psToolTip.toString());
        if (!pInfo->psWhatsThis.isEmpty())
            pAction->setWhatsThis(pInfo->psWhatsThis.toString());
        else if (!pInfo->psToolTip.isEmpty())
            pAction->setWhatsThis(pInfo->psToolTip.toString());

        pAction->setObjectName(QLatin1String(pInfo->psName));
    }

    KActionCollection *collection = qobject_cast<KActionCollection *>(parent);
    if (collection && pAction)
        collection->addAction(pAction->objectName(), pAction);

    return pAction;
}

QAction *KStandardGameAction::create(StandardGameAction id, const QObject *recvr, const char *slot, QObject *parent)
{
    QAction *pAction = _k_createInternal(id, parent);
    if (recvr && slot) {
        switch (id) {
        case LoadRecent:
            QObject::connect(pAction, SIGNAL(urlSelected(QUrl)), recvr, slot);
            break;
        case ChooseGameType:
            QObject::connect(pAction, SIGNAL(indexTriggered(int)), recvr, slot);
            break;
        default:
            QObject::connect(pAction, SIGNAL(triggered(bool)), recvr, slot);
            break;
        }
    }
    return pAction;
}

const char *KStandardGameAction::name(StandardGameAction id)
{
    const KStandardGameActionInfo *pInfo = infoPtr(id);
    return (pInfo) ? pInfo->psName : nullptr;
}

QAction *KStandardGameAction::gameNew(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(New, recvr, slot, parent);
}
QAction *KStandardGameAction::load(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(Load, recvr, slot, parent);
}
KRecentFilesAction *KStandardGameAction::loadRecent(const QObject *recvr, const char *slot, QObject *parent)
{
    return static_cast<KRecentFilesAction *>(KStandardGameAction::create(LoadRecent, recvr, slot, parent));
}
QAction *KStandardGameAction::save(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(Save, recvr, slot, parent);
}
QAction *KStandardGameAction::saveAs(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(SaveAs, recvr, slot, parent);
}
QAction *KStandardGameAction::end(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(End, recvr, slot, parent);
}
KToggleAction *KStandardGameAction::pause(const QObject *recvr, const char *slot, QObject *parent)
{
    return static_cast<KToggleAction *>(KStandardGameAction::create(Pause, recvr, slot, parent));
}
QAction *KStandardGameAction::highscores(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(Highscores, recvr, slot, parent);
}
QAction *KStandardGameAction::statistics(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(Highscores, recvr, slot, parent);
}
QAction *KStandardGameAction::clearStatistics(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(ClearStatistics, recvr, slot, parent);
}
QAction *KStandardGameAction::print(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(Print, recvr, slot, parent);
}
QAction *KStandardGameAction::quit(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(Quit, recvr, slot, parent);
}

QAction *KStandardGameAction::repeat(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(Repeat, recvr, slot, parent);
}
QAction *KStandardGameAction::undo(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(Undo, recvr, slot, parent);
}

QAction *KStandardGameAction::redo(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(Redo, recvr, slot, parent);
}

QAction *KStandardGameAction::roll(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(Roll, recvr, slot, parent);
}
QAction *KStandardGameAction::endTurn(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(EndTurn, recvr, slot, parent);
}

QAction *KStandardGameAction::carddecks(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(Carddecks, recvr, slot, parent);
}
QAction *KStandardGameAction::configureHighscores(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(ConfigureHighscores, recvr, slot, parent);
}
QAction *KStandardGameAction::hint(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(Hint, recvr, slot, parent);
}
KToggleAction *KStandardGameAction::demo(const QObject *recvr, const char *slot, QObject *parent)
{
    return static_cast<KToggleAction *>(KStandardGameAction::create(Demo, recvr, slot, parent));
}
QAction *KStandardGameAction::solve(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(Solve, recvr, slot, parent);
}
KSelectAction *KStandardGameAction::chooseGameType(const QObject *recvr, const char *slot, QObject *parent)
{
    return static_cast<KSelectAction *>(KStandardGameAction::create(ChooseGameType, recvr, slot, parent));
}
QAction *KStandardGameAction::restart(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardGameAction::create(Restart, recvr, slot, parent);
}
