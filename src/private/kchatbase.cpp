/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>
    SPDX-FileCopyrightText: 2007 Gael de Chalendar (aka Kleag) <kleag@free.fr>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kchatbase.h"
#include "kchatbase_p.h"

// own
#include "kchatbaseitemdelegate.h"
#include "kchatbasemodel.h"
// KF
#include <KConfig>
#include <KLineEdit>
#include <KLocalizedString>
// Qt
#include <QComboBox>
#include <QHBoxLayout>
#include <QList>
#include <QListView>
#include <QVBoxLayout>

Q_LOGGING_CATEGORY(GAMES_PRIVATE, "org.kde.games.private", QtWarningMsg)

KChatBasePrivate::KChatBasePrivate(KChatBaseModel *model, KChatBaseItemDelegate *delegate, QWidget *parent)
{
    if (!model) {
        model = new KChatBaseModel(parent);
    }
    if (!delegate) {
        delegate = new KChatBaseItemDelegate(parent);
    }

    mModel = model;
    mDelegate = delegate;
}

void KChatBase::setModel(KChatBaseModel *m)
{
    Q_D(KChatBase);

    // delete d->mModel;
    d->mModel = m;
}

KChatBaseModel *KChatBase::model()
{
    Q_D(KChatBase);

    return d->mModel;
}

KChatBase::KChatBase(QWidget *parent, KChatBaseModel *model, KChatBaseItemDelegate *delegate, bool noComboBox)
    : KChatBase(*new KChatBasePrivate(model, delegate, parent), parent, noComboBox)
{
}

KChatBase::KChatBase(KChatBasePrivate &dd, QWidget *parent, bool noComboBox)
    : QFrame(parent)
    , d(&dd)
{
    setMinimumWidth(100);
    setMinimumHeight(150);

    QVBoxLayout *l = new QVBoxLayout(this);

    d->mBox = new QListView();
    d->mBox->setModel(d->mModel);
    d->mBox->setItemDelegate(d->mDelegate);
    l->addWidget(d->mBox);

    connect(d->mModel, &QAbstractItemModel::rowsInserted, d->mBox, &QAbstractItemView::scrollToBottom);

    connect(d->mBox, &QListView::customContextMenuRequested, this, &KChatBase::customMenuHandler);

    d->mBox->setContextMenuPolicy(Qt::CustomContextMenu);
    d->mBox->setFocusPolicy(Qt::NoFocus);
    d->mBox->setSelectionMode(QAbstractItemView::SingleSelection);

    l->addSpacing(5);

    QHBoxLayout *h = new QHBoxLayout;
    l->addLayout(h);
    d->mEdit = new KLineEdit(this);
    d->mEdit->setHandleSignals(false);
    d->mEdit->setTrapReturnKey(true);
    d->mEdit->completionObject(); // add the completion object
    d->mEdit->setCompletionMode(KCompletion::CompletionNone);
    connect(d->mEdit, &KLineEdit::returnKeyPressed, this, &KChatBase::slotReturnPressed);
    h->addWidget(d->mEdit);

    if (!noComboBox) {
        d->mCombo = new QComboBox(this);
        h->addWidget(d->mCombo);
        addSendingEntry(i18n("Send to All Players"), SendToAll); // FIXME: where to put the id?
    }

    d->mAcceptMessage = true; // by default
    setMaxItems(-1); // unlimited

    readConfig();
}

const QModelIndex KChatBase::indexAt(const QPoint &pos) const
{
    Q_D(const KChatBase);

    return d->mBox->indexAt(pos);
}

void KChatBase::customMenuHandler(const QPoint &pos)
{
    qCDebug(GAMES_PRIVATE) << "custom menu has been requested at position=" << pos << ". Implement handler at subclass if you need it.";
}

KChatBase::~KChatBase()
{
    // qCDebug(GAMES_LIB) << "KChatBase: DESTRUCT (" << this << ")";
    saveConfig();
}

bool KChatBase::acceptMessage() const
{
    Q_D(const KChatBase);

    return d->mAcceptMessage;
}

void KChatBase::setAcceptMessage(bool a)
{
    Q_D(KChatBase);

    d->mAcceptMessage = a;
}

bool KChatBase::addSendingEntry(const QString &text, int id)
{
    // FIXME: is ID used correctly?
    // do we need ID at all?
    // what the hell should be here?
    // d->mCombo->insertItem(i18n("Send to All Players"), SendToAll);
    return insertSendingEntry(text, id);
}

bool KChatBase::insertSendingEntry(const QString &text, int id, int index)
{
    Q_D(KChatBase);

    if (!d->mCombo) {
        qCWarning(GAMES_LIB) << "KChatBase: Cannot add an entry to the combo box";
        return false;
    }
    if (d->mIndex2Id.indexOf(id) != -1) {
        qCCritical(GAMES_LIB) << "KChatBase: Cannot add more than one entry with the same ID! ";
        qCCritical(GAMES_LIB) << "KChatBase: Text=" << text;
        return false;
    }
    d->mCombo->insertItem(index, text);
    if (index < 0) {
        d->mIndex2Id.prepend(id);
    } else {
        d->mIndex2Id.insert(d->mIndex2Id.at(index), id);
    }
    if (d->mIndex2Id.count() != d->mCombo->count()) {
        qCCritical(GAMES_LIB) << "KChatBase: internal ERROR - local IDs do not match combo box entries!";
    }
    return true;
}

int KChatBase::sendingEntry() const
{
    Q_D(const KChatBase);

    if (!d->mCombo) {
        qCWarning(GAMES_PRIVATE_KGAME) << "Cannot retrieve index from NULL combo box";
        return -1;
    }
    const int index = d->mCombo->currentIndex();
    if (index >= 0 && index < d->mIndex2Id.size())
        return d->mIndex2Id[index];

    qCWarning(GAMES_LIB) << "could not find the selected sending entry!";
    return -1;
}

void KChatBase::removeSendingEntry(int id)
{
    Q_D(KChatBase);

    if (!d->mCombo) {
        qCWarning(GAMES_LIB) << "KChatBase: Cannot remove an entry from the combo box";
        return;
    }
    int idx = findIndex(id);
    // Guard, passing -1 will crash qcombobox
    if (idx >= 0)
        d->mCombo->removeItem(idx);
    d->mIndex2Id.removeAll(id);
}

void KChatBase::changeSendingEntry(const QString &text, int id)
{
    Q_D(KChatBase);

    if (!d->mCombo) {
        qCWarning(GAMES_LIB) << "KChatBase: Cannot change an entry in the combo box";
        return;
    }
    int index = findIndex(id);
    d->mCombo->setItemText(index, text);
}

void KChatBase::setSendingEntry(int id)
{
    Q_D(KChatBase);

    if (!d->mCombo) {
        qCWarning(GAMES_LIB) << "KChatBase: Cannot set an entry in the combo box";
        return;
    }
    d->mCombo->setCurrentIndex(findIndex(id));
}

int KChatBase::findIndex(int id) const
{
    Q_D(const KChatBase);

    return d->mIndex2Id.indexOf(id);
}

int KChatBase::nextId() const
{
    Q_D(const KChatBase);

    int i = SendToAll + 1;
    while (d->mIndex2Id.indexOf(i) != -1) {
        i++;
    }
    return i;
}

void KChatBase::slotReturnPressed(const QString &text)
{
    Q_D(KChatBase);

    if (text.length() <= 0) {
        // no text entered - probably hit return by accident
        return;
    } else if (!acceptMessage()) {
        return;
    }
    d->mEdit->completionObject()->addItem(text);
    d->mEdit->clear();
    returnPressed(text);
}

QString KChatBase::comboBoxItem(const QString &name) const
{ // TODO: such a function for "send to all" and "send to my group"
    return i18n("Send to %1", name);
}

void KChatBase::slotClear()
{
    clear();
}

void KChatBase::setCompletionMode(KCompletion::CompletionMode mode)
{
    Q_D(KChatBase);

    d->mEdit->setCompletionMode(mode);
}

void KChatBase::saveConfig(KConfig *conf)
{
    Q_D(KChatBase);

    if (conf == nullptr) {
        return;
    }
    d->mModel->saveConfig(conf);
}

void KChatBase::readConfig(KConfig *conf)
{
    Q_D(KChatBase);

    if (conf == nullptr) {
        return;
    }
    d->mModel->readConfig(conf);
}

void KChatBase::clear()
{
    Q_D(KChatBase);

    d->mModel->removeRows(0, d->mModel->rowCount());
}

void KChatBase::setMaxItems(int maxItems)
{
    Q_D(KChatBase);

    d->mModel->setMaxItems(maxItems);
    // TODO cut too many messages
    if (maxItems == 0) {
        clear();
    } else if (maxItems > 0) {
        while (d->mModel->rowCount() > (int)maxItems) {
            d->mModel->removeRow(0);
        }
    }
}

int KChatBase::maxItems() const
{
    Q_D(const KChatBase);

    return d->mModel->maxItems();
}

QFont KChatBase::nameFont() const
{
    Q_D(const KChatBase);

    return d->mModel->nameFont();
}

QFont KChatBase::messageFont() const
{
    Q_D(const KChatBase);

    return d->mModel->messageFont();
}

QFont KChatBase::systemNameFont() const
{
    Q_D(const KChatBase);

    return d->mModel->systemNameFont();
}

QFont KChatBase::systemMessageFont() const
{
    Q_D(const KChatBase);

    return d->mModel->systemMessageFont();
}

void KChatBase::setNameFont(const QFont &font)
{
    Q_D(KChatBase);

    d->mModel->setNameFont(font);
}

void KChatBase::setMessageFont(const QFont &font)
{
    Q_D(KChatBase);

    d->mModel->setMessageFont(font);
}

void KChatBase::setBothFont(const QFont &font)
{
    Q_D(KChatBase);

    d->mModel->setBothFont(font);
}

void KChatBase::setSystemNameFont(const QFont &font)
{
    Q_D(KChatBase);

    d->mModel->setSystemNameFont(font);
}

void KChatBase::setSystemMessageFont(const QFont &font)
{
    Q_D(KChatBase);

    d->mModel->setSystemMessageFont(font);
}

void KChatBase::setSystemBothFont(const QFont &font)
{
    Q_D(KChatBase);

    d->mModel->setSystemBothFont(font);
}

void KChatBase::addMessage(const QString &fromName, const QString &text)
{
    Q_D(KChatBase);

    d->mModel->addMessage(fromName, text);
}

void KChatBase::addSystemMessage(const QString &fromName, const QString &text)
{
    Q_D(KChatBase);

    d->mModel->addSystemMessage(fromName, text);
}
