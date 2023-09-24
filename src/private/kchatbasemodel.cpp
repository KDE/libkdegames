/*
    This file is part of the KDE games library
    SPDX-FileCopyrightText: 2001 Andreas Beckermann <b_mann@gmx.de>
    SPDX-FileCopyrightText: 2007 Gael de Chalendar (aka Kleag) <kleag@free.fr>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kchatbasemodel.h"

// KF
#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
// Qt
#include <QFont>
#include <QSharedData>

Q_LOGGING_CATEGORY(GAMES_LIB, "org.kde.games.lib", QtWarningMsg)

class KChatBaseMessagePrivate : public QSharedData
{
public:
    KChatBaseMessagePrivate()
    {
        m_type = KChatBaseMessage::Normal;
    }

    KChatBaseMessage::MessageType m_type;
};

KChatBaseMessage::KChatBaseMessage()
    : d(new KChatBaseMessagePrivate())
{
}

KChatBaseMessage::KChatBaseMessage(const QString &sender, const QString &message, MessageType type)
    : QPair<QString, QString>(sender, message)
    , d(new KChatBaseMessagePrivate())
{
    d->m_type = type;
}

KChatBaseMessage::KChatBaseMessage(const KChatBaseMessage &other) = default;

KChatBaseMessage::~KChatBaseMessage() = default;

KChatBaseMessage &KChatBaseMessage::operator=(const KChatBaseMessage &other) = default;

class KChatBaseModelPrivate
{
public:
    KChatBaseModelPrivate()
    {
        mAcceptMessage = true;
        mMaxItems = -1;
    }

    bool mAcceptMessage;
    int mMaxItems;

    QList<int> mIndex2Id;

    QFont mNameFont;
    QFont mMessageFont;
    QFont mSystemNameFont;
    QFont mSystemMessageFont;

    QList<KChatBaseMessage> m_messages;
};

KChatBaseModel::KChatBaseModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new KChatBaseModelPrivate())
{
}

KChatBaseModel::~KChatBaseModel()
{
    qCDebug(GAMES_LIB) << "KChatBaseModelPrivate: DESTRUCT (" << this << ")";
    saveConfig();
}

void KChatBaseModel::slotClear()
{
    clear();
}

void KChatBaseModel::setNameFont(const QFont &font)
{
    d->mNameFont = font;
}

void KChatBaseModel::setMessageFont(const QFont &font)
{
    d->mMessageFont = font;
}

void KChatBaseModel::setBothFont(const QFont &font)
{
    setNameFont(font);
    setMessageFont(font);
}

const QFont &KChatBaseModel::nameFont() const
{
    return d->mNameFont;
}

const QFont &KChatBaseModel::messageFont() const
{
    return d->mMessageFont;
}

void KChatBaseModel::setSystemNameFont(const QFont &font)
{
    d->mSystemNameFont = font;
}

void KChatBaseModel::setSystemMessageFont(const QFont &font)
{
    d->mSystemMessageFont = font;
}

void KChatBaseModel::setSystemBothFont(const QFont &font)
{
    setSystemNameFont(font);
    setSystemMessageFont(font);
}

const QFont &KChatBaseModel::systemNameFont() const
{
    return d->mSystemNameFont;
}

const QFont &KChatBaseModel::systemMessageFont() const
{
    return d->mSystemMessageFont;
}

void KChatBaseModel::saveConfig(KConfig *conf)
{
    if (!conf) {
        conf = KSharedConfig::openConfig().data();
    }
    KConfigGroup cg(conf, "KChatBaseModelPrivate");

    cg.writeEntry("NameFont", nameFont());
    cg.writeEntry("MessageFont", messageFont());
    cg.writeEntry("SystemNameFont", systemNameFont());
    cg.writeEntry("SystemMessageFont", systemMessageFont());
    cg.writeEntry("MaxMessages", maxItems());
}

void KChatBaseModel::readConfig(KConfig *conf)
{
    if (!conf) {
        conf = KSharedConfig::openConfig().data();
    }
    KConfigGroup cg(conf, "KChatBaseModelPrivate");

    setNameFont(cg.readEntry("NameFont", QFont()));
    setMessageFont(cg.readEntry("MessageFont", QFont()));
    setSystemNameFont(cg.readEntry("SystemNameFont", QFont()));
    setSystemMessageFont(cg.readEntry("SystemMessageFont", QFont()));
    setMaxItems(cg.readEntry("MaxMessages", -1));
}

void KChatBaseModel::clear()
{
    removeRows(0, rowCount());
}

void KChatBaseModel::setMaxItems(int maxItems)
{
    d->mMaxItems = maxItems;
    // TODO cut too many messages
    if (maxItems == 0) {
        clear();
    } else if (maxItems > 0) {
        while (rowCount() > (int)maxItems) {
            removeRow(0);
        }
    }
}

int KChatBaseModel::maxItems() const
{
    return d->mMaxItems;
}

QVariant KChatBaseModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        KChatBaseMessage p = d->m_messages[index.row()];
        return QVariant::fromValue(p);
    }
    return QVariant();
}

int KChatBaseModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return d->m_messages.size();
}

void KChatBaseModel::addMessage(const QString &fromName, const QString &text)
{
    int row;
    row = d->m_messages.size();
    beginInsertRows(QModelIndex(), row, row);
    d->m_messages.push_back(KChatBaseMessage(fromName, text));
    endInsertRows();

    while (maxItems() > -1 && rowCount() > maxItems()) {
        beginRemoveRows(QModelIndex(), row, row);
        d->m_messages.pop_front();
        endRemoveRows();
    }
}

void KChatBaseModel::addSystemMessage(const QString &fromName, const QString &text)
{
    int row;
    row = d->m_messages.size();
    beginInsertRows(QModelIndex(), row, row);
    d->m_messages.push_back(KChatBaseMessage(fromName, text, KChatBaseMessage::System));
    endInsertRows();
}

#include "moc_kchatbasemodel.cpp"
