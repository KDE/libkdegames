/*
    This file is part of the KDE games library
    Copyright 2008 Andreas Pakulat <apaku@gmx.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QFileInfo>
#include <QDir>

#include <klocale.h>
#include <kstandarddirs.h>
#include <krandom.h>
#include <kdebug.h>
#include <kconfiggroup.h>
#include <kglobal.h>

#include "carddeckinfo.h"
#include "carddeckinfo_p.h"

/**
 * Local static information.
 */
class KCardThemeInfoStatic
{
public:
    KCardThemeInfoStatic()
    {
        KGlobal::dirs()->addResourceType( "cards", "data", "carddecks/" );
        KGlobal::locale()->insertCatalog( "libkdegames" );
        readBacks();
        readFronts();
    }
    ~KCardThemeInfoStatic()
    {
    }
    
    
    // Translate back side
    QString findi18nBack( QString& name )
    {
        if ( name.isNull() ) return name;
    
        QMap<QString, KCardThemeInfo> temp = svgBackInfo;
        temp.unite( pngBackInfo );
    
        QMapIterator<QString, KCardThemeInfo> it( temp );
        while ( it.hasNext() )
        {
            it.next();
            KCardThemeInfo v = it.value();
            if ( v.noi18Name == name ) return v.name;
        }
        kError() << "No translation for back card " << name << "found";
        return name;
    }

    void readFronts()
    {
        // Empty data
        pngFrontInfo.clear();
        svgFrontInfo.clear();

        QStringList svg;
        // Add SVG card sets
        svg = KGlobal::dirs()->findAllResources( "cards", "svg*/index.desktop", KStandardDirs::NoDuplicates );
        QStringList list = svg + KGlobal::dirs()->findAllResources( "cards", "card*/index.desktop", KStandardDirs::NoDuplicates );

        if ( list.isEmpty() ) return;

        for ( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it )
        {
            KConfig cfg( *it, KConfig::SimpleConfig );
            KConfigGroup cfgcg( &cfg, "KDE Backdeck" );
            QString path = ( *it ).left(( *it ).lastIndexOf( '/' ) + 1 );
            Q_ASSERT( path[path.length() - 1] == '/' );
            QPixmap pixmap( path + cfgcg.readEntry( "Preview", "12c.png" ) );
            if ( pixmap.isNull() ) continue;

            QString idx  = cfgcg.readEntryUntranslated( "Name", i18n( "unnamed" ) );
            QString name = cfgcg.readEntry( "Name", i18n( "unnamed" ) );
            KCardThemeInfo info;
            info.name         = name;
            info.noi18Name    = idx;
            info.comment      = cfgcg.readEntry( "Comment", i18n( "KDE card deck" ) );
            info.preview      = pixmap;
            info.path         = path;
            info.back         = cfgcg.readEntry( "Back", QString() );
            // The back name is read UNTRANSLATED...we need to find the right name for it now
            info.back         = findi18nBack( info.back );
            // if (!info.back.isNull()) kDebug() << "FOUND BACK " << info.back;
            info.size         = cfgcg.readEntry( "BackSize", QSizeF( pixmap.size() ) );
            info.isDefault    = cfgcg.readEntry( "Default", false );

            QString svg    = cfgcg.readEntry( "SVG", QString() );
            if ( !svg.isEmpty() )
            {
                QFileInfo svgInfo( QDir( path ), svg );
                info.svgfile = svgInfo.filePath();
                svgFrontInfo[name] = info;
            }
            else
            {
                info.svgfile = QString();
                pngFrontInfo[name] = info;
            }
        }

    }


    void readBacks()
    {
        // Empty data
        svgBackInfo.clear();
        pngBackInfo.clear();
    
        QStringList list = KGlobal::dirs()->findAllResources( "cards", "decks/*.desktop", KStandardDirs::NoDuplicates );

        if ( list.isEmpty() ) return;
    
        for ( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it )
        {
            KConfig cfg( *it, KConfig::SimpleConfig );
            QString path = ( *it ).left(( *it ).lastIndexOf( '/' ) + 1 );
            Q_ASSERT( path[path.length() - 1] == '/' );
            QPixmap pixmap( getBackFileNameFromIndex( *it ) );
            if ( pixmap.isNull() ) continue;
            //pixmap = pixmap.scaledToWidth(72, Qt::SmoothTransformation);
            QPixmap previewPixmap = pixmap.scaled( QSize( 32, 43 ), Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
    
            KConfigGroup cfgcg( &cfg, "KDE Cards" );
            QString idx  = cfgcg.readEntryUntranslated( "Name", i18n( "unnamed" ) );
            QString name = cfgcg.readEntry( "Name", i18n( "unnamed" ) );
            KCardThemeInfo info;
            info.name         = name;
            info.noi18Name    = idx;
            info.path         = getBackFileNameFromIndex( *it );
            info.comment      = cfgcg.readEntry( "Comment", i18n( "KDE card deck" ) );
            info.preview      = pixmap;
            info.size         = cfgcg.readEntry( "Size", QSizeF( pixmap.size() ) );
            info.isDefault    = cfgcg.readEntry( "Default", false );
    
            QString svg    = cfgcg.readEntry( "SVG", QString() );
            if ( !svg.isEmpty() )
            {
                QFileInfo svgInfo( QDir( path ), svg );
                info.svgfile = svgInfo.filePath();
                svgBackInfo[name] = info;
            }
            else
            {
                info.svgfile = QString();
                pngBackInfo[name] = info;
            }
        }
    
    }

    QString getBackFileNameFromIndex( const QString& desktop )
    {
        QString entry = desktop.left( desktop.length() - strlen( ".desktop" ) );
        if ( KStandardDirs::exists( entry + QString::fromLatin1( ".png" ) ) )
            return entry + QString::fromLatin1( ".png" );

        // rather theoretical
        if ( KStandardDirs::exists( entry + QString::fromLatin1( ".xpm" ) ) )
            return entry + QString::fromLatin1( ".xpm" );
        return QString();
    }


    /** The card front sides for PNG decks.
     */
    QMap<QString, KCardThemeInfo> pngFrontInfo;

    /** The card front sides for SVG decks.
     */
    QMap<QString, KCardThemeInfo> svgFrontInfo;

    /** The card back sides for PNG decks.
     */
    QMap<QString, KCardThemeInfo> pngBackInfo;

    /** The card back sides for SVG decks.
     */
    QMap<QString, KCardThemeInfo> svgBackInfo;

    /** The default front side name.
     */
    QString defaultFront;

    /** The default back side name.
     */
    QString defaultBack;
};

K_GLOBAL_STATIC( KCardThemeInfoStatic, deckinfoStatic )

namespace CardDeckInfo
{

// Retrieve default card set name
QString defaultFrontName( bool pAllowSVG, bool pAllowPNG )
{
    QString noDefault;
    // Count filtered cards
    QMap<QString, KCardThemeInfo> temp;
    if ( pAllowSVG )
    {
        temp.unite( deckinfoStatic->svgFrontInfo );
    }
    if ( pAllowPNG )
    {
        temp.unite( deckinfoStatic->pngFrontInfo );
    }
    QMapIterator<QString, KCardThemeInfo> it( temp );
    while ( it.hasNext() )
    {
        it.next();
        KCardThemeInfo v = it.value();
        // Filter
        if ( v.isDefault ) return v.name;
        // Collect any deck if no default is stored
        noDefault = v.name;
    }
    if ( noDefault.isNull() ) kError() << "Could not find default card name";
    return noDefault;
}


// Retrieve default deck name
QString defaultBackName( bool pAllowSVG, bool pAllowPNG )
{
    QString noDefault;
    QMap<QString, KCardThemeInfo> temp;
    if ( pAllowSVG )
    {
        temp.unite( deckinfoStatic->svgBackInfo );
    }
    if ( pAllowPNG )
    {
        temp.unite( deckinfoStatic->pngBackInfo );
    }

    QMapIterator<QString, KCardThemeInfo> it( temp );
    while ( it.hasNext() )
    {
        it.next();
        KCardThemeInfo v = it.value();
        // Filter
        if ( v.isDefault ) return v.name;
        // Collect any deck if no default is stored
        noDefault = v.name;
    }
    if ( noDefault.isNull() ) kError() << "Could not find default deck name";
    return noDefault;
}


// Retrieve a random card name
QString randomFrontName( bool pAllowSVG, bool pAllowPNG )
{
    // Collect matching items
    QStringList list;
    if ( pAllowSVG )
    {
        list += deckinfoStatic->svgFrontInfo.keys();
    }
    if ( pAllowPNG )
    {
        list += deckinfoStatic->pngFrontInfo.keys();
    }

    // Draw random one
    int d = KRandom::random() % list.count();
    return list.at( d );
}


// Retrieve a random deck name
QString randomBackName( bool pAllowSVG, bool pAllowPNG )
{
    // Collect matching items
    QStringList list;

    if ( pAllowSVG )
    {
        list += deckinfoStatic->svgBackInfo.keys();
    }
    if ( pAllowPNG )
    {
        list += deckinfoStatic->pngBackInfo.keys();
    }

    // Draw random one
    int d = KRandom::random() % list.count();
    return list.at( d );
}


// Retrieve the PNG filename for a back side from its index.desktop filename
QString getBackFileNameFromIndex( const QString &desktop )
{
    QString entry = desktop.left( desktop.length() - strlen( ".desktop" ) );
    if ( KStandardDirs::exists( entry + QString::fromLatin1( ".png" ) ) )
        return entry + QString::fromLatin1( ".png" );

    // rather theoretical
    if ( KStandardDirs::exists( entry + QString::fromLatin1( ".xpm" ) ) )
        return entry + QString::fromLatin1( ".xpm" );
    return QString();
}



// Retrieve the SVG file belonging to the given card back deck.
QString backSVGFilePath( const QString& name )
{
    if ( !deckinfoStatic->svgBackInfo.contains( name ) ) return QString();
    KCardThemeInfo v = deckinfoStatic->svgBackInfo.value( name );
    return v.svgfile;
}


// Retrieve the SVG file belonging to the given card fronts.
QString frontSVGFilePath( const QString& name )
{
    if ( !deckinfoStatic->svgFrontInfo.contains( name ) ) return QString();
    KCardThemeInfo v = deckinfoStatic->svgFrontInfo.value( name );
    return v.svgfile;
}


// Retrieve the PNG file belonging to the given card back deck.
QString backFilename( const QString& name )
{
    if ( !deckinfoStatic->pngBackInfo.contains( name ) ) return QString();
    KCardThemeInfo v = deckinfoStatic->pngBackInfo.value( name );
    return v.path;
}


// Retrieve the directory belonging to the given card fronts.
QString frontDir( const QString& name )
{
    if ( !deckinfoStatic->pngFrontInfo.contains( name ) ) return QString();
    KCardThemeInfo v = deckinfoStatic->pngFrontInfo.value( name );
    return v.path;
}


// Check whether a card set is SVG
bool isSVGFront( const QString& name )
{
    return deckinfoStatic->svgFrontInfo.contains( name );
}


// Check whether a card deck is SVG
bool isSVGBack( const QString& name )
{
    return deckinfoStatic->svgBackInfo.contains( name );
}

QStringList frontNames()
{
    return ( deckinfoStatic->svgFrontInfo.keys() + deckinfoStatic->pngFrontInfo.keys() );
}

QStringList backNames()
{
    return ( deckinfoStatic->svgBackInfo.keys() + deckinfoStatic->pngBackInfo.keys() );
}

KCardThemeInfo frontInfo( const QString& name )
{
    if ( deckinfoStatic->svgFrontInfo.contains( name ) )
        return deckinfoStatic->svgFrontInfo.value( name );
    if ( deckinfoStatic->pngFrontInfo.contains( name ) )
        return deckinfoStatic->pngFrontInfo.value( name );
    return KCardThemeInfo();
}

KCardThemeInfo backInfo( const QString& name )
{
    if ( deckinfoStatic->svgFrontInfo.contains( name ) )
        return deckinfoStatic->svgBackInfo.value( name );
    if ( deckinfoStatic->pngBackInfo.contains( name ) )
        return deckinfoStatic->pngBackInfo.value( name );
    return KCardThemeInfo();
}

}

