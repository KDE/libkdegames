/*
    SPDX-FileCopyrightText: 2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KGAMESVGDOCUMENTTEST_H
#define KGAMESVGDOCUMENTTEST_H

// own
#define USE_UNSTABLE_LIBKDEGAMESPRIVATE_API
#include "kgamesvgdocument.h"
// Qt
#include <QObject>

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
