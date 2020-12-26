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

#ifndef KGAMESVGDOCUMENTTEST_H
#define KGAMESVGDOCUMENTTEST_H

// own
#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include <kgamesvgdocument.h>

class tst_KGameSvgDocument : public QObject
{
    Q_OBJECT

    KGameSvgDocument m_svgDom;

// Declare test functions as private slots, or they won't get executed
private Q_SLOTS:

    /// @brief This function is called first, so you can do init stuff here.
    void initTestCase();

    /// @brief Test various style manipulations to make sure they are reversible
    void style();

    /// @brief We scale up, then back, and verify scaling is reversible
    void scale();

    /// @brief Test the transform attribute QRegExp
    void transformRegex();

    /// @brief We test that transforms can be read and written to DOM
    void transform();

    /// @brief This function is called last, so you can do final stuff here.
    void cleanupTestCase();
};

#endif // KGAMESVGDOCUMENTTEST_H
