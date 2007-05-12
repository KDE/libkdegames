/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann <b_mann@gmx.de>
    Copyright (C) 2007 Simon Hürlimann <simon.huerlimann@huerlisi.ch>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kstandardgameaction.h"

#include <klocale.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardshortcut.h>
#include <ktoggleaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kicon.h>
#include <krecentfilesaction.h>


struct KStandardGameActionInfo
{
    KStandardGameAction::StandardGameAction id;
    KStandardShortcut::StandardShortcut globalAccel; // if we reuse a global accel
    int shortcut; // specific shortcut (NH: should be configurable)
    const char* psName;
    const char* psLabel;
    const char* psWhatsThis;
    const char* psIconName;
    const char* psToolTip;
};

const KStandardGameActionInfo g_rgActionInfo[] = {
// "game" menu
    { KStandardGameAction::New, KStandardShortcut::New, 0, "game_new", I18N_NOOP2("new game", "&New"), 0, "document-new", "Start a new game" },
    { KStandardGameAction::Load, KStandardShortcut::Open, 0, "game_load", I18N_NOOP("&Load..."), 0, "document-open", "Open a saved game..." },
    { KStandardGameAction::LoadRecent, KStandardShortcut::AccelNone, 0, "game_load_recent", I18N_NOOP("Load &Recent"), 0, 0, "Open a recently saved game..." },
    { KStandardGameAction::Restart, KStandardShortcut::Reload, 0, "game_restart", I18N_NOOP("Restart &Game"), 0, "view-refresh", "Restart the game" },
    { KStandardGameAction::Save, KStandardShortcut::Save, 0, "game_save", I18N_NOOP("&Save"), 0, "document-save", "Save the current game" },
    { KStandardGameAction::SaveAs, KStandardShortcut::AccelNone, 0, "game_save_as", I18N_NOOP("Save &As..."), 0, "document-save-as", "Save the current game to another file" },
    { KStandardGameAction::End, KStandardShortcut::End, 0, "game_end", I18N_NOOP("&End Game"), 0, "window-close", "End the current game" },
    { KStandardGameAction::Pause, KStandardShortcut::AccelNone, Qt::Key_P, "game_pause", I18N_NOOP("Pa&use"), 0, "media-playback-pause", "Pause the game" },
    { KStandardGameAction::Highscores, KStandardShortcut::AccelNone, Qt::CTRL+Qt::Key_H, "game_highscores", I18N_NOOP("Show &Highscores"), 0, "highscore", "Show highscores" },
    { KStandardGameAction::Print, KStandardShortcut::Print, 0, "game_print", I18N_NOOP("&Print..."), 0, "document-print", 0 },
    { KStandardGameAction::Quit, KStandardShortcut::Quit, 0, "game_quit", I18N_NOOP("&Quit"), 0, "application-exit", "Quit the program" },
// "move" menu
    { KStandardGameAction::Repeat, KStandardShortcut::AccelNone, 0, "move_repeat", I18N_NOOP("Repeat"), 0, 0, "Repeat the last move" },
    { KStandardGameAction::Undo, KStandardShortcut::Undo, 0, "move_undo", I18N_NOOP("Und&o"), 0, "edit-undo", "Undo the last move" },
    { KStandardGameAction::Redo, KStandardShortcut::Redo, 0, "move_redo", I18N_NOOP("Re&do"), 0, "edit-redo", "Redo the latest move" },
    { KStandardGameAction::Roll, KStandardShortcut::AccelNone, Qt::CTRL+Qt::Key_R, "move_roll", I18N_NOOP("&Roll Dice"), 0, "roll", "Roll the dice" },
    { KStandardGameAction::EndTurn, KStandardShortcut::AccelNone, 0, "move_end_turn", I18N_NOOP("End Turn"), 0, "endturn", 0  },
    { KStandardGameAction::Hint, KStandardShortcut::AccelNone, Qt::Key_H, "move_hint", I18N_NOOP("&Hint"), 0, "idea", "Give a hint" },
    { KStandardGameAction::Demo, KStandardShortcut::AccelNone, Qt::Key_D, "move_demo", I18N_NOOP("&Demo"), 0, "media-playback-start", "Play a demo" },
    { KStandardGameAction::Solve, KStandardShortcut::AccelNone, 0, "move_solve", I18N_NOOP("&Solve"), 0, "wizard", "Solve the game" },
// "settings" menu
    { KStandardGameAction::ChooseGameType, KStandardShortcut::AccelNone, 0, "options_choose_game_type", I18N_NOOP("Choose Game &Type"), 0, 0, 0 },
    { KStandardGameAction::Carddecks, KStandardShortcut::AccelNone, 0, "options_configure_carddecks", I18N_NOOP("Configure &Carddecks..."), 0, 0, 0  },
    { KStandardGameAction::ConfigureHighscores, KStandardShortcut::AccelNone, 0, "options_configure_highscores", I18N_NOOP("Configure &Highscores..."), 0, 0, 0  },

    { KStandardGameAction::ActionNone, KStandardShortcut::AccelNone, 0, 0, 0, 0, 0, 0  }
};

static const KStandardGameActionInfo* infoPtr( KStandardGameAction::StandardGameAction id )
{
    for (uint i = 0; g_rgActionInfo[i].id!=KStandardGameAction::ActionNone; i++) {
      if( g_rgActionInfo[i].id == id )
        return &g_rgActionInfo[i];
    }
    return 0;
}


KAction* KStandardGameAction::create(StandardGameAction id, const QObject *recvr, const char *slot,
                                     QObject* parent )
{
    KAction* pAction = 0;
    const KStandardGameActionInfo* pInfo = infoPtr( id );
    kDebug(125) << "KStandardGameAction::create( " << id << "=" << (pInfo ? pInfo->psName : (const char*)0) << ", " << parent << " )" << endl;
    if( pInfo ) {
        QString sLabel = i18n(pInfo->psLabel);
        KShortcut cut = (pInfo->globalAccel==KStandardShortcut::AccelNone
                         ? KShortcut(pInfo->shortcut)
                         : KStandardShortcut::shortcut(pInfo->globalAccel));
        bool do_connect = (recvr && slot); //both not 0
        switch( id ) {
        case LoadRecent:
            pAction = new KRecentFilesAction(sLabel, parent);
            pAction->setShortcut( cut );
            if (pInfo->psToolTip)
              pAction->setToolTip(i18n(pInfo->psToolTip));
            if(do_connect)
                QObject::connect( pAction, SIGNAL(urlSelected(const KUrl&)), recvr, slot);
            break;
        case Pause:
        case Demo:
            pAction = new KToggleAction(KIcon(pInfo->psIconName), sLabel, parent);
            pAction->setShortcut(cut);
            if (pInfo->psToolTip)
              pAction->setToolTip(i18n(pInfo->psToolTip));
            if(do_connect)
                QObject::connect(pAction, SIGNAL(triggered(bool) ), recvr, slot);
            break;
        case ChooseGameType:
            pAction = new KSelectAction( KIcon(pInfo->psIconName), sLabel, parent);
            pAction->setShortcut( cut );
            if (pInfo->psToolTip)
              pAction->setToolTip(i18n(pInfo->psToolTip));
            if(do_connect)
                QObject::connect( pAction, SIGNAL( triggered(int) ), recvr, slot );
            break;
        default:
            pAction = new KAction(KIcon(pInfo->psIconName),  sLabel, parent);
            pAction->setShortcut(cut);
            if (pInfo->psToolTip)
              pAction->setToolTip(i18n(pInfo->psToolTip));
            if(do_connect)
                QObject::connect(pAction, SIGNAL(triggered(bool) ), recvr, slot);
            break;
        }

        pAction->setObjectName(pInfo->psName);
    }

    KActionCollection *collection = qobject_cast<KActionCollection *>(parent);
    if (collection && pAction)
        collection->addAction(pAction->objectName(), pAction);

    return pAction;
}

const char* KStandardGameAction::name( StandardGameAction id )
{
    const KStandardGameActionInfo* pInfo = infoPtr( id );
    return (pInfo) ? pInfo->psName : 0;
}

KAction *KStandardGameAction::gameNew(const QObject *recvr, const char *slot,
                                      QObject *parent)
{ return KStandardGameAction::create(New, recvr, slot, parent); }
KAction *KStandardGameAction::load(const QObject *recvr, const char *slot,
                                   QObject *parent)
{ return KStandardGameAction::create(Load, recvr, slot, parent); }
KRecentFilesAction *KStandardGameAction::loadRecent(const QObject *recvr, const char *slot,
                                                    QObject *parent)
{ return static_cast<KRecentFilesAction *>(KStandardGameAction::create(LoadRecent, recvr, slot, parent)); }
KAction *KStandardGameAction::save(const QObject *recvr, const char *slot,
                                   QObject *parent)
{ return KStandardGameAction::create(Save, recvr, slot, parent); }
KAction *KStandardGameAction::saveAs(const QObject *recvr, const char *slot,
                                     QObject *parent)
{ return KStandardGameAction::create(SaveAs, recvr, slot, parent); }
KAction *KStandardGameAction::end(const QObject *recvr, const char *slot,
                                  QObject *parent)
{ return KStandardGameAction::create(End, recvr, slot, parent); }
KToggleAction *KStandardGameAction::pause(const QObject *recvr, const char *slot,
                                          QObject *parent)
{ return static_cast<KToggleAction *>(KStandardGameAction::create(Pause, recvr, slot, parent)); }
KAction *KStandardGameAction::highscores(const QObject *recvr, const char *slot,
                                         QObject *parent)
{ return KStandardGameAction::create(Highscores, recvr, slot, parent); }
KAction *KStandardGameAction::print(const QObject *recvr, const char *slot,
                                    QObject *parent)
{ return KStandardGameAction::create(Print, recvr, slot, parent); }
KAction *KStandardGameAction::quit(const QObject *recvr, const char *slot,
                                   QObject *parent)
{ return KStandardGameAction::create(Quit, recvr, slot, parent); }

KAction *KStandardGameAction::repeat(const QObject *recvr, const char *slot,
                                     QObject *parent)
{ return KStandardGameAction::create(Repeat, recvr, slot, parent); }
KAction *KStandardGameAction::undo(const QObject *recvr, const char *slot,
                                   QObject *parent)
{ return KStandardGameAction::create(Undo, recvr, slot, parent); }

KAction *KStandardGameAction::redo(const QObject *recvr, const char *slot,
                                   QObject *parent)
{ return KStandardGameAction::create(Redo, recvr, slot, parent); }

KAction *KStandardGameAction::roll(const QObject *recvr, const char *slot,
                                   QObject *parent)
{ return KStandardGameAction::create(Roll, recvr, slot, parent); }
KAction *KStandardGameAction::endTurn(const QObject *recvr, const char *slot,
                                      QObject *parent)
{ return KStandardGameAction::create(EndTurn, recvr, slot, parent); }

KAction *KStandardGameAction::carddecks(const QObject *recvr, const char *slot,
                                        QObject *parent)
{ return KStandardGameAction::create(Carddecks, recvr, slot, parent); }
KAction *KStandardGameAction::configureHighscores(const QObject*recvr, const char *slot,
                                                  QObject *parent)
{ return KStandardGameAction::create(ConfigureHighscores, recvr, slot, parent); }
KAction *KStandardGameAction::hint(const QObject*recvr, const char *slot,
                                   QObject *parent)
{ return KStandardGameAction::create(Hint, recvr, slot, parent); }
KToggleAction *KStandardGameAction::demo(const QObject*recvr, const char *slot,
                                         QObject *parent)
{ return static_cast<KToggleAction *>(KStandardGameAction::create(Demo, recvr, slot, parent)); }
KAction *KStandardGameAction::solve(const QObject*recvr, const char *slot,
                                    QObject *parent)
{ return KStandardGameAction::create(Solve, recvr, slot, parent); }
KSelectAction *KStandardGameAction::chooseGameType(const QObject*recvr, const char *slot,
                                                   QObject *parent)
{ return static_cast<KSelectAction *>(KStandardGameAction::create(ChooseGameType, recvr, slot, parent)); }
KAction *KStandardGameAction::restart(const QObject*recvr, const char *slot,
                                      QObject *parent)
{ return KStandardGameAction::create(Restart, recvr, slot, parent); }
