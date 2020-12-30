/*
    SPDX-FileCopyrightText: 2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KGAMEPROPERTYTEST_H
#define KGAMEPROPERTYTEST_H

// own
#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include "kgame/kgame.h"
#include "kgame/kgamepropertyhandler.h"
// Qt
#include <QDataStream>

/// @brief A test class for KGameSvgDigits
class tst_KGamePropertyTest : public QObject
{
    Q_OBJECT

  // Declare test functions as private slots, or they won't get executed
  private Q_SLOTS:

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
