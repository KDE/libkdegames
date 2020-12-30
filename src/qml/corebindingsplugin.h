/*
    SPDX-FileCopyrightText: 2012 Viranch Mehta <viranch.mehta@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef COREBINDINGSPLUGIN_H
#define COREBINDINGSPLUGIN_H

// Qt
#include <QQmlExtensionPlugin>

class CoreBindingsPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override;
};

#endif // COREBINDINGSPLUGIN_H
