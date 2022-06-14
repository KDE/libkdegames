/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2003 Andreas Beckermann <b_mann@gmx.de>
    SPDX-FileCopyrightText: 2003 Martin Heni <kde at heni-online.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef __KGAMEVERSION_H__
#define __KGAMEVERSION_H__

/**
 * \headerfile kgameversion.h <KGame/KGameVersion>
 *
 * In this file you find a couple of defines that indicate whether a specific
 * feature or function is present in this version of the KGame library.
 *
 * You don't need this for KDE CVS, but for games that live outside of KDE CVS
 * it may be very helpful and a lot easier than writing configure scripts for
 * this task.
 *
 * All defines are prefixed with KGAME_ to avoid conflicts.
 */

// KGame::savegame() didn't exist in KDE 3.0
#define KGAME_HAVE_KGAME_SAVEGAME 1

// KGameNetwork::port(), KMessageIO::peerPort() and friends were added in KDE 3.2
#define KGAME_HAVE_KGAME_PORT 1

// KGameNetwork::hostName(), KMessageIO::peerName() and friends were added in KDE 3.2
#define KGAME_HAVE_KGAME_HOSTNAME 1

// KGameSequence class was added in KDE 3.2
#define KGAME_HAVE_KGAMESEQUENCE 1

// KGame::addPlayer() needs to assign an ID to new players, otherwise network is
// broken. this is done in KDE 3.2.
#define KGAME_HAVE_FIXED_ADDPLAYER_ID 1

#endif
