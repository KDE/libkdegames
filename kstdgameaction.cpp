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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kstdgameaction.h>
#include <klocale.h>
#include <kaction.h>
#include <kstdaccel.h>

KStdGameAction::KStdGameAction()
{
}

KStdGameAction::~KStdGameAction()
{
}

KAction *KStdGameAction::action(StdGameAction act_enum, const QObject *recvr,
                            const char *slot, KActionCollection *parent, const char *name )
{
    switch (act_enum)
    {
    case New:
        return gameNew(recvr, slot, parent, name);
    case Load:
        return load(recvr, slot, parent, name);
    case LoadRecent:
        return loadRecent(recvr, slot, parent, name);
    case Save:
        return save(recvr, slot, parent, name);
    case SaveAs:
        return saveAs(recvr, slot, parent, name);
    case End:
        return end(recvr, slot, parent, name);
    case Pause:
        return pause(recvr, slot, parent, name);
    case Highscores:
        return highscores(recvr, slot, parent, name);
    case Print:
        return print(recvr, slot, parent, name);
    case Quit:
        return quit(recvr, slot, parent, name);
    case Repeat:
        return repeat(recvr, slot, parent, name);
    case Undo:
        return undo(recvr, slot, parent, name);
    case Redo:
        return redo(recvr, slot, parent, name);
    case EndTurn:
        return endTurn(recvr, slot, parent, name);
    case Roll:
        return roll(recvr, slot, parent, name);
    case Carddecks:
        return carddecks(recvr, slot, parent, name);
    default:
        break;
    }
    return 0;
}

const char* KStdGameAction::stdName(StdGameAction act_enum)
{
    switch (act_enum)
    {
    case New:
        return "game_new";
    case Load:
        return "game_load";
    case LoadRecent:
        return "game_load_recent";
    case Save:
        return "game_save";
    case SaveAs:
        return "game_save_as";
    case End:
        return "game_end";
    case Pause:
        return "game_pause";
    case Highscores:
        return "game_highscores";
    case Print:
        return "game_print";
    case Quit:
        return "game_quit";
    case Repeat:
        return "move_repeat";
    case Undo:
        return "move_undo";
    case Redo:
        return "move_redo";
    case Roll:
        return "move_roll";
    case EndTurn:
        return "move_end_turn";
    case Carddecks:
        return "options_configure_carddecks";
    default:
        break;
    }

    return "";
}

KAction *KStdGameAction::gameNew(const QObject *recvr, const char *slot,
                             KActionCollection *parent, const char *name )
{
    return new KAction(i18n("new game", "&New"), "filenew",
                       KStdAccel::key(KStdAccel::New), recvr, slot, parent,
                       name ? name : stdName(New));
}

KAction *KStdGameAction::load(const QObject *recvr, const char *slot,
                                                  KActionCollection *parent, const char *name )
{
    return new KAction(i18n("&Load..."), "fileopen",
                       KStdAccel::key(KStdAccel::Open), recvr, slot, parent,
                       name ? name : stdName(Load));
}

KRecentFilesAction *KStdGameAction::loadRecent(const QObject *recvr, const char *slot,
                                                  KActionCollection *parent, const char *name )
{
    return new KRecentFilesAction(i18n("Load &Recent"), 0,
                       KStdAccel::AccelNone, recvr, slot, parent,
                       name ? name : stdName(LoadRecent));
}

KAction *KStdGameAction::save(const QObject *recvr, const char *slot,
                                                  KActionCollection *parent, const char *name )
{
    return new KAction(i18n("&Save"), "filesave",
                       KStdAccel::key(KStdAccel::Save), recvr, slot, parent,
                       name ? name : stdName(Save));
}

KAction *KStdGameAction::saveAs(const QObject *recvr, const char *slot,
                                                        KActionCollection *parent, const char *name )
{
    return new KAction(i18n("Save &As..."), 0, recvr, slot, parent,
                       name ? name : stdName(SaveAs));
}

KToggleAction *KStdGameAction::pause(const QObject *recvr, const char *slot,
                                                  KActionCollection *parent, const char *name )
{
    return new KToggleAction(i18n("Pa&use"), "player_pause", Qt::Key_P, recvr, slot, parent,
                       name ? name : stdName(Pause));
}

KAction *KStdGameAction::highscores(const QObject *recvr, const char *slot,
                                                        KActionCollection *parent, const char *name )
{
//hmm perhaps we need a KStdGameAccel one day? currently this entry is hard
//coded...
    return new KAction(i18n("Show &Highscores"), "highscore",
                       Qt::CTRL+Qt::Key_H, recvr, slot, parent,
                       name ? name : stdName(Highscores));
}

KAction *KStdGameAction::print(const QObject *recvr, const char *slot,
                                                   KActionCollection *parent, const char *name )
{
    return new KAction(i18n("&Print..."), "fileprint",
                       KStdAccel::key(KStdAccel::Print), recvr, slot, parent,
                       name ? name : stdName(Print));
}

KAction *KStdGameAction::end(const QObject *recvr, const char *slot,
                                                   KActionCollection *parent, const char *name )
{
    return new KAction(i18n("&End Game"), "fileclose",
                       KStdAccel::key(KStdAccel::End), recvr, slot, parent,
                       name ? name : stdName(End));
}

KAction *KStdGameAction::quit(const QObject *recvr, const char *slot,
                                                  KActionCollection *parent, const char *name )
{
    return new KAction(i18n("&Quit"), "exit",
                       KStdAccel::key(KStdAccel::Quit), recvr, slot, parent,
                       name ? name : stdName(Quit));
}

KAction *KStdGameAction::repeat(const QObject *recvr, const char *slot,
                                                  KActionCollection *parent, const char *name )
{
    return new KAction(i18n("Repeat"), /*0,*/// hm do we have a suitable icon for this?
//                       KStdAccel::key(KStdAccel::Redo), recvr, slot, parent,
                       0, recvr, slot, parent, // what about an accel?
                       name ? name : stdName(Repeat));
}

KAction *KStdGameAction::undo(const QObject *recvr, const char *slot,
                                                  KActionCollection *parent, const char *name )
{
    return new KAction(i18n("Und&o"), "undo",
                       KStdAccel::key(KStdAccel::Undo), recvr, slot, parent,
                       name ? name : stdName(Undo));
}

KAction *KStdGameAction::redo(const QObject *recvr, const char *slot,
                                                  KActionCollection *parent, const char *name )
{
    return new KAction(i18n("Re&do"), "redo",
                       KStdAccel::key(KStdAccel::Redo), recvr, slot, parent,
                       name ? name : stdName(Redo));
}

KAction *KStdGameAction::roll(const QObject *recvr, const char *slot,
                                                  KActionCollection *parent, const char *name )
{
//hmm perhaps we need a KStdGameAccel one day? currently this entry is hard
//coded...
    return new KAction(i18n("&Roll Dice"), "roll",
                       Qt::CTRL+Qt::Key_R, recvr, slot, parent,
                       name ? name : stdName(Roll));
}

KAction *KStdGameAction::endTurn(const QObject *recvr, const char *slot,
                                                  KActionCollection *parent, const char *name )
{
    return new KAction(i18n("End Turn"), "endturn",
                       0, recvr, slot, parent,
                       name ? name : stdName(EndTurn));
}

KAction *KStdGameAction::carddecks(const QObject *recvr, const char *slot,
                                                  KActionCollection *parent, const char *name )
{
//AB: maybe we need an icon?
    return new KAction(i18n("Configure &Carddecks..."),
                       0, recvr, slot, parent,
                       name ? name : stdName(Carddecks));
}

