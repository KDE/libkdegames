/***************************************************************************
                           KStdGameAction
                           -------------------
    begin                : 13.04.2001
    copyright            : (C) 2001 by Andreas Beckermann
    email                : b_mann@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Additional license: Any of the above copyright holders can add an     *
 *   enhanced license which complies with the license of the KDE core      *
 *   libraries so that this file resp. this library is compatible with     *
 *   the KDE core libraries.                                               *
 *   The user of this program shall have the choice which license to use   *
 *                                                                         *
 ***************************************************************************/

#include <kstdgameaction.h>
#include <kstdaccel.h>
#include <klocale.h>
#include <kaction.h>

KStdGameAction::KStdGameAction()
{
}

KStdGameAction::~KStdGameAction()
{
}

KAction *KStdGameAction::action(StdGameAction act_enum, const QObject *recvr,
                            const char *slot, QObject *parent, const char *name )
{
    KAction *act;
    switch (act_enum)
    {
    case New:
        act = gameNew(recvr, slot, parent, name);
        break;
    case Load:
        act = load(recvr, slot, parent, name);
        break;
    case Save:
        act = save(recvr, slot, parent, name);
        break;
    case SaveAs:
        act = saveAs(recvr, slot, parent, name);
        break;
    case End:
        act = end(recvr, slot, parent, name);
        break;
    case Pause:
        act = pause(recvr, slot, parent, name);
	break;
    case Highscores:
        act = highscores(recvr, slot, parent, name);
	break;
    case Print:
        act = print(recvr, slot, parent, name);
        break;
    case Quit:
        act = quit(recvr, slot, parent, name);
        break;
    case Repeat:
        act = repeat(recvr, slot, parent, name);
        break;
    case Undo:
        act = undo(recvr, slot, parent, name);
        break;
    case Redo:
        act = redo(recvr, slot, parent, name);
        break;
    case EndTurn:
        act = endTurn(recvr, slot, parent, name);
        break;
    case Carddecks:
        act = carddecks(recvr, slot, parent, name);
        break;

    default:
        act = 0;
    }

    return act;
}

const char* KStdGameAction::stdName(StdGameAction act_enum)
{
    const char *ret;
    switch (act_enum)
    {
    case New:
        ret = "game_new";
        break;
    case Load:
        ret = "game_load";
        break;
    case Save:
        ret = "game_save";
        break;
    case SaveAs:
        ret = "game_save_as";
        break;
    case End:
        ret = "game_end";
        break;
    case Pause:
        ret = "game_pause";
	break;
    case Highscores:
        ret = "game_highscores";
	break;
    case Print:
        ret = "game_print";
        break;
    case Quit:
        ret = "game_quit";
        break;
    case Repeat:
        ret = "move_repeat";
	break;
    case Undo:
        ret = "move_undo";
	break;
    case Redo:
        ret = "move_redo";
	break;
    case EndTurn:
        ret = "move_endturn";
	break;
    case Carddecks:
        ret = "options_configure_carddecks";
	break;

    default:
        ret = "";
    }

    return ret;
}

KAction *KStdGameAction::gameNew(const QObject *recvr, const char *slot,
                             QObject *parent, const char *name )
{
    return new KAction(i18n("&New"), "filenew",
                       KStdAccel::key(KStdAccel::New), recvr, slot, parent,
                       name ? name : stdName(New));
}

KAction *KStdGameAction::load(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Load..."), "fileopen",
                       KStdAccel::key(KStdAccel::Open), recvr, slot, parent,
                       name ? name : stdName(Load));
}

KAction *KStdGameAction::save(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Save"), "filesave",
                       KStdAccel::key(KStdAccel::Save), recvr, slot, parent,
                       name ? name : stdName(Save));
}

KAction *KStdGameAction::saveAs(const QObject *recvr, const char *slot,
                                                        QObject *parent, const char *name )
{
    return new KAction(i18n("Save &As..."), 0, recvr, slot, parent,
                       name ? name : stdName(SaveAs));
}

KAction *KStdGameAction::pause(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("Pa&use"), 0, recvr, slot, parent,
                       name ? name : stdName(Pause));
}

KAction *KStdGameAction::highscores(const QObject *recvr, const char *slot,
                                                        QObject *parent, const char *name )
{
//hmm perhaps we need a KStdGameAccel one day? currently this entry is hard
//coded...
    return new KAction(i18n("Show Highscores"), KAccel::stringToKey("CTRL+H"), recvr, slot, parent,
                       name ? name : stdName(Highscores));
}

KAction *KStdGameAction::print(const QObject *recvr, const char *slot,
                                                   QObject *parent, const char *name )
{
    return new KAction(i18n("&Print..."), "fileprint",
                       KStdAccel::key(KStdAccel::Print), recvr, slot, parent,
                       name ? name : stdName(Print));
}

KAction *KStdGameAction::end(const QObject *recvr, const char *slot,
                                                   QObject *parent, const char *name )
{
    return new KAction(i18n("&End game"), "fileclose",
                       KStdAccel::key(KStdAccel::End), recvr, slot, parent,
                       name ? name : stdName(End));
}

KAction *KStdGameAction::quit(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("&Quit"), "exit",
                       KStdAccel::key(KStdAccel::Quit), recvr, slot, parent,
                       name ? name : stdName(Quit));
}

KAction *KStdGameAction::repeat(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("Repeat"), /*0,*/// hm do we have a suitable icon for this?
//                       KStdAccel::key(KStdAccel::Redo), recvr, slot, parent,
                       0, recvr, slot, parent, // what about an accel?
                       name ? name : stdName(Repeat));
}

KAction *KStdGameAction::undo(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("Und&o"), "undo",
                       KStdAccel::key(KStdAccel::Undo), recvr, slot, parent,
                       name ? name : stdName(Undo));
}

KAction *KStdGameAction::redo(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("Re&do"), "redo",
                       KStdAccel::key(KStdAccel::Redo), recvr, slot, parent,
                       name ? name : stdName(Redo));
}

KAction *KStdGameAction::endTurn(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
    return new KAction(i18n("End Turn"), 
                       0, recvr, slot, parent,
                       name ? name : stdName(EndTurn));
}

KAction *KStdGameAction::carddecks(const QObject *recvr, const char *slot,
                                                  QObject *parent, const char *name )
{
//AB: maybe we need an icon?
    return new KAction(i18n("Configure &Carddecks"), 
                       0, recvr, slot, parent,
                       name ? name : stdName(Carddecks));
}

