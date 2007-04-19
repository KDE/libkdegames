/***************************************************************************
 *   Copyright (C) 2007 Mark A. Taff <kde@marktaff.com>                    *
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

#include "kgamesvgdigits.h"

#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QMap>
#include <QtCore/QTimer>
#include <QtCore/QString>
#include <QtGui/QPainter>

#include <KConfig>
#include <KGameSvgDocument>

/**
 * @brief A class holding private members for KGameSvgDigits
 *
 * @see KGameSvgDocument
 * @author Mark A. Taff \<kde@marktaff.com\>
 */
class KGameSvgDigitsPrivate
{
    public:

    /**
     * @brief Instantiates a KGameSvgDigitsPrivate object
     */
    KGameSvgDigitsPrivate() : m_scaleX(1), m_scaleY(1), m_skewX(0), m_skewY(0)
    {}

    ~KGameSvgDigitsPrivate()
    {}

    /**
     * @brief Renders the digit.
     * 
     * @param node The node to render
     * @param cacheID The id of the digit in the pixmap cache
     * @returns returns nothing.
     */
    void renderDigit(const QDomNode& node, const QString& cacheID);

    /**
     * @brief Swaps colors between normal foreground/background colors and highlight foreground/background colors
     *
     * @returns returns nothing.
     */
    void swapColors();

    /**
     * @brief Applies the digit bitmask
     *
     * @returns returns nothing.
     */
    void applyBitmask(QDomNodeList nodes);

    /**
     * @brief Applies an RGBA color to a node in the svg DOM tree
     *
     * @param node The DOM node to apply the color to
     * @param color The RGBA color to apply
     * @returns returns nothing.
     */
    void applyColor(QDomNode node, QColor color);

    /**
     * @brief Cache individual style digits
     * @returns returns nothing.
     */
    void cacheIndividualDigits();

    /**
     * @brief Cache seven and fourteen segment digits
     * @returns returns nothing.
     */
    void cacheSegmentedDigits();

    /**
     * @brief Helps render segmented digits
     *
     * @param element The svg element to process
     * @param map The digit map
     * @returns returns nothing.
     */
    void cacheSegmentedDigits_helper(const QString& element, QMap<QString, QString>& map);

    /**
     * @brief DOM of m_svgFile
     */
    KGameSvgDocument m_svgDOM;

    /**
     * @brief The svg file to load
     */
    QString m_svgFile;

    /**
     * @brief The .desktop file to load
     */
    QString m_desktopFile;

    /**
     * @brief The number of digits to the left of the decimal to display
     */
    int m_numberOfDigits;

    /**
     * @brief Background color of digit.
     */
    QColor m_backgroundColor;

    /**
     * @brief Foreground color of digit.
     */
    QColor m_foregroundColor;

    /**
     * @brief Second color for foreground, used to highlight or flash the foregound.
     */
    QColor m_foregroundHighlightColor;

    /**
     * @brief Second color for background, used to highlight or flash the background.
     */
    QColor m_backgroundHighlightColor;

    /**
     * @brief The factor to scale the X-axis of the svg element by
     */
    double m_scaleX;

    /**
     * @brief The factor to scale the Y-axis of the svg element by
     */
    double m_scaleY;

    /**
     * @brief Degrees, in radians, to horizontally skew the digits.
     */
    double m_skewX;

    /**
     * @brief Degrees, in radians, to horizontally skew the digits.
     */
    double m_skewY;

    /**
     * @brief Is the pixmap cache dirty?
     */
    bool m_pixmapCacheDirty;

    /**
     * @brief Level of the alpha channel for the "off" segments of the LED digit.
     *
     * Valid range is from 0-255
     */
    int m_ledOffSegmentAlphaLevel;

    /**
     * @brief The bitmask to render.
     *
     */
    ulong m_bitmask;

    /**
     * @brief Should this digit be rendered as an LED instead of an LCD?
     */
    int m_digitStyle;

    /**
     * @brief A hash of the digit bitmasks, read from the .desktop file
     */
    QHash<QString, ulong> m_bitmasks;

    /**
     * @brief The caching option
     */
    int m_cacheOption;

    /**
     * @brief The type of the digit, individual|seven|fourteen
     */
    int m_digitType;

    /**
     * @brief Is the digit highlighted
     */
    bool m_highlighted;

    /**
     * @brief The map of numeral digits to ids
     */
    QMap<QString, QString> m_numbersMap;

    /**
     * @brief The map of punctuation digits to ids
     */
    QMap<QString, QString> m_punctuationMap;

    /**
     * @brief The map of alpha digits to ids
     */
    QMap<QString, QString> m_alphaMap;

    /**
     * @brief The svg element's id
     */
    QString m_elementId;

    /**
     * @brief The svg renderer
     */
    KSvgRenderer m_svgRenderer;

    /**
     * @brief Collection of pixmaps of rendered digits
     */
    QHash<QString, QPixmap> m_digitsPixmapCache;

    /**
     * @brief The timer for flashing the display
     */
    QTimer *m_flashTimer_ptr;


}; // End KGameSvgDigitsPrivate definition


KGameSvgDigits::KGameSvgDigits(const QString& svgFile) : d(new KGameSvgDigitsPrivate)
{
    // Set default colors
    /// @todo get feedback from #kdegames@freenode about good default colors
    setForegroundColor(QColor::fromRgba(0xffffaa00));
    setBackgroundColor(QColor::fromRgba(0xff323232));
    setForegroundHighlightColor(QColor::fromRgba(0xffff0000));
    setBackgroundHighlightColor(QColor::fromRgba(0xffeeeeec));

    // Set default values
    setNumberOfDigits(5);
    setLedOffSegmentAlphaLevel(10);
    setDigitStyle(LedStyle);
    setCacheOption(CacheNumeralsOnly);

    loadTheme(svgFile);
}

KGameSvgDigits::KGameSvgDigits() : d(new KGameSvgDigitsPrivate)
{
    // Set default colors
    /// @todo get feedback from #kdegames@freenode about good default colors
    setForegroundColor(QColor::fromRgba(0xffffaa00));
    setBackgroundColor(QColor::fromRgba(0xff323232));
    setForegroundHighlightColor(QColor::fromRgba(0xffff0000));
    setBackgroundHighlightColor(QColor::fromRgba(0xffeeeeec));

    // Set default values
    setNumberOfDigits(5);
    setLedOffSegmentAlphaLevel(10);
    setDigitStyle(LedStyle);
    setCacheOption(CacheNumeralsOnly);
}

KGameSvgDigits::~KGameSvgDigits()
{
    delete d;
}

void KGameSvgDigits::loadTheme(const QString& svgFile)
{
    QFileInfo file;
    bool ok;

    d->m_svgFile = svgFile;
    d->m_desktopFile = svgFile;

    file = QFileInfo(d->m_desktopFile);
    d->m_desktopFile = file.absolutePath() + '/' + file.completeBaseName() + ".desktop";

    d->m_svgDOM.load(d->m_svgFile);

    // Get type of digit from .desktop file
    KConfig *config = new KConfig(d->m_desktopFile, KConfig::OnlyLocal);
    QMap<QString, QString> desktopEntry = config->entryMap("Desktop Entry");
    setDigitType(desktopEntry.value("Type"));

    d->m_numbersMap = config->entryMap("Digits");
    d->m_punctuationMap = config->entryMap("Punctuation");
    d->m_alphaMap = config->entryMap("Alpha");

    // Parse the artist-friendly map strings into bitmasks.
    if (digitType() != IndividualDigit)
    {
        // Numbers
        QMapIterator<QString, QString> it(d->m_numbersMap);
        while (it.hasNext())
        {
            it.next();
            d->m_bitmasks.insert(it.key(), toBitmask(it.value()));
        }

        // Punctuation
        it = d->m_punctuationMap;
        while (it.hasNext())
        {
            it.next();
            d->m_bitmasks.insert(it.key(), toBitmask(it.value()));
        }

        // Alpha
        it = d->m_alphaMap;
        while (it.hasNext())
        {
            it.next();
            d->m_bitmasks.insert(it.key(), toBitmask(it.value()));
        }
    }

    // Override any default values with values from theme
    QMap<QString, QString> settings = config->entryMap("Settings");

    if (!settings.value("foregroundColor").isEmpty())
        {setForegroundColor(QColor::fromRgba(settings.value("foregroundColor").toLong(&ok, 16)));}
    if (!settings.value("backgroundColor").isEmpty())
        {setBackgroundColor(QColor::fromRgba(settings.value("backgroundColor").toLong(&ok, 16)));}
    if (!settings.value("foregroundHighlightColor").isEmpty())
        {setForegroundHighlightColor(QColor::fromRgba(settings.value("foregroundHighlightColor").toLong(&ok, 16)));}
    if (!settings.value("backgroundHighlightColor").isEmpty())
         {setBackgroundHighlightColor(QColor::fromRgba(settings.value("backgroundHighlightColor").toLong(&ok, 16)));}

    if (!settings.value("ledOffSegmentAlphaLevel").isEmpty())
        { setLedOffSegmentAlphaLevel(settings.value("ledOffSegmentAlphaLevel").toInt()); }
    if (!settings.value("skewX").isEmpty()) {setSkewX(settings.value("skewX").toInt()); }
    if (!settings.value("skewY").isEmpty()) {setSkewY(settings.value("skewY").toInt());}
    if (!settings.value("scaleX").isEmpty()) {setScaleX(settings.value("scaleX").toInt());}
    if (!settings.value("scaleY").isEmpty()) {setScaleY(settings.value("scaleY").toInt());}
    if (!settings.value("digitStyle").isEmpty()) {setDigitStyle(settings.value("digitStyle"));}
    if (!settings.value("cacheOption").isEmpty()) {setCacheOption(settings.value("cacheOption"));}

    kDebug () << "Cache option set to: " << settings.value("cacheOption") << cacheOption() << endl;

    d->m_highlighted = false;

    kDebug () << "Theme set to: " << desktopEntry.value("Name") << endl;
}

QPixmap KGameSvgDigits::display(const QString& display)
{
    QString character, str;
//  KGameSvgDigit digit;
    int i;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    while (numberOfDigits() < display.size())
    {
        kDebug () << "number of digits is set to " << numberOfDigits() << " but you tried to display " << display.size() << endl;
        setNumberOfDigits(numberOfDigits() + 1);
    }

    int t_width = static_cast<int> (d->m_digitsPixmapCache.value("8").width() * numberOfDigits() * 1.1);
    int t_height = static_cast<int> (d->m_digitsPixmapCache.value("8").height() * numberOfDigits() * 1.1);
    QPixmap finalPixmap = QPixmap(t_width, t_height);

    QPainter painter(&finalPixmap);

    kDebug () << "about to display: " << display << endl;
    str = display;

    for (i=0; i<display.size(); i++)
    {
        character = str.left(1);
        str.remove(0,1);

        if (d->m_highlighted)
        {
            if (character == " ") {character = "blank.highlight";}
            else {character = character + ".highlight";}
        }
        else
        {
            character = character;
            if (character == " ") {character = "blank";}
        }

        if (d->m_digitsPixmapCache.value(character).isNull())
        {
            kDebug () << "couldn't find pixmap for character '" << character << "'. Skipping." << endl;
            continue;
        }

        x = width + x;
        width = static_cast<int> (d->m_digitsPixmapCache.value(character).width());
        height = static_cast<int> (d->m_digitsPixmapCache.value(character).height());

        painter.drawPixmap(x, y, d->m_digitsPixmapCache.value(character));

//      kDebug () << "set digit: " << character << endl;
    }
    return finalPixmap.copy(0, 0, width + x, height);
}

void KGameSvgDigits::flash(int interval)
{
    d->m_flashTimer_ptr = new QTimer();
    connect(d->m_flashTimer_ptr, SIGNAL(timeout()), this, SLOT(updateFlash()));
    d->m_flashTimer_ptr->start(interval);
}

void KGameSvgDigits::flash()
{
    int interval = 800;
    flash(interval);
}

void KGameSvgDigits::stopFlashing()
{
    d->m_flashTimer_ptr->stop();
    if (d->m_highlighted)
    {
        d->swapColors();
    }
    emit signalDisplayDirty();
}

void KGameSvgDigits::highlight()
{
    d->swapColors();
    emit signalDisplayDirty();
}

ulong KGameSvgDigits::toBitmask(QString string)
{
    ulong tmpMask = 0;
    // Parse the string to give us an ulong
    for (int j = 0; j < string.count(); ++j)
    {
        if (string.data()[j] == '1')
        {
            tmpMask |= (1 << j);
            //qDebug () << "bit is on" << flag;
        }
    }
    return tmpMask;
}

void KGameSvgDigits::refreshCache()
{
    d->m_digitsPixmapCache.clear();
    kDebug () << "cache has been cleared" << endl;

    kDebug () << cacheOption() << endl;
    if (cacheOption() == CacheNone){return;}
    if (cacheOption() == CachePreviouslyRendered){return;}

    if (cacheOption() == CacheNumeralsOnly)
    {
        // cache blank, 0-9, punctuation
        switch (digitType())
        {
            case IndividualDigit:
                d->cacheIndividualDigits();

                break;

            case SevenSegmentDigit:
                d->cacheSegmentedDigits();

                break;

            case FourteenSegmentDigit:
                d->cacheSegmentedDigits();

                break;
        }
    }

    kDebug () << "cache has been rebuilt" << endl;
}

/*
 * Setters and Accessors
 */

void KGameSvgDigits::setPixmapCacheDirty(bool isDirty)
{
    d->m_pixmapCacheDirty = isDirty;
}

bool KGameSvgDigits::isPixmapCacheDirty()
{
    return d->m_pixmapCacheDirty;
}

void KGameSvgDigits::setCacheOption(CacheOptions option)
{
    d->m_cacheOption = option;
}

void KGameSvgDigits::setCacheOption(const QString& option)
{
    if (option.toLower() == "cacheall")
    {
        setCacheOption(CacheAll);
    }
    else if (option.toLower() == "cachenone")
    {
        setCacheOption(CacheNone);
    }
    else if (option.toLower() == "cachenumeralsonly")
    {
        setCacheOption(CacheNumeralsOnly);
    }
    else if (option.toLower() == "cachepreviouslyrendered")
    {
        setCacheOption(CachePreviouslyRendered);
    }
}

int KGameSvgDigits::cacheOption()
{
    return d->m_cacheOption;
}

void KGameSvgDigits::setDigitType(DigitTypes type)
{
    d->m_digitType = type;
}

void KGameSvgDigits::setDigitType(const QString& type)
{
    if (type == "IndividualDigit")
    {
        d->m_digitType = IndividualDigit;
    }
    else if (type == "SevenSegmentDigit")
    {
        d->m_digitType = SevenSegmentDigit;
    }
    else if (type == "FourteenSegmentDigit")
    {
        d->m_digitType = FourteenSegmentDigit;
    }
}

int KGameSvgDigits::digitType()
{
    return d->m_digitType;
}

void KGameSvgDigits::setDigitStyle(const DigitStyle& style)
{
    d->m_digitStyle = style;
    d->m_pixmapCacheDirty = true;
}

void KGameSvgDigits::setDigitStyle(const QString& style)
{
    if (style == "LedStyle")
    {
        setDigitStyle(LedStyle);
    }
    else if (style == "LcdStyle")
    {
        setDigitStyle(LedStyle);
    }
}

int KGameSvgDigits::digitStyle()
{
    return d->m_digitStyle;
}

void KGameSvgDigits::setNumberOfDigits(int numberOfDigits)
{
    d->m_numberOfDigits = numberOfDigits;
}

int KGameSvgDigits::numberOfDigits()
{
    return d->m_numberOfDigits;
}

void KGameSvgDigits::setForegroundColor(const QColor& foregroundColor)
{
    d->m_foregroundColor = foregroundColor;
    d->m_pixmapCacheDirty = true;
}

QColor KGameSvgDigits::foregroundColor()
{
    return d->m_foregroundColor;
}

void KGameSvgDigits::setBackgroundColor(const QColor& backgroundColor)
{
    d->m_backgroundColor = backgroundColor;
    d->m_pixmapCacheDirty = true;
}

QColor KGameSvgDigits::backgroundColor()
{
    return d->m_backgroundColor;
}

void KGameSvgDigits::setForegroundHighlightColor(const QColor& foregroundHighlightColor)
{
    d->m_foregroundHighlightColor = foregroundHighlightColor;
    d->m_pixmapCacheDirty = true;
}

QColor KGameSvgDigits::foregroundHighlightColor()
{
    return d->m_foregroundHighlightColor;
}

void KGameSvgDigits::setBackgroundHighlightColor(const QColor& backgroundHighlightColor)
{
    d->m_backgroundHighlightColor = backgroundHighlightColor;
    d->m_pixmapCacheDirty = true;
}

QColor KGameSvgDigits::backgroundHighlightColor()
{
    return d->m_backgroundHighlightColor;
}

void KGameSvgDigits::setScaleX(double scale)
{
    d->m_scaleX = scale;
    d->m_pixmapCacheDirty = true;
}

double KGameSvgDigits::scaleX()
{
    return d->m_scaleX;
}

void KGameSvgDigits::setScaleY(double scale)
{
    d->m_scaleY = scale;
    d->m_pixmapCacheDirty = true;
}

double KGameSvgDigits::scaleY()
{
    return d->m_scaleY;
}

void KGameSvgDigits::setSkewX(double skew)
{
    d->m_skewX = skew;
    d->m_pixmapCacheDirty = true;
}

double KGameSvgDigits::skewX()
{
    return d->m_skewX;
}

void KGameSvgDigits::setSkewY(double skew)
{
    d->m_skewY = skew;
    d->m_pixmapCacheDirty = true;
}

double KGameSvgDigits::skewY()
{
    return d->m_skewY;
}

void KGameSvgDigits::setLedOffSegmentAlphaLevel(int alphaLevel)
{
    d->m_ledOffSegmentAlphaLevel = alphaLevel;
    d->m_pixmapCacheDirty = true;
}

int KGameSvgDigits::ledOffSegmentAlphaLevel()
{
    return d->m_ledOffSegmentAlphaLevel;
}

void KGameSvgDigits::setBitmask(ulong mask)
{
    d->m_bitmask = mask;
}

ulong KGameSvgDigits::bitmask()
{
    return d->m_bitmask;
}

void KGameSvgDigits::setElementId(const QString& elementId)
{
    d->m_elementId = elementId;
}

QString KGameSvgDigits::elementId()
{
    return d->m_elementId;
}

//
// SLOTS
//

void KGameSvgDigits::updateFlash()
{
    d->m_highlighted = !d->m_highlighted;
    kDebug () << "flashing " << d->m_highlighted << endl;
    emit signalDisplayDirty();
}


//
// KGameSvgDigitsPrivate definition
//

void KGameSvgDigitsPrivate::renderDigit(const QDomNode& node, const QString& cacheID)
{
    QByteArray svg;
    QPixmap pixmap;

    m_svgDOM.setCurrentNode(node);

    // set scale, set skew
    m_svgDOM.skew(m_skewX, m_skewY, KGameSvgDocument::ReplaceCurrentMatrix);
    m_svgDOM.scale(m_scaleX, m_scaleY, KGameSvgDocument::ApplyToCurrentMatrix);

    svg = m_svgDOM.nodeToByteArray();
    m_svgRenderer.load(svg);

    double width = m_svgRenderer.boundsOnElement(m_elementId).width();
    double height = m_svgRenderer.boundsOnElement(m_elementId).height();

    int x = qRound(width);
    int y = qRound(height);

    int translateX = 0;
    int translateY = 0;

    QRectF viewport = QRectF(translateX, translateY, x, y);

    pixmap = QPixmap((x + translateX), (y + translateY));
    pixmap.fill(m_backgroundColor);

    QPainter painter (&pixmap);

    m_svgRenderer.render(&painter, m_elementId, viewport);

    if (m_cacheOption != KGameSvgDigits::CacheNone)
    {
        m_digitsPixmapCache.insert(cacheID, pixmap);
        kDebug () << "caching digit: " << cacheID << endl;
    }

}

void KGameSvgDigitsPrivate::cacheIndividualDigits()
{
    QDomNode containerElement;
    QDomNode backgroundElement;
    QDomNode faceElement;
    QString s;
    QString id;
    QString colorStyle = "";
    QMap<QString, QString> elementsToRender = m_numbersMap;

    if (m_cacheOption == KGameSvgDigits::CacheNumeralsOnly) {elementsToRender.unite(m_punctuationMap);}
    if (m_cacheOption == KGameSvgDigits::CacheAll) {elementsToRender.unite(m_alphaMap);}

    QMapIterator<QString, QString> it(elementsToRender);

    for (int i=0; i<2; i++)
    {
        if (i == 1)
        {
            colorStyle = ".highlight";
        }
        while (it.hasNext())
        {
            it.next();

            id = it.key();
            m_elementId = it.value();
            containerElement = m_svgDOM.elementById(m_elementId);
            faceElement = m_svgDOM.elementById(m_elementId + "Foreground");

//          kDebug () << "elementId: " << m_elementId << endl;

            if(!faceElement.isNull() && (!m_elementId.contains("blank")) )
            {
                applyColor(faceElement, m_foregroundColor);
            }

            id += colorStyle;
//          kDebug () << "elementId: " << d->m_elementId << " id: " << id << endl;
            renderDigit(containerElement, id);
        }
        swapColors();
        it.toFront();
    }

}

void KGameSvgDigitsPrivate::cacheSegmentedDigits_helper(const QString& element, QMap<QString, QString>& map)
{
    QDomNode containerElement;
    QDomNode backgroundElement;
    QDomNode faceElement;
    QString s;
    QString id;
    QString colorStyle = "";

    QDomNodeList faceElementPaths;
    QMapIterator<QString, QString> it(map);

    m_elementId = element;
    containerElement = m_svgDOM.elementById(m_elementId);
    faceElement = QDomNode(m_svgDOM.elementById(m_elementId + "Face"));

    faceElementPaths = faceElement.childNodes();

    for (int i=0; i<2; i++)
    {
        if (i == 1)
        {
            colorStyle = ".highlight";
        }
        while (it.hasNext())
        {
            it.next();

            id = it.key();
            m_bitmask = m_bitmasks.value(it.key());
            applyBitmask(faceElementPaths);

            id += colorStyle;
            renderDigit(containerElement, id);
        }

        swapColors();
        it.toFront();
        colorStyle = "";
    }
}

void KGameSvgDigitsPrivate::cacheSegmentedDigits()
{
    cacheSegmentedDigits_helper(QString("punctuation"), m_punctuationMap);
    cacheSegmentedDigits_helper(QString("digit"), m_numbersMap);
    cacheSegmentedDigits_helper(QString("digit"), m_alphaMap);
}

void KGameSvgDigitsPrivate::swapColors()
{

    QColor fg_temp, bg_temp;
    fg_temp = m_foregroundColor;
    bg_temp = m_backgroundColor;

    m_foregroundColor = m_foregroundHighlightColor;
    m_backgroundColor = m_backgroundHighlightColor;

    m_foregroundHighlightColor = fg_temp;
    m_backgroundHighlightColor = bg_temp;

    m_highlighted = !m_highlighted;
}

void KGameSvgDigitsPrivate::applyColor(QDomNode node, QColor color)
{
    /** In SVG, the alpha channel is spec'd in the fill-opacity property, not
     * in the fill property.  SVG fill-opacity must be spec'd as a number from 0 to 1
     */
    QString svgFillOpacity = QString::number(color.alpha() * 0.00392156862745); // multiply by 1/255
    QString svgFill = color.name();

//      kDebug () << "Fill: " << svgFill << " Opacity: "<< svgFillOpacity << endl;
    m_svgDOM.setCurrentNode(node);
//  qDebug () << "Style before:" << m_svgDOM.style();

    m_svgDOM.setStyleProperty("fill", svgFill);
    m_svgDOM.setStyleProperty("fill-opacity", svgFillOpacity);

//  qDebug () << " Style after:" << m_svgDOM.style() << endl;
}

void KGameSvgDigitsPrivate::applyBitmask(QDomNodeList nodes)
{
    bool ok;
    int hex;
    QColor ledOffColor;
    QDomElement element;
    QString id;

    ledOffColor = m_foregroundColor;
    ledOffColor.setAlpha(m_ledOffSegmentAlphaLevel);

    for (int j = 0; j < nodes.count(); ++j)
    {
        element = nodes.item(j).toElement();
        id = element.attribute( "id", "not found");

        hex = id.toInt(&ok, 16);

        if ((m_bitmask & (1 << hex)) != 0)
        {
//          kDebug () << "segment on" << endl;
            applyColor(nodes.item(j), m_foregroundColor);
        }
        else
        {
//          kDebug () << "off" << endl;
            if (m_digitStyle == KGameSvgDigits::LedStyle)
            {
                applyColor(nodes.item(j), ledOffColor);
            }
            else
            {
                applyColor(nodes.item(j), m_backgroundColor);
            }
        }
    }
}

#include "kgamesvgdigits.moc"
