/*
    SPDX-FileCopyrightText: 2007 Dmitry Suzdalev <dimsuz@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KGAMEPOPUPITEM_TEST_H
#define KGAMEPOPUPITEM_TEST_H

// own
#include "ui_kgamepopupitemtest.h"
// KDEGames
#include <KGamePopupItem>
// KF
#include <KXmlGuiWindow>

class QGraphicsScene;
class QGraphicsSimpleTextItem;

class KGpiMainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    KGpiMainWindow();

private Q_SLOTS:
    void onPopupTL();
    void onPopupTR();
    void onPopupBL();
    void onPopupBR();
    void onPopupCenter();
    void onLinkClicked(const QString &);
    void hideTextItem();
    void onTimeoutChanged(int);
    void changeIcon();
    void doInstantHide();
    void doAnimatedHide();
    void changeOpacity(int);
    void textColorChanged(const QColor &col);
    void bkgndColorChanged(const QColor &col);
    void replaceModeChanged();
    void sharpnessChanged(int);

private:
    Ui::KGpiMainWidget m_mainWid;
    QGraphicsScene *m_scene;
    KGamePopupItem *m_popupItem;
    QGraphicsSimpleTextItem *m_textItem;
    KGamePopupItem::ReplaceMode m_replaceMode;
};

#endif
