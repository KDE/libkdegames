/*
    SPDX-FileCopyrightText: 2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KGAMETHEMESELECTOR_H
#define KGAMETHEMESELECTOR_H

// own
#include "kdegames_export.h"
#include "kgamethemeprovider.h"
// Qt
#include <QWidget>
// Std
#include <memory>

/*!
 * \class KGameThemeSelector
 * \inmodule LibKDEGames
 * \brief Theme selection widget.
 *
 * This widget allows the user to change the theme selection of a
 * KGameThemeProvider. Selections are immediately applied to allow the user
 * to quickly preview themes. In simple cases, the widget can be used
 * standalone with the showAsDialog() method.
 *
 * \code
 * K_GLOBAL_STATIC_WITH_ARGS(KGameThemeSelector, selector, (provider))
 * ...
 * selector->showAsDialog();
 * \endcode
 */
class KDEGAMES_EXPORT KGameThemeSelector : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(KGameThemeSelector)

public:
    /*!
     * \enum KGameThemeSelector::Option
     * Flags which control the behavior of KGameThemeSelector.
     * \value DefaultBehavior
     * \value EnableNewStuffDownload
     *        Enable downloading of additional themes with KNewStuff.
     *        This requires a KNS config file to be installed for this app.
     */
    enum Option {
        DefaultBehavior = 0,
        EnableNewStuffDownload = 1 << 0
    };
    /**
     * Stores a combination of \l Option values.
     */
    Q_DECLARE_FLAGS(Options, Option)

    /*!
     * Constructs a KGameThemeSelector with \a provider, \a options, and \ a parent.
     */
    explicit KGameThemeSelector(KGameThemeProvider *provider, Options options = DefaultBehavior, QWidget *parent = nullptr);
    ~KGameThemeSelector() override;

    void setNewStuffConfigFileName(const QString &configFileName);

public Q_SLOTS:
    /*
     * Creates and shows a non-modal dialog with \a caption which displays this selector.
     * The dialog will be automatically cleaned up when it's closed, but it
     * is ensured that the selector is not deleted.
     *
     * This method does nothing if the selector widget is already visible.
     */
    void showAsDialog(const QString &caption = QString());

private:
    class Dialog;
    std::unique_ptr<class KGameThemeSelectorPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(KGameThemeSelector)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KGameThemeSelector::Options)

#endif // KGAMETHEMESELECTOR_H
