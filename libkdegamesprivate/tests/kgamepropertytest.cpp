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

#include "kgamepropertytest.h"
#include "config-tests.h"

#include <QtCore>
#include <QtTest>


void tst_KGamePropertyTest::initTestCase()
{
    // Do nothing.
}

void tst_KGamePropertyTest::testHandler()
{
  mCnt_Send = 0;
  mCnt_Emit = 0;

  // (TESTS_PATH"kgamesvgdigitstest.svg")
  mHandler = new KGamePropertyHandler(this);

   mHandler->registerHandler(1000 /* ID */,
                             this,SLOT(sendProperty(int,QDataStream&,bool*)),
                                  SLOT(emitSignal(KGamePropertyBase*)));

   var1.registerData(10 /* ID */, mHandler, QStringLiteral("VAR1"));
   var1.setLocal(-1); 
   var2.registerData(11 /* ID */, mHandler, QStringLiteral("VAR2"));
   var2.setLocal(QStringLiteral("Hallo"));


   // Check VAR 1
   int checkI = var1;
   QVERIFY(checkI == -1);
   var1 = 42;
   QVERIFY(var1 == 42);

   // Check VAR 2
   QString checkS = var2;
   QVERIFY(checkS == QString("Hallo"));


   // Check names
   QString nameS = mHandler->propertyName(10);
   QVERIFY(nameS.contains( QLatin1String( "VAR1" )));
   nameS = mHandler->propertyName(11);
   QVERIFY(nameS.contains( QLatin1String( "VAR2" )));


   // Check IDs
   QVERIFY(var1.id() == 10);
   QVERIFY(var2.id() == 11);

   // Flush handler
   mHandler->flush();

   // Check size and clear
   QMultiHash<int, KGamePropertyBase*> dict1 = mHandler->dict();
   QVERIFY(dict1.size() == 2);
   mHandler->clear();
   QMultiHash<int, KGamePropertyBase*> dict2 = mHandler->dict();
   QVERIFY(dict2.size() == 0);


   // Check all count statistic
   QVERIFY(mCnt_Emit == 3);
   QVERIFY(mCnt_Send == 2);
}

void tst_KGamePropertyTest::sendProperty(int , QDataStream& , bool* sent)
{
  *sent = true;
  mCnt_Send++;

}
void tst_KGamePropertyTest::emitSignal(KGamePropertyBase*)
{
 mCnt_Emit++;
}


void tst_KGamePropertyTest::cleanupTestCase()
{
    // Do nothing.
}

QTEST_MAIN(tst_KGamePropertyTest)

