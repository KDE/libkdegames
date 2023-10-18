/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>
    SPDX-FileCopyrightText: 2007 Gael de Chalendar (aka Kleag) <kleag@free.fr>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef __KCHATBASE_P_H__
#define __KCHATBASE_P_H__

#include "kchatbase.h"

class QListView;
class KLineEdit;
class QComboBox;

class KChatBasePrivate
{
public:
    KChatBasePrivate(KChatBaseModel *model, KChatBaseItemDelegate *delegate, QWidget *parent);
    virtual ~KChatBasePrivate() = default;

public:
    QListView *mBox = nullptr;
    KLineEdit *mEdit = nullptr;
    QComboBox *mCombo = nullptr;
    bool mAcceptMessage = true;

    QList<int> mIndex2Id;

    KChatBaseModel *mModel;
    KChatBaseItemDelegate *mDelegate;
};

#endif
