/*
    SPDX-FileCopyrightText: 2012 Viranch Mehta <viranch.mehta@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgdeclarativeview.h"

// KF
#include <KDeclarative/KDeclarative>
#include <KLocalizedContext>
// Qt
#include <QQmlContext>

KgDeclarativeView::KgDeclarativeView(QWidget *parent) :
    QQuickWidget(parent),
    d(nullptr) //unused for now, for future expandability
{
    QQmlEngine* engine = this->engine();
    KDeclarative::KDeclarative::setupEngine(engine);

    KLocalizedContext *localizedContextObject = new KLocalizedContext(engine);
    engine->rootContext()->setContextObject(localizedContextObject);

    setResizeMode(SizeRootObjectToView);
}


