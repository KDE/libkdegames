/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>
    SPDX-FileCopyrightText: 2007 Simon Hürlimann <simon.huerlimann@huerlisi.ch>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgamestandardaction.h"

// KF
#include <KActionCollection>
#include <KLazyLocalizedString>
#include <KLocalizedString>
#include <KStandardShortcut>
// Qt
#include <QIcon>

struct KGameStandardActionInfo {
    KGameStandardAction::GameStandardAction id;
    KStandardShortcut::StandardShortcut globalAccel; // if we reuse a global accel
    int shortcut; // specific shortcut (NH: should be configurable)
    const char *psName;
    const KLazyLocalizedString psLabel;
    const KLazyLocalizedString psWhatsThis;
    const char *psIconName;
    const KLazyLocalizedString psToolTip;
};

#define CTRL(x) QKeyCombination(Qt::CTRL | Qt::Key_##x).toCombined()

const KGameStandardActionInfo g_rgActionInfo[] = {
    // clang-format off
    // "game" menu
    { KGameStandardAction::New,             KStandardShortcut::New,       0,                    "game_new",
      kli18nc("new game", "&New"),  kli18n("Start a new game."), "document-new",        kli18n("Start a new game") },
    { KGameStandardAction::Load,            KStandardShortcut::Open,      0,                    "game_load",
      kli18n("&Load..."),           {},                          "document-open",       kli18n("Open a saved game...") },
    { KGameStandardAction::LoadRecent,      KStandardShortcut::AccelNone, 0,                    "game_load_recent",
      kli18n("Load &Recent"),       {},                         nullptr,                kli18n("Open a recently saved game...") },
    { KGameStandardAction::Restart,         KStandardShortcut::Reload,    0,                    "game_restart",
      kli18n("Restart &Game"),      {},                         "view-refresh",         kli18n("Restart the game") },
    { KGameStandardAction::Save,            KStandardShortcut::Save,      0,                    "game_save",
      kli18n("&Save"),              {},                         "document-save",        kli18n("Save the current game") },
    { KGameStandardAction::SaveAs,          KStandardShortcut::AccelNone, 0,                    "game_save_as",
      kli18n("Save &As..."),        {},                         "document-save-as",     kli18n("Save the current game to another file") },
    { KGameStandardAction::End,             KStandardShortcut::End,       0,                    "game_end",
      kli18n("&End Game"),          {},                         "window-close",         kli18n("End the current game") },
    { KGameStandardAction::Pause,           KStandardShortcut::AccelNone, Qt::Key_P,            "game_pause",
      kli18n("Pa&use"),             {},                         "media-playback-pause", kli18n("Pause the game") },
    { KGameStandardAction::Highscores,      KStandardShortcut::AccelNone, CTRL(H),              "game_highscores",
      kli18n("Show &High Scores"),  {},                         "games-highscores",     kli18n("Show high scores") },
    { KGameStandardAction::ClearHighscores, KStandardShortcut::AccelNone, 0,                    "game_clear_highscores",
      kli18n("&Clear High Scores"), {},                         "clear_highscore",      kli18n("Clear high scores") },
    { KGameStandardAction::Statistics,      KStandardShortcut::AccelNone, 0,                    "game_statistics",
      kli18n("Show Statistics"),    {},                         "highscore",            kli18n("Show statistics") },
    { KGameStandardAction::ClearStatistics, KStandardShortcut::AccelNone, 0,                    "game_clear_statistics",
      kli18n("&Clear Statistics"),  {},                         "flag",                 kli18n("Delete all-time statistics.") },
    { KGameStandardAction::Print,           KStandardShortcut::Print,     0,                    "game_print",
      kli18n("&Print..."),          {},                         "document-print",       {} },
    { KGameStandardAction::Quit,            KStandardShortcut::Quit,      0,                    "game_quit",
      kli18n("&Quit"),              {},                         "application-exit",     kli18n("Quit the program") },

    // "move" menu
    { KGameStandardAction::Repeat,  KStandardShortcut::AccelNone, 0,                    "move_repeat",
      kli18n("Repeat"),     {}, nullptr,                kli18n("Repeat the last move") },
    { KGameStandardAction::Undo,    KStandardShortcut::Undo,      0,                    "move_undo",
      kli18n("Und&o"),      {}, "edit-undo",            kli18n("Undo the last move") },
    { KGameStandardAction::Redo,    KStandardShortcut::Redo,      0,                    "move_redo",
      kli18n("Re&do"),      {}, "edit-redo",            kli18n("Redo the latest move") },
    { KGameStandardAction::Roll,    KStandardShortcut::AccelNone, CTRL(R),              "move_roll",
      kli18n("&Roll Dice"), {}, "roll",                 kli18n("Roll the dice") },
    { KGameStandardAction::EndTurn, KStandardShortcut::AccelNone, 0,                    "move_end_turn",
      kli18n("End Turn"),   {}, "games-endturn",        {}  },
    { KGameStandardAction::Hint,    KStandardShortcut::AccelNone, Qt::Key_H,            "move_hint",
      kli18n("&Hint"),      {}, "games-hint",           kli18n("Give a hint") },
    { KGameStandardAction::Demo,    KStandardShortcut::AccelNone, Qt::Key_D,            "move_demo",
      kli18n("&Demo"),      {}, "media-playback-start", kli18n("Play a demo") },
    { KGameStandardAction::Solve,   KStandardShortcut::AccelNone, 0,                    "move_solve",
      kli18n("&Solve"),     {}, "games-solve",          kli18n("Solve the game") },

    // "settings" menu
    { KGameStandardAction::Carddecks,           KStandardShortcut::AccelNone, 0, "options_configure_carddecks",
      kli18n("Configure &Carddecks..."),   {}, nullptr, {} },
    { KGameStandardAction::ConfigureHighscores, KStandardShortcut::AccelNone, 0, "options_configure_highscores",
      kli18n("Configure &High Scores..."), {}, nullptr, {} },

    { KGameStandardAction::ActionNone, KStandardShortcut::AccelNone, 0, nullptr, {}, {}, nullptr, {} }
    // clang-format on
};

static const KGameStandardActionInfo *infoPtr(KGameStandardAction::GameStandardAction id)
{
    for (uint i = 0; g_rgActionInfo[i].id != KGameStandardAction::ActionNone; i++) {
        if (g_rgActionInfo[i].id == id)
            return &g_rgActionInfo[i];
    }
    return nullptr;
}

QAction *KGameStandardAction::_k_createInternal(KGameStandardAction::GameStandardAction id, QObject *parent)
{
    QAction *pAction = nullptr;
    const KGameStandardActionInfo *pInfo = infoPtr(id);
    // qCDebug(GAMES_UI) << "KGameStandardAction::create( " << id << "=" << (pInfo ? pInfo->psName : (const char*)nullptr) << "," << parent << " )";
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

QAction *KGameStandardAction::create(GameStandardAction id, const QObject *recvr, const char *slot, QObject *parent)
{
    QAction *pAction = _k_createInternal(id, parent);
    if (recvr && slot) {
        switch (id) {
        case LoadRecent:
            QObject::connect(pAction, SIGNAL(urlSelected(QUrl)), recvr, slot);
            break;
        default:
            QObject::connect(pAction, SIGNAL(triggered(bool)), recvr, slot);
            break;
        }
    }
    return pAction;
}

const char *KGameStandardAction::name(GameStandardAction id)
{
    const KGameStandardActionInfo *pInfo = infoPtr(id);
    return (pInfo) ? pInfo->psName : nullptr;
}

QAction *KGameStandardAction::gameNew(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(New, recvr, slot, parent);
}
QAction *KGameStandardAction::load(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(Load, recvr, slot, parent);
}
KRecentFilesAction *KGameStandardAction::loadRecent(const QObject *recvr, const char *slot, QObject *parent)
{
    return static_cast<KRecentFilesAction *>(KGameStandardAction::create(LoadRecent, recvr, slot, parent));
}
QAction *KGameStandardAction::save(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(Save, recvr, slot, parent);
}
QAction *KGameStandardAction::saveAs(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(SaveAs, recvr, slot, parent);
}
QAction *KGameStandardAction::end(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(End, recvr, slot, parent);
}
KToggleAction *KGameStandardAction::pause(const QObject *recvr, const char *slot, QObject *parent)
{
    return static_cast<KToggleAction *>(KGameStandardAction::create(Pause, recvr, slot, parent));
}
QAction *KGameStandardAction::highscores(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(Highscores, recvr, slot, parent);
}
QAction *KGameStandardAction::statistics(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(Highscores, recvr, slot, parent);
}
QAction *KGameStandardAction::clearStatistics(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(ClearStatistics, recvr, slot, parent);
}
QAction *KGameStandardAction::print(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(Print, recvr, slot, parent);
}
QAction *KGameStandardAction::quit(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(Quit, recvr, slot, parent);
}

QAction *KGameStandardAction::repeat(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(Repeat, recvr, slot, parent);
}
QAction *KGameStandardAction::undo(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(Undo, recvr, slot, parent);
}

QAction *KGameStandardAction::redo(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(Redo, recvr, slot, parent);
}

QAction *KGameStandardAction::roll(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(Roll, recvr, slot, parent);
}
QAction *KGameStandardAction::endTurn(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(EndTurn, recvr, slot, parent);
}

QAction *KGameStandardAction::carddecks(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(Carddecks, recvr, slot, parent);
}
QAction *KGameStandardAction::configureHighscores(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(ConfigureHighscores, recvr, slot, parent);
}
QAction *KGameStandardAction::hint(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(Hint, recvr, slot, parent);
}
KToggleAction *KGameStandardAction::demo(const QObject *recvr, const char *slot, QObject *parent)
{
    return static_cast<KToggleAction *>(KGameStandardAction::create(Demo, recvr, slot, parent));
}
QAction *KGameStandardAction::solve(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(Solve, recvr, slot, parent);
}
QAction *KGameStandardAction::restart(const QObject *recvr, const char *slot, QObject *parent)
{
    return KGameStandardAction::create(Restart, recvr, slot, parent);
}
