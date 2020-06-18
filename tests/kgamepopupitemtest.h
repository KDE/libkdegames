/***************************************************************************
 *   Copyright 2007 Dmitry Suzdalev <dimsuz@gmail.com>                     *
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

#ifndef KGAMEPOPUPITEM_TEST_H
#define KGAMEPOPUPITEM_TEST_H

#include <KXmlGuiWindow>

#include "ui_kgamepopupitemtest.h"
#include <KGamePopupItem>

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
    void onLinkClicked(const QString&);
    void hideTextItem();
    void onTimeoutChanged(int);
    void changeIcon();
    void doInstantHide();
    void doAnimatedHide();
    void changeOpacity(int);
    void textColorChanged(const QColor& col);
    void bkgndColorChanged(const QColor& col);
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
