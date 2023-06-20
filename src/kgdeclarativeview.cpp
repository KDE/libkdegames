/*
    SPDX-FileCopyrightText: 2012 Viranch Mehta <viranch.mehta@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgdeclarativeview.h"

// KF
#include <KLocalizedContext>
#include <KQuickIconProvider>
// Qt
#include <QQmlContext>

#if KDEGAMES_BUILD_DEPRECATED_SINCE(7, 5)

class KgDeclarativeViewPrivate
{
};

KgDeclarativeView::KgDeclarativeView(QWidget *parent)
    : QQuickWidget(parent)
    , d(nullptr) // unused for now, for future expandability
{
    QQmlEngine *engine = this->engine();

    // setup ImageProvider for KIconTheme icons
    engine->addImageProvider(QStringLiteral("icon"), new KQuickIconProvider);

    KLocalizedContext *localizedContextObject = new KLocalizedContext(engine);
    engine->rootContext()->setContextObject(localizedContextObject);

    setResizeMode(SizeRootObjectToView);
}

KgDeclarativeView::~KgDeclarativeView() = default;

#include "moc_kgdeclarativeview.cpp"

#endif
