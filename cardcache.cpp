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

#include "cardcache.h"
#include "cardcache_p.h"

#include <QMap>
#include <QPixmap>
#include <QPair>
#include <QImage>
#include <QMutexLocker>
#include <QPainter>
#include <QSizeF>
#include <QFileInfo>
#include <QDir>

#include <ksvgrenderer.h>
#include <kpixmapcache.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include "kcarddialog.h"

#include <kdebug.h>

#define DECKLIST_LENGTH 53

KCardInfo fullDeckList[DECKLIST_LENGTH] = {
    KCardInfo( KCardInfo::Club, KCardInfo::Ace ),
    KCardInfo( KCardInfo::Heart, KCardInfo::Ace ),
    KCardInfo( KCardInfo::Diamond, KCardInfo::Ace ),
    KCardInfo( KCardInfo::Spade, KCardInfo::Ace ),
    KCardInfo( KCardInfo::Club, KCardInfo::King ),
    KCardInfo( KCardInfo::Heart, KCardInfo::King ),
    KCardInfo( KCardInfo::Diamond, KCardInfo::King ),
    KCardInfo( KCardInfo::Spade, KCardInfo::King ),
    KCardInfo( KCardInfo::Club, KCardInfo::Queen ),
    KCardInfo( KCardInfo::Heart, KCardInfo::Queen ),
    KCardInfo( KCardInfo::Diamond, KCardInfo::Queen ),
    KCardInfo( KCardInfo::Spade, KCardInfo::Queen ),
    KCardInfo( KCardInfo::Club, KCardInfo::Jack ),
    KCardInfo( KCardInfo::Heart, KCardInfo::Jack ),
    KCardInfo( KCardInfo::Diamond, KCardInfo::Jack ),
    KCardInfo( KCardInfo::Spade, KCardInfo::Jack ),
    KCardInfo( KCardInfo::Club, KCardInfo::Ten ),
    KCardInfo( KCardInfo::Heart, KCardInfo::Ten ),
    KCardInfo( KCardInfo::Diamond, KCardInfo::Ten ),
    KCardInfo( KCardInfo::Spade, KCardInfo::Ten ),
    KCardInfo( KCardInfo::Club, KCardInfo::Nine ),
    KCardInfo( KCardInfo::Heart, KCardInfo::Nine ),
    KCardInfo( KCardInfo::Diamond, KCardInfo::Nine ),
    KCardInfo( KCardInfo::Spade, KCardInfo::Nine ),
    KCardInfo( KCardInfo::Club, KCardInfo::Eight ),
    KCardInfo( KCardInfo::Heart, KCardInfo::Eight ),
    KCardInfo( KCardInfo::Diamond, KCardInfo::Eight ),
    KCardInfo( KCardInfo::Spade, KCardInfo::Eight ),
    KCardInfo( KCardInfo::Club, KCardInfo::Seven ),
    KCardInfo( KCardInfo::Heart, KCardInfo::Seven ),
    KCardInfo( KCardInfo::Diamond, KCardInfo::Seven ),
    KCardInfo( KCardInfo::Spade, KCardInfo::Seven ),
    KCardInfo( KCardInfo::Club, KCardInfo::Six ),
    KCardInfo( KCardInfo::Heart, KCardInfo::Six ),
    KCardInfo( KCardInfo::Diamond, KCardInfo::Six ),
    KCardInfo( KCardInfo::Spade, KCardInfo::Six ),
    KCardInfo( KCardInfo::Club, KCardInfo::Five ),
    KCardInfo( KCardInfo::Heart, KCardInfo::Five ),
    KCardInfo( KCardInfo::Diamond, KCardInfo::Five ),
    KCardInfo( KCardInfo::Spade, KCardInfo::Five ),
    KCardInfo( KCardInfo::Club, KCardInfo::Four ),
    KCardInfo( KCardInfo::Heart, KCardInfo::Four ),
    KCardInfo( KCardInfo::Diamond, KCardInfo::Four ),
    KCardInfo( KCardInfo::Spade, KCardInfo::Four ),
    KCardInfo( KCardInfo::Club, KCardInfo::Three ),
    KCardInfo( KCardInfo::Heart, KCardInfo::Three ),
    KCardInfo( KCardInfo::Diamond, KCardInfo::Three ),
    KCardInfo( KCardInfo::Spade, KCardInfo::Three ),
    KCardInfo( KCardInfo::Club, KCardInfo::Two ),
    KCardInfo( KCardInfo::Heart, KCardInfo::Two ),
    KCardInfo( KCardInfo::Diamond, KCardInfo::Two ),
    KCardInfo( KCardInfo::Spade, KCardInfo::Two ),
    KCardInfo( KCardInfo::None, KCardInfo::Joker )
};

KCardInfo::KCardInfo( KCardInfo::Suit s, KCardInfo::Card c )
    : m_suit( s ), m_card( c )
{
}

KCardInfo::Card KCardInfo::card() const
{
    return m_card;
}

KCardInfo::Suit KCardInfo::suit() const
{
    return m_suit;
}

void KCardInfo::setCard( KCardInfo::Card c )
{
    m_card = c;
}

void KCardInfo::setSuit( KCardInfo::Suit s )
{
    m_suit = s;
}

bool KCardInfo::operator==( const KCardInfo& c ) const
{
    return ( c.card() == card() && c.suit() == suit() );
}

QString KCardInfo::pngName() const
{
    if( suit() == Club    && card() == Ace ) return "1.png";
    if( suit() == Spade   && card() == Ace ) return "2.png";
    if( suit() == Heart   && card() == Ace ) return "3.png";
    if( suit() == Diamond && card() == Ace ) return "4.png";
    if( suit() == Club    && card() == King ) return "5.png";
    if( suit() == Spade   && card() == King ) return "6.png";
    if( suit() == Heart   && card() == King ) return "7.png";
    if( suit() == Diamond && card() == King ) return "8.png";
    if( suit() == Club    && card() == Queen ) return "9.png";
    if( suit() == Spade   && card() == Queen ) return "10.png";
    if( suit() == Heart   && card() == Queen ) return "11.png";
    if( suit() == Diamond && card() == Queen ) return "12.png";
    if( suit() == Club    && card() == Jack ) return "13.png";
    if( suit() == Spade   && card() == Jack ) return "14.png";
    if( suit() == Heart   && card() == Jack ) return "15.png";
    if( suit() == Diamond && card() == Jack ) return "16.png";
    if( suit() == Club    && card() == Ten ) return "17.png";
    if( suit() == Spade   && card() == Ten ) return "18.png";
    if( suit() == Heart   && card() == Ten ) return "19.png";
    if( suit() == Diamond && card() == Ten ) return "20.png";
    if( suit() == Club    && card() == Nine ) return "21.png";
    if( suit() == Spade   && card() == Nine ) return "22.png";
    if( suit() == Heart   && card() == Nine ) return "23.png";
    if( suit() == Diamond && card() == Nine ) return "24.png";
    if( suit() == Club    && card() == Eight ) return "25.png";
    if( suit() == Spade   && card() == Eight ) return "26.png";
    if( suit() == Heart   && card() == Eight ) return "27.png";
    if( suit() == Diamond && card() == Eight ) return "28.png";
    if( suit() == Club    && card() == Seven ) return "29.png";
    if( suit() == Spade   && card() == Seven ) return "30.png";
    if( suit() == Heart   && card() == Seven ) return "31.png";
    if( suit() == Diamond && card() == Seven ) return "32.png";
    if( suit() == Club    && card() == Six ) return "33.png";
    if( suit() == Spade   && card() == Six ) return "34.png";
    if( suit() == Heart   && card() == Six ) return "35.png";
    if( suit() == Diamond && card() == Six ) return "36.png";
    if( suit() == Club    && card() == Five ) return "37.png";
    if( suit() == Spade   && card() == Five ) return "38.png";
    if( suit() == Heart   && card() == Five ) return "39.png";
    if( suit() == Diamond && card() == Five ) return "40.png";
    if( suit() == Club    && card() == Four ) return "41.png";
    if( suit() == Spade   && card() == Four ) return "42.png";
    if( suit() == Heart   && card() == Four ) return "43.png";
    if( suit() == Diamond && card() == Four ) return "44.png";
    if( suit() == Club    && card() == Three ) return "45.png";
    if( suit() == Spade   && card() == Three ) return "46.png";
    if( suit() == Heart   && card() == Three ) return "47.png";
    if( suit() == Diamond && card() == Three ) return "48.png";
    if( suit() == Club    && card() == Two ) return "49.png";
    if( suit() == Spade   && card() == Two ) return "50.png";
    if( suit() == Heart   && card() == Two ) return "51.png";
    if( suit() == Diamond && card() == Two ) return "52.png";
    return "";
}

QString KCardInfo::svgName() const
{
    QString s;
    if( card() == KCardInfo::Ace )
        s += "1_";
    if( card() == KCardInfo::King )
        s += "king_";
    if( card() == KCardInfo::Queen )
        s += "queen_";
    if( card() == KCardInfo::Jack )
        s += "jack_";
    if( card() == KCardInfo::Ten )
        s += "10_";
    if( card() == KCardInfo::Nine )
        s += "9_";
    if( card() == KCardInfo::Eight )
        s += "8_";
    if( card() == KCardInfo::Seven )
        s += "7_";
    if( card() == KCardInfo::Six )
        s += "6_";
    if( card() == KCardInfo::Five )
        s += "5_";
    if( card() == KCardInfo::Four )
        s += "4_";
    if( card() == KCardInfo::Three )
        s += "3_";
    if( card() == KCardInfo::Two )
        s += "2_";
    if( suit() == KCardInfo::Club )
        s += "club";
    if( suit() == KCardInfo::Spade )
        s += "spade";
    if( suit() == KCardInfo::Diamond )
        s += "diamond";
    if( suit() == KCardInfo::Heart )
        s += "heart";
    return s;
}

QPixmap doRender( const QString& element, KSvgRenderer* r, const QSize& s )
{
    QImage img = QImage( s, QImage::Format_ARGB32 );
    img.fill( qRgba( 0, 0, 255, 0 ) );
    QPainter p( &img );
    if( r->elementExists( element ) )
    {
        r->render( &p, element );
    }else
    {
        r->render( &p, "layer1" );
    }
    p.end();
    return QPixmap::fromImage( img );
}

QString keyForPixmap( const QString& theme, const QString& element, const QSize& s )
{
    return theme + "_" + element + "_" 
                  + QString::number( s.width() ) + "_" 
                  + QString::number( s.height() );
}

void KCardCachePrivate::ensureNonNullPixmap( QPixmap& pix )
{
    if( pix.isNull() )
    {
        pix = QPixmap( size );
        QPainter p(&pix);
        p.fillRect( QRect( 0,0, pix.width(), pix.height() ), QBrush( Qt::white ) );
        QPen pen = p.pen();
        pen.setWidth( 4 );
        pen.setColor( QColor( Qt::red ) );
        p.setPen( pen );
        p.drawLine( QPoint( 2,2 ), QPoint( pix.width()-2, pix.height()-2 ) );
        p.drawLine( QPoint( pix.width()-2,2 ), QPoint( 2, pix.height()-2 ) );
        p.end();
    }
}

QPixmap KCardCachePrivate::renderFrontSvg( const QString& element )
{
    QMutexLocker l( frontRendererMutex );
    return doRender( element, frontRenderer, size );
}

QPixmap KCardCachePrivate::renderBackSvg( const QString& element )
{
    QMutexLocker l( backRendererMutex );
    return doRender( element, backRenderer, size );
}

LoadThread::LoadThread( KCardCache::LoadInfos l_, KCardCachePrivate* d_ )
    : d( d_ ), infos( l_ ), killMutex( new QMutex )
{
}

void LoadThread::setSize( const QSize & s )
{
    size = s;
}

void LoadThread::setFrontTheme( const QString & frontTheme_ )
{
    frontTheme = frontTheme_;
}

void LoadThread::setBackTheme( const QString & backTheme_ )
{
    backTheme = backTheme_;
}

void LoadThread::kill()
{
    QMutexLocker l(killMutex);
    doKill = true;
}

void LoadThread::run()
{
    {
        QMutexLocker l( killMutex );
        if( doKill )
            return;
    }
    if( infos & KCardCache::LoadBackSide )
    {
        bool found = false;
        QString key = keyForPixmap( backTheme, "back", size );
        QPixmap pix;
        {
            QMutexLocker l( d->backcacheMutex );
            if( d->backcache && d->backcache->find( key, pix ) )
                found = true;
        }
        if( !found )
        {
            {
                QMutexLocker l( d->backRendererMutex );
                pix = doRender( "back", d->backRenderer, size );
            }
            {
                QMutexLocker l( d->backcacheMutex );
                if( d->backcache )
                    d->backcache->insert( keyForPixmap( backTheme, "back", size ), pix );
            }
        }
    }
    {
        QMutexLocker l( killMutex );
        if( doKill )
            return;
    }
    if( infos & KCardCache::LoadFrontSide )
    {
        int numCards = 53;
        if( infos & KCardCache::Load52Cards )
            numCards = 52;
        if( infos & KCardCache::Load32Cards )
            numCards = 32;
        for( int i = 0; i < numCards ; i++ )
        {
            QString element = fullDeckList[i].svgName();
            QString key = keyForPixmap( frontTheme, element, size );
            QPixmap pix;
            {
                QMutexLocker l( killMutex );
                if( doKill )
                    return;
            }
            {
                QMutexLocker l( d->frontcacheMutex );
                if( d->frontcache && d->frontcache->find( key, pix ) )
                    continue;
            }
            {
                QMutexLocker l( d->frontRendererMutex );
                pix = doRender( element, d->frontRenderer, size );
            }
            {
                QMutexLocker l( d->frontcacheMutex );
                QPixmap tmp;
                if( d->frontcache )
                    d->frontcache->insert( key, pix );
            }
        }
    }
}

KCardCache::KCardCache()
    : d( new KCardCachePrivate )
{
    d->frontcache = 0;
    d->backcache = 0;
    d->frontcacheMutex = new QMutex();
    d->backcacheMutex = new QMutex();
    d->frontRendererMutex = new QMutex();
    d->backRendererMutex = new QMutex();
    d->frontRenderer = 0;
    d->backRenderer = 0;
    d->loadThread = 0;
    
}

KCardCache::~KCardCache()
{
    if( d->loadThread && d->loadThread->isRunning() )
    {
        d->loadThread->kill();
    }
    if( d->loadThread )
        delete d->loadThread;
    delete d->frontcache;
    delete d->backcache;
    delete d->frontcacheMutex;
    delete d->backcacheMutex;
    delete d->frontRendererMutex;
    delete d->backRendererMutex;
    delete d->frontRenderer;
    delete d->backRenderer;
    delete d;
}

QPixmap KCardCache::backside( int variant ) const
{
    QPixmap pix;
    if( d->backTheme.isEmpty() || d->size.isEmpty() )
        return pix;
    QString element = "back";
    if( variant > 0 )
    {
        element += QString::number(variant);
    }
    QString key = keyForPixmap( d->backTheme, element, d->size );
    if( !KCardDialog::isSVGDeck( d->backTheme ) )
    {
        QMutexLocker l( d->backcacheMutex );
        if( d->backcache && ( !d->backcache->find( key, pix ) || pix.isNull() ) )
        {
            QMatrix matrix;
            QImage img;
            bool ret = img.load( KCardDialog::deckFilename( d->backTheme ), "PNG" );
            if( !ret )
                return QPixmap();
            matrix.scale( (qreal)d->size.width() / (qreal)img.width(),
                  (qreal)d->size.height() / (qreal)img.height() );
            pix = QPixmap::fromImage( img.transformed( matrix ) );
            d->backcache->insert( key, pix );
        }
    }else
    {
        QMutexLocker l( d->backcacheMutex );
        if( d->backcache && ( !d->backcache->find( key, pix ) || pix.isNull() ) )
        {
            pix = d->renderBackSvg( element );
            d->backcache->insert( key, pix );
        }
    }
    // Make sure we never return an invalid pixmap
    d->ensureNonNullPixmap( pix );
    return pix;
}

QPixmap KCardCache::frontside( const KCardInfo& info ) const
{
    QPixmap pix;
    if( d->frontTheme.isEmpty() || d->size.isEmpty() )
        return pix;
    QString key = keyForPixmap( d->frontTheme, info.svgName() , d->size );
    
    if( !KCardDialog::isSVGCard( d->frontTheme ) )
    {
        QMutexLocker l( d->frontcacheMutex );
        if( d->frontcache && ( !d->frontcache->find( key, pix ) || pix.isNull() ) )
        {
            QMatrix matrix;
            QImage img;
            QString filename = KCardDialog::cardDir( d->frontTheme )
                    + "/" + info.pngName();
            bool ret = img.load( filename, "PNG" );
            if( !ret )
                return QPixmap();
            matrix.scale( (qreal)d->size.width() / (qreal)img.width(),
                  (qreal)d->size.height() / (qreal)img.height() );
            pix = QPixmap::fromImage( img.transformed( matrix ) );
            d->frontcache->insert( key, pix );
        }
    }else
    {
        QMutexLocker l( d->frontcacheMutex );
        if( d->frontcache && ( !d->frontcache->find( key, pix ) || pix.isNull() ) )
        {
            pix = d->renderFrontSvg( info.svgName() );
            d->frontcache->insert( key, pix );
        }
    }
    // Make sure we never return an invalid pixmap
    d->ensureNonNullPixmap( pix );
    return pix;
}

void KCardCache::setSize( const QSize& s )
{
    if( s != d->size )
        d->size = s;
}

QSize KCardCache::size() const
{
    return d->size;
}

void KCardCache::setFrontTheme( const QString& theme )
{
    {
        QMutexLocker l( d->frontcacheMutex );
        delete d->frontcache;
        d->frontcache = new KPixmapCache( QString( "kdegames-cards_%1" ).arg( theme ) );
        d->frontcache->setUseQPixmapCache( true );
        QDateTime dt;
        if( KCardDialog::isSVGCard( theme ) ) 
        {
            dt = QFileInfo( KCardDialog::cardSVGFilePath( theme ) ).lastModified();

        } else
        {
            QDir carddir( KCardDialog::cardDir( theme ) );
            foreach( QFileInfo entry, carddir.entryInfoList( QStringList() << "*.png" ) )
            {
                if( dt.isNull() || dt < entry.lastModified() )
                {
                    dt = entry.lastModified();
                }
            }
        }
        if( d->frontcache->timestamp() < dt.toTime_t() )
        {
            d->frontcache->discard();
            d->frontcache->setTimestamp( dt.toTime_t() );
        }
    }
    {
        QMutexLocker l( d->frontRendererMutex );
        delete d->frontRenderer;
        d->frontRenderer = new KSvgRenderer( KCardDialog::cardSVGFilePath( theme ) );
    }
    d->frontTheme = theme;
}

QString KCardCache::frontTheme() const
{
    return d->frontTheme;
}

void KCardCache::setBackTheme( const QString& theme )
{
    {
        QMutexLocker l( d->backcacheMutex );
        delete d->backcache;
        d->backcache = new KPixmapCache( QString( "kdegames-cards_%1" ).arg( theme ) );
        d->backcache->setUseQPixmapCache( true );
        QDateTime dt;
        if( KCardDialog::isSVGDeck( theme ) )
        {
            dt = QFileInfo( KCardDialog::deckSVGFilePath( theme ) ).lastModified();

        } else
        {
            dt = QFileInfo( KCardDialog::deckFilename( theme ) ).lastModified();
        }
        if( d->backcache->timestamp() < dt.toTime_t() )
        {
            d->backcache->discard();
            d->backcache->setTimestamp( dt.toTime_t() );
        }
    }
    {
        QMutexLocker l( d->backRendererMutex );
        delete d->backRenderer;
        d->backRenderer = new KSvgRenderer( KCardDialog::deckSVGFilePath( theme ) );
    }
    d->backTheme = theme;
}

QString KCardCache::backTheme() const
{
    return d->backTheme;
}

void KCardCache::loadTheme( LoadInfos infos )
{
    if( d->loadThread->isRunning() )
    {
        d->loadThread->kill();
        d->loadThread->wait();
        delete d->loadThread;
    }
    d->loadThread = new LoadThread( infos, d );
    d->loadThread->setBackTheme( d->backTheme );
    d->loadThread->setFrontTheme( d->frontTheme );
    d->loadThread->setSize( d->size );
    d->loadThread->start();
}

QSizeF KCardCache::defaultFrontSize( const KCardInfo& info ) const
{
    QSizeF size;
    if( d->frontTheme.isEmpty() )
        return size;
    if( !KCardDialog::isSVGCard( d->frontTheme ) )
    {
        QImage img;
        if( img.load( KCardDialog::cardDir( d->frontTheme ) 
            + "/" + info.pngName(), "PNG" ) )
            size = img.size();
    }else
    {
        QMutexLocker( d->frontRendererMutex );
        size = d->frontRenderer->boundsOnElement( info.svgName() ).size();
    }
    return size;
}

QSizeF KCardCache::defaultBackSize( int variant ) const
{
    QSizeF size;
    if( d->backTheme.isEmpty() ) 
        return size;
    QString element = "back";
    if( variant > 0 )
    {
        element += QString::number(variant);
    }
    
    if( !KCardDialog::isSVGCard( d->backTheme ) )
    {
        QImage img;
        if( img.load( KCardDialog::deckFilename( d->backTheme ), "PNG" ) )
        {
            size = img.size();
        }
    }else
    {
        QMutexLocker( d->backRendererMutex );
        size = d->backRenderer->boundsOnElement( element ).size();
    }
    return size;
}

void KCardCache::invalidateFrontside()
{
    QMutexLocker l( d->frontcacheMutex );
    if( d->frontcache )
        d->frontcache->discard();
}

void KCardCache::invalidateBackside()
{
    QMutexLocker l( d->backcacheMutex );
    if( d->backcache )
        d->backcache->discard();
}

#include "cardcache_p.moc"
