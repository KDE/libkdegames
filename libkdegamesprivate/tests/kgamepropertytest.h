/***************************************************************************
 *   Copyright 2012 Stefan Majewsky <majewsky@gmx.net>                     *
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

#ifndef KGAMEPROPERTYTEST_H
#define KGAMEPROPERTYTEST_H

#include <QDataStream>

#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include "kgame/kgame.h"
#include "kgame/kgamepropertyhandler.h"

/// @brief A test class for KGameSvgDigits
class tst_KGamePropertyTest : public QObject
{
    Q_OBJECT

  // Declare test functions as private slots, or they won't get executed
  private slots:

    /// @brief This function is called first, so you can do init stuff here.
    void initTestCase();

    /// @brief This function is called last, so you can do final stuff here.
    void cleanupTestCase();

    /// @brief Test the property handler
    void testHandler();

    /// @brief Implement property slot to test property signals
    void emitSignal(KGamePropertyBase *me);

    /// @brief Implement property slot to test property sending
    void sendProperty(int msgid, QDataStream& stream, bool* sent);

  private:
    KGamePropertyHandler* mHandler;
    KGamePropertyInt var1;
    KGamePropertyQString var2;
    int mCnt_Emit;
    int mCnt_Send;

};

#endif // KGAMEPROPERTYTEST_H
