/*
    SPDX-FileCopyrightText: 2012 Viranch Mehta <viranch.mehta@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "corebindingsplugin.h"

// own
#include <KgThemeProvider>

void CoreBindingsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QByteArray("org.kde.games.core"));

    qmlRegisterType<KgThemeProvider>(uri, 0, 1, "ThemeProvider");
}


