/*
    SPDX-FileCopyrightText: 2007 Mauricio Piacentini <mauricio@tabuleiro.com>
    SPDX-FileCopyrightText: 2007 Matt Williams <matt@milliams.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgamethemeselector.h"

// own
#include "kgametheme.h"
#include "ui_kgamethemeselector.h"
// KF
#include <KConfigSkeleton>
// Qt
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QIcon>
#include <QStandardPaths>

#if KDEGAMESPRIVATE_BUILD_DEPRECATED_SINCE(4, 9)

class KGameThemeSelectorPrivate
{
public:
    KGameThemeSelectorPrivate(KGameThemeSelector *parent)
        : q(parent)
    {
    }
    ~KGameThemeSelectorPrivate()
    {
        qDeleteAll(themeMap);
    }

    KGameThemeSelector *q;

    QMap<QString, KGameTheme *> themeMap;
    Ui::KGameThemeSelectorBase ui;
    QString lookupDirectory;
    QString groupName;

    void setupData(KConfigSkeleton *config, KGameThemeSelector::NewStuffState knsflags);
    void findThemes(const QString &initialSelection);

    // private slots
    void _k_updatePreview();
    void _k_updateThemeList(const QString &strTheme);
};

KGameThemeSelector::KGameThemeSelector(QWidget *parent,
                                       KConfigSkeleton *aconfig,
                                       KGameThemeSelector::NewStuffState knsflags,
                                       const QString &groupName,
                                       const QString &directory)
    : QWidget(parent)
    , d(new KGameThemeSelectorPrivate(this))
{
    d->lookupDirectory = directory;
    d->groupName = groupName;
    d->setupData(aconfig, knsflags);
}

KGameThemeSelector::~KGameThemeSelector() = default;

void KGameThemeSelectorPrivate::setupData(KConfigSkeleton *aconfig, KGameThemeSelector::NewStuffState knsflags)
{
    ui.setupUi(q);

    // The lineEdit widget holds our theme path for automatic connection via KConfigXT.
    // But the user should not manipulate it directly, so we hide it.
    ui.kcfg_Theme->hide();
    QObject::connect(ui.kcfg_Theme, &QLineEdit::textChanged, q, [this](const QString &text) {
        _k_updateThemeList(text);
    });

    // Disable KNS button?
    if (knsflags == KGameThemeSelector::NewStuffDisableDownload) {
        ui.getNewButton->hide();
    }

    // Get the last used theme path from the KConfigSkeleton
    KConfigSkeletonItem *configItem = aconfig->findItem(QStringLiteral("Theme"));
    QString lastUsedTheme = configItem->property().toString();

    // Now get our themes into the list widget

    findThemes(lastUsedTheme);

    ui.getNewButton->setConfigFile(QCoreApplication::applicationName() + QLatin1String(".knsrc"));
    QObject::connect(ui.getNewButton, &KNSWidgets::Button::dialogFinished, q, [this](const QList<KNSCore::Entry> &changedEntries) {
        if (!changedEntries.isEmpty()) {
            findThemes(ui.kcfg_Theme->text());
        }
    });
}

void KGameThemeSelectorPrivate::findThemes(const QString &initialSelection)
{
    qDeleteAll(themeMap);
    themeMap.clear();

    // Disconnect the themeList as we are going to clear it and do not want previews generated
    ui.themeList->disconnect();
    ui.themeList->clear();
    ui.themeList->setSortingEnabled(true);

    QStringList themesAvailable;

    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                       QCoreApplication::applicationName() + QLatin1Char('/') + lookupDirectory,
                                                       QStandardPaths::LocateDirectory); // Added subdirectory for finding gamethemeselector resources
    for (const QString &dir : dirs) {
        QDirIterator it(dir, QStringList() << QStringLiteral("*.desktop"), QDir::NoFilter, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QFileInfo fileInfo(it.next());
            const QString filePath = QDir(dir).relativeFilePath(fileInfo.filePath());
            themesAvailable.append(filePath);
        }
    }

    bool initialFound = false;
    for (const QString &file : qAsConst(themesAvailable)) {
        const QString themePath = lookupDirectory + QLatin1Char('/') + file;
        KGameTheme *atheme = new KGameTheme(groupName);

        if (atheme->load(themePath)) {
            QString themeName = atheme->themeProperty(QStringLiteral("Name"));
            // Add underscores to avoid duplicate names.
            while (themeMap.contains(themeName))
                themeName += QLatin1Char('_');
            themeMap.insert(themeName, atheme);
            QListWidgetItem *item = new QListWidgetItem(themeName, ui.themeList);

            // Find if this is our currently configured theme
            if (themePath == initialSelection) {
                initialFound = true;
                ui.themeList->setCurrentItem(item);
                _k_updatePreview();
            }
        } else {
            delete atheme;
        }
    }

    if (!initialFound) {
        // TODO change this if we ever change KGameTheme::loadDefault
        QString defaultPath = QStringLiteral("themes/default.desktop");
        for (KGameTheme *theme : qAsConst(themeMap)) {
            if (theme->path().endsWith(defaultPath)) {
                const QList<QListWidgetItem *> itemList = ui.themeList->findItems(theme->themeProperty(QStringLiteral("Name")), Qt::MatchExactly);
                // never can be != 1 but better safe than sorry
                if (itemList.count() == 1) {
                    ui.themeList->setCurrentItem(itemList.first());
                    _k_updatePreview();
                }
            }
        }
    }

    // Reconnect the themeList
    QObject::connect(ui.themeList, &QListWidget::currentItemChanged, q, [this]() {
        _k_updatePreview();
    });
}

void KGameThemeSelectorPrivate::_k_updatePreview()
{
    KGameTheme *seltheme = themeMap.value(ui.themeList->currentItem()->text());
    // Sanity checkings. Should not happen.
    if (!seltheme)
        return;
    if (seltheme->path() == ui.kcfg_Theme->text()) {
        return;
    }
    ui.kcfg_Theme->setText(seltheme->fileName());

    QString authstr(QStringLiteral("Author"));
    QString contactstr(QStringLiteral("AuthorEmail"));
    QString descstr(QStringLiteral("Description"));
    QString emailstr;
    if (!seltheme->themeProperty(contactstr).isEmpty()) {
        emailstr = QStringLiteral("<a href=\"mailto:%1\">%1</a>").arg(seltheme->themeProperty(contactstr));
    }

    ui.themeAuthor->setText(seltheme->themeProperty(authstr));
    ui.themeContact->setText(emailstr);
    ui.themeDescription->setText(seltheme->themeProperty(descstr));

    // Draw the preview
    const qreal dpr = q->devicePixelRatioF();
    QPixmap pix(seltheme->preview());
    pix.setDevicePixelRatio(dpr);
    ui.themePreview->setPixmap(pix.scaled(ui.themePreview->size() * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void KGameThemeSelectorPrivate::_k_updateThemeList(const QString &strTheme)
{
    // find theme and set selection to the current theme; happens when pressing "Default"
    QListWidgetItem *currentItem = ui.themeList->currentItem();
    if (!currentItem || themeMap.value(currentItem->text())->fileName() != strTheme) {
        for (int i = 0; i < ui.themeList->count(); i++) {
            if (themeMap.value(ui.themeList->item(i)->text())->fileName() == strTheme) {
                ui.themeList->setCurrentItem(ui.themeList->item(i));
                break;
            }
        }
    }
}

#include "moc_kgamethemeselector.cpp"

#endif
