/*
    Copyright (C) 2007 Mauricio Piacentini  <mauricio@tabuleiro.com>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "kgamethemeselector.h"

#include <KLocale>
#include <KStandardDirs>
#include <QtGui/QPainter>
#include <KConfigSkeleton>
#include <knewstuff2/engine.h>

#include "ui_kgamethemeselector.h"
#include "kgametheme.h"

class KGameThemeSelector::KGameThemeSelectorPrivate
{
    public:
        KGameThemeSelectorPrivate(KGameThemeSelector* parent) : q(parent) {};
        KGameThemeSelector* q;
        
        QMap<QString, KGameTheme*> themeMap;
        Ui::KGameThemeSelectorBase ui;
        
        void setupData(KConfigSkeleton* config);
};

KGameThemeSelector::KGameThemeSelector( QWidget* parent, KConfigSkeleton * aconfig )
    : QWidget(parent), d(new KGameThemeSelectorPrivate(this))
{
    d->setupData(aconfig);
}

void KGameThemeSelector::KGameThemeSelectorPrivate::setupData(KConfigSkeleton * aconfig)
{
    ui.setupUi(q);
    
    //Get our currently configured Tileset entry
    KConfig * config = aconfig->config();
    KConfigGroup group = config->group("General");
    QString initialGroup = group.readEntry("Theme");

    //The lineEdit widget holds our bg path, but the user does not manipulate it directly
    ui.kcfg_Theme->hide();

    KGameTheme bg;

    //Now get our tilesets into a list
    QStringList themesAvailable;
    KGlobal::dirs()->findAllResources("gametheme", QString("*.desktop"), KStandardDirs::Recursive, themesAvailable);
    QString namestr("Name");
    int numvalidentries = 0;
    for (int i = 0; i < themesAvailable.size(); ++i)
    {
        KGameTheme * atheme = new KGameTheme();
        QString themepath = themesAvailable.at(i);
        if (atheme->load(themepath)) {
            themeMap.insert(atheme->authorProperty(namestr), atheme);
            ui.themeList->addItem(atheme->authorProperty(namestr));
            //Find if this is our currently configured Theme
            if (themepath==initialGroup) {
                //Select current entry
                ui.themeList->setCurrentRow(numvalidentries);
                q->updatePreview();
            }
            ++numvalidentries;
        } else {
            delete atheme;
        }
    }
    
    connect(ui.themeList, SIGNAL(currentItemChanged ( QListWidgetItem * , QListWidgetItem * )), q, SLOT(updatePreview()));
    connect(ui.getNewButton, SIGNAL(clicked()), q, SLOT(openKNewStuffDialog()));
}

void KGameThemeSelector::updatePreview()
{
    KGameTheme * seltheme = d->themeMap.value(d->ui.themeList->currentItem()->text());
        //Sanity checkings. Should not happen.
    if (!seltheme) return;
    if (seltheme->path()==d->ui.kcfg_Theme->text()) {
        return;
    }
    QString authstr("Author");
    QString contactstr("AuthorEmail");
    QString descstr("Description");
    d->ui.kcfg_Theme->setText(seltheme->fileName());
    d->ui.themeAuthor->setText(seltheme->authorProperty(authstr));
    d->ui.themeContact->setText(seltheme->authorProperty(contactstr));
    d->ui.themeDescription->setText(seltheme->authorProperty(descstr));

    //Draw the preview
    //TODO here: add code to maintain aspect ration?
    d->ui.themePreview->setPixmap(seltheme->preview());

}

void KGameThemeSelector::openKNewStuffDialog()
{
    KNS::Entry::List entries = KNS::Engine::download();
}

#include "kgamethemeselector.moc"
