/*
    This file is part of the KDE games library
    Copyright (C) 2001 Andreas Beckermann (b_mann@gmx.de)

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

#include "kstdgameaction.h"

#include <klocale.h>
#include <kaction.h>
#include <kstandardshortcut.h>
#include <ktoggleaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kicon.h>


KStdGameAction::KStdGameAction()
{}

KStdGameAction::~KStdGameAction()
{}

KAction *KStdGameAction::action(StdGameAction act_enum, const QObject *recvr,
                                const char *slot, KActionCollection *parent,
                                const char *name)
{
    return create( act_enum, name, recvr, slot, parent );
}

const char* KStdGameAction::stdName(StdGameAction act_enum)
{
    return name(act_enum);
}

struct KStdGameActionInfo
{
	KStdGameAction::StdGameAction id;
	KStandardShortcut::StandardShortcut globalAccel; // if we reuse a global accel
    int shortcut; // specific shortcut (NH: should be configurable)
	const char* psName;
	const char* psLabel;
	const char* psWhatsThis;
	const char* psIconName;
};

const KStdGameActionInfo g_rgActionInfo[] = {
// "game" menu
    { KStdGameAction::New, KStandardShortcut::New, 0, "game_new", I18N_NOOP2("new game", "&New"), 0, "filenew" },
    { KStdGameAction::Load, KStandardShortcut::Open, 0, "game_load", I18N_NOOP("&Load..."), 0, "fileopen" },
    { KStdGameAction::LoadRecent, KStandardShortcut::AccelNone, 0, "game_load_recent", I18N_NOOP("Load &Recent"), 0, 0 },
    { KStdGameAction::Restart, KStandardShortcut::Reload, 0, "game_restart", I18N_NOOP("Restart &Game"), 0, "reload" },
    { KStdGameAction::Save, KStandardShortcut::Save, 0, "game_save", I18N_NOOP("&Save"), 0, "filesave" },
    { KStdGameAction::SaveAs, KStandardShortcut::AccelNone, 0, "game_save_as", I18N_NOOP("Save &As..."), 0, "filesaveas" },
    { KStdGameAction::End, KStandardShortcut::End, 0, "game_end", I18N_NOOP("&End Game"), 0, "fileclose" },
    { KStdGameAction::Pause, KStandardShortcut::AccelNone, Qt::Key_P, "game_pause", I18N_NOOP("Pa&use"), 0, "player_pause" },
    { KStdGameAction::Highscores, KStandardShortcut::AccelNone, Qt::CTRL+Qt::Key_H, "game_highscores", I18N_NOOP("Show &Highscores"), 0, "highscore" },
    { KStdGameAction::Print, KStandardShortcut::Print, 0, "game_print", I18N_NOOP("&Print..."), 0, "fileprint" },
    { KStdGameAction::Quit, KStandardShortcut::Quit, 0, "game_quit", I18N_NOOP("&Quit"), 0, "exit" },
// "move" menu
    { KStdGameAction::Repeat, KStandardShortcut::AccelNone, 0, "move_repeat", I18N_NOOP("Repeat"), 0, 0 },
    { KStdGameAction::Undo, KStandardShortcut::Undo, 0, "move_undo", I18N_NOOP("Und&o"), 0, "undo" },
    { KStdGameAction::Redo, KStandardShortcut::Redo, 0, "move_redo", I18N_NOOP("Re&do"), 0, "redo" },
    { KStdGameAction::Roll, KStandardShortcut::AccelNone, Qt::CTRL+Qt::Key_R, "move_roll", I18N_NOOP("&Roll Dice"), 0, "roll" },
    { KStdGameAction::EndTurn, KStandardShortcut::AccelNone, 0, "move_end_turn", I18N_NOOP("End Turn"), 0, "endturn" },
    { KStdGameAction::Hint, KStandardShortcut::AccelNone, Qt::Key_H, "move_hint", I18N_NOOP("&Hint"), 0, "idea" },
    { KStdGameAction::Demo, KStandardShortcut::AccelNone, Qt::Key_D, "move_demo", I18N_NOOP("&Demo"), 0, "1rightarrow" },
    { KStdGameAction::Solve, KStandardShortcut::AccelNone, 0, "move_solve", I18N_NOOP("&Solve"), 0, "wizard" },
// "settings" menu
    { KStdGameAction::ChooseGameType, KStandardShortcut::AccelNone, 0, "options_choose_game_type", I18N_NOOP("Choose Game &Type"), 0, 0 },
    { KStdGameAction::Carddecks, KStandardShortcut::AccelNone, 0, "options_configure_carddecks", I18N_NOOP("Configure &Carddecks..."), 0, 0 },
    { KStdGameAction::ConfigureHighscores, KStandardShortcut::AccelNone, 0, "options_configure_highscores", I18N_NOOP("Configure &Highscores..."), 0, 0 },

    { KStdGameAction::ActionNone, KStandardShortcut::AccelNone, 0, 0, 0, 0, 0 }
};

static const KStdGameActionInfo* infoPtr( KStdGameAction::StdGameAction id )
{
	for (uint i = 0; g_rgActionInfo[i].id!=KStdGameAction::ActionNone; i++) {
		if( g_rgActionInfo[i].id == id )
			return &g_rgActionInfo[i];
	}
	return 0;
}


KAction* KStdGameAction::create(StdGameAction id, const char *name,
                                const QObject *recvr, const char *slot,
                                KActionCollection* parent )
{
	KAction* pAction = 0;
	const KStdGameActionInfo* pInfo = infoPtr( id );
	kDebug(125) << "KStdGameAction::create( " << id << "=" << (pInfo ? pInfo->psName : (const char*)0) << ", " << parent << ", " << name << " )" << endl;
	if( pInfo ) {
		QString sLabel = i18n(pInfo->psLabel);
		KShortcut cut = (pInfo->globalAccel==KStandardShortcut::AccelNone
                         ? KShortcut(pInfo->shortcut)
                         : KStandardShortcut::shortcut(pInfo->globalAccel));
        const char *n = name ? name : pInfo->psName;
        bool do_connect = (recvr && slot); //both not 0
        switch( id ) {
        case LoadRecent:
            pAction = new KRecentFilesAction(sLabel, parent, n);
            pAction->setShortcut( cut );
            if(do_connect)
                QObject::connect( pAction, SIGNAL(urlSelected(const KUrl&)), recvr, slot);
            break;
        case Pause:
        case Demo:
            pAction = new KToggleAction(KIcon(pInfo->psIconName), sLabel, parent, n);
            pAction->setShortcut(cut);
            if(do_connect)
                QObject::connect(pAction, SIGNAL(triggered(bool) ), recvr, slot);
            break;
        case ChooseGameType:
            pAction = new KSelectAction( KIcon(pInfo->psIconName), sLabel, parent, n);
            pAction->setShortcut( cut );
            if(do_connect)
                QObject::connect( pAction, SIGNAL( triggered(int) ), recvr, slot );
            break;
        default:
            pAction = new KAction(KIcon(pInfo->psIconName),  sLabel, parent, n);
            pAction->setShortcut(cut);
            if(do_connect)
                QObject::connect(pAction, SIGNAL(triggered(bool) ), recvr, slot);
            break;
		}
	}
	return pAction;
}

const char* KStdGameAction::name( StdGameAction id )
{
	const KStdGameActionInfo* pInfo = infoPtr( id );
	return (pInfo) ? pInfo->psName : 0;
}

KAction *KStdGameAction::gameNew(const QObject *recvr, const char *slot,
                             KActionCollection *parent, const char *name )
{ return KStdGameAction::create(New, name, recvr, slot, parent); }
KAction *KStdGameAction::load(const QObject *recvr, const char *slot,
                              KActionCollection *parent, const char *name )
{ return KStdGameAction::create(Load, name, recvr, slot, parent); }
KRecentFilesAction *KStdGameAction::loadRecent(const QObject *recvr, const char *slot,
                                               KActionCollection *parent, const char *name )
{ return static_cast<KRecentFilesAction *>(KStdGameAction::create(LoadRecent, name, recvr, slot, parent)); }
KAction *KStdGameAction::save(const QObject *recvr, const char *slot,
                              KActionCollection *parent, const char *name )
{ return KStdGameAction::create(Save, name, recvr, slot, parent); }
KAction *KStdGameAction::saveAs(const QObject *recvr, const char *slot,
                                KActionCollection *parent, const char *name )
{ return KStdGameAction::create(SaveAs, name, recvr, slot, parent); }
KAction *KStdGameAction::end(const QObject *recvr, const char *slot,
                             KActionCollection *parent, const char *name )
{ return KStdGameAction::create(End, name, recvr, slot, parent); }
KToggleAction *KStdGameAction::pause(const QObject *recvr, const char *slot,
                                     KActionCollection *parent, const char *name )
{ return static_cast<KToggleAction *>(KStdGameAction::create(Pause, name, recvr, slot, parent)); }
KAction *KStdGameAction::highscores(const QObject *recvr, const char *slot,
                                    KActionCollection *parent, const char *name )
{ return KStdGameAction::create(Highscores, name, recvr, slot, parent); }
KAction *KStdGameAction::print(const QObject *recvr, const char *slot,
                               KActionCollection *parent, const char *name )
{ return KStdGameAction::create(Print, name, recvr, slot, parent); }
KAction *KStdGameAction::quit(const QObject *recvr, const char *slot,
                              KActionCollection *parent, const char *name )
{ return KStdGameAction::create(Quit, name, recvr, slot, parent); }

KAction *KStdGameAction::repeat(const QObject *recvr, const char *slot,
                                KActionCollection *parent, const char *name )
{ return KStdGameAction::create(Repeat, name, recvr, slot, parent); }
KAction *KStdGameAction::undo(const QObject *recvr, const char *slot,
                              KActionCollection *parent, const char *name )
{ return KStdGameAction::create(Undo, name, recvr, slot, parent); }

KAction *KStdGameAction::redo(const QObject *recvr, const char *slot,
                              KActionCollection *parent, const char *name )
{ return KStdGameAction::create(Redo, name, recvr, slot, parent); }

KAction *KStdGameAction::roll(const QObject *recvr, const char *slot,
                              KActionCollection *parent, const char *name )
{ return KStdGameAction::create(Roll, name, recvr, slot, parent); }
KAction *KStdGameAction::endTurn(const QObject *recvr, const char *slot,
                                 KActionCollection *parent, const char *name )
{ return KStdGameAction::create(EndTurn, name, recvr, slot, parent); }

KAction *KStdGameAction::carddecks(const QObject *recvr, const char *slot,
                                   KActionCollection *parent, const char *name )
{ return KStdGameAction::create(Carddecks, name, recvr, slot, parent); }
KAction *KStdGameAction::configureHighscores(const QObject*recvr, const char *slot,
                                             KActionCollection *parent, const char *name)
{ return KStdGameAction::create(ConfigureHighscores, name, recvr, slot, parent); }
KAction *KStdGameAction::hint(const QObject*recvr, const char *slot,
                              KActionCollection *parent, const char *name)
{ return KStdGameAction::create(Hint, name, recvr, slot, parent); }
KToggleAction *KStdGameAction::demo(const QObject*recvr, const char *slot,
                               KActionCollection *parent, const char *name)
{ return static_cast<KToggleAction *>(KStdGameAction::create(Demo, name, recvr, slot, parent)); }
KAction *KStdGameAction::solve(const QObject*recvr, const char *slot,
                               KActionCollection *parent, const char *name)
{ return KStdGameAction::create(Solve, name, recvr, slot, parent); }
KSelectAction *KStdGameAction::chooseGameType(const QObject*recvr, const char *slot,
                                          KActionCollection *parent, const char *name)
{ return static_cast<KSelectAction *>(KStdGameAction::create(ChooseGameType, name, recvr, slot, parent)); }
KAction *KStdGameAction::restart(const QObject*recvr, const char *slot,
                                 KActionCollection *parent, const char *name)
{ return KStdGameAction::create(Restart, name, recvr, slot, parent); }
