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

#include <QtCore/QFileInfo>
#include <QtCore/QMap>
#include <QtCore/QTimer>
#include <QtCore/QString>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include <KConfig>
#include <KDebug>
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
    KGameSvgDigitsPrivate() : m_scaleX(1), m_scaleY(1), m_skewX(0), m_skewY(0), m_letterSpacing(0),
                              m_paddingTop(0), m_paddingRight(0), m_paddingBottom(0), m_paddingLeft(0)
    {}

    ~KGameSvgDigitsPrivate()
    {}

    /**
     * @brief Renders the digit.
     *
     * @param node The node to render
     * @param cacheId The id of the digit in the pixmap cache
     * @returns returns the pixmap
     */
    QPixmap renderDigit(const QDomNode& node, const QString& cacheId);

    /**
     * @brief Renders the digit.
     *
     * @param element The node to render
     * @param cacheId The id of the digit in the pixmap cache
     * @returns returns nothing.
     */
    QPixmap renderSegmentedDigit(const QString& element, const QString& cacheId);

    /**
     * @brief Swaps colors between normal foreground/background colors and highlight foreground/background colors
     *
     * @returns returns nothing.
     */
    void swapColors();

    /**
     * @brief Returns the name of the cacheOption value
     *
     * @param option A cacheOption enum value
     * @returns returns the name of the option
     */
    QString lookupCacheOptionName(const int option);

    /**
     * @brief Returns the name of the digit style
     *
     * @param style A digitStyle enum value
     * @returns returns the name of the digit style
     */
    QString lookupDigitStyleName(const int style);

    /**
     * @brief Applies the digit bitmask
     *
     * @param nodes A list of segment nodes to turn on or off
     * @returns returns nothing.
     */
    void applyBitmask(const QDomNodeList& nodes);

    /**
     * @brief Applies an RGBA color to a node in the svg DOM tree
     *
     * @param node The DOM node to apply the color to
     * @param color The RGBA color to apply
     * @returns returns nothing.
     */
    void applyColor(const QDomNode &node, const QColor &color);

    /**
     * @brief Renders individual style digits
     * @returns returns nothing.
     */
    void renderIndividualDigits();

    /**
     * @brief Renders an individual style digit
     *
     * @param element The svg element to process
     * @param cacheId The id of the digit in the pixmap cache
     * @returns returns nothing.
     */
    QPixmap renderIndividualDigit(const QString& element, const QString& cacheId);

    /**
     * @brief Renders segmented digits
     *
     * @param element The svg element to process
     * @param map The digit map
     * @returns returns nothing.
     */
    void renderSegmentedDigits(const QString& element, const QMap<QString, QString>& map);

    /**
     * @brief Convert a string binary representation to convert to a bitmask
     *
     * @param string The string binary representation, e.g. "01001111", to convert to a bitmask
     * @returns returns the bitmask
     */
    ulong toBitmask(const QString& string);

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
     * @brief The width of a blank digit
     */
    int m_widthHint;

    /**
     * @brief The height of a blank digit
     */
    int m_heightHint;

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
     * @brief The map of all characters digits to ids
     */
    QMap<QString, QString> m_characterMap;

    /**
     * @brief The svg element's id
     */
    QString m_elementId;

    /**
     * @brief The svg renderer
     */
    QSvgRenderer m_svgRenderer;

    /**
     * @brief Collection of pixmaps of rendered digits
     */
    QHash<QString, QPixmap> m_digitsPixmapCache;

    /**
     * @brief The timer for flashing the display
     */
    QTimer *m_flashTimer_ptr;

    /**
     * @brief The horizontal spacing in pixels between the digits
     */
    int m_letterSpacing;

    /**
     * @brief The padding in pixels between the top of the digits and the top of the display
     */
    int m_paddingTop;

    /**
     * @brief The padding in pixels between the right-most digit and the right edge of the display
     */
    int m_paddingRight;

    /**
     * @brief The padding in pixels between the bottom of the digits and the botom of the display
     */
    int m_paddingBottom;

    /**
     * @brief The padding in pixels between the leftmost digit and the left edge of the display
     */
    int m_paddingLeft;

}; // End KGameSvgDigitsPrivate definition


KGameSvgDigits::KGameSvgDigits(const QString& themeFile) : d(new KGameSvgDigitsPrivate)
{
    // Set default colors
    /// @todo get feedback from other devs about good default colors
    setForegroundColor(QColor::fromRgba(0xffffaa00));
    setBackgroundColor(QColor::fromRgba(0xff323232));
    setForegroundHighlightColor(QColor::fromRgba(0xffff0000));
    setBackgroundHighlightColor(QColor::fromRgba(0xffeeeeec));

    // Set default values
    setNumberOfDigits(5);
    setLedOffSegmentAlphaLevel(10);
    setDigitStyle(LedStyle);
    setCacheOption(CacheNumeralsOnly);

    loadTheme(themeFile);
}

KGameSvgDigits::KGameSvgDigits() : d(new KGameSvgDigitsPrivate)
{
    // Set default colors
    /// @todo get feedback from other devs about good default colors
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

void KGameSvgDigits::loadTheme(const QString& themeFile)
{
    QFileInfo file;
    bool ok;

    d->m_svgFile = themeFile;
    d->m_desktopFile = themeFile;

    file = QFileInfo(d->m_svgFile);
    d->m_svgFile = file.absolutePath() + '/' + file.completeBaseName() + ".svg";
    /// @todo Handle *.svgz files as well

    d->m_svgDOM.load(d->m_svgFile);

    // Get type of digit from .desktop file
    KConfig *config = new KConfig(d->m_desktopFile, KConfig::SimpleConfig);
    QMap<QString, QString> desktopEntry = config->entryMap("Desktop Entry");
    setDigitType(desktopEntry.value("Type"));

    d->m_numbersMap = config->entryMap("Digits");
    d->m_punctuationMap = config->entryMap("Punctuation");
    d->m_alphaMap = config->entryMap("Alpha");

    // Parse the artist-friendly map strings into bitmasks.
    if (digitType() != IndividualDigit)
    {
        QMapIterator<QString, QString> it(d->m_numbersMap);
        while (it.hasNext())
        {
            it.next();
            d->m_bitmasks.insert(it.key(), d->toBitmask(it.value()));
        }
        it = d->m_punctuationMap;
        while (it.hasNext())
        {
            it.next();
            d->m_bitmasks.insert(it.key(), d->toBitmask(it.value()));
        }
        it = d->m_alphaMap;
        while (it.hasNext())
        {
            it.next();
            d->m_bitmasks.insert(it.key(), d->toBitmask(it.value()));
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

    if (!settings.value("letter-spacing").isEmpty()) {setLetterSpacing(settings.value("letter-spacing").toInt());}
    if (!settings.value("padding-top").isEmpty()) {setPaddingTop(settings.value("padding-top").toInt());}
    if (!settings.value("padding-right").isEmpty()) {setPaddingRight(settings.value("padding-right").toInt());}
    if (!settings.value("padding-bottom").isEmpty()) {setPaddingBottom(settings.value("padding-bottom").toInt());}
    if (!settings.value("padding-left").isEmpty()) {setPaddingLeft(settings.value("padding-left").toInt());}

    if (!settings.value("ledOffSegmentAlphaLevel").isEmpty())
        { setLedOffSegmentAlphaLevel(settings.value("ledOffSegmentAlphaLevel").toInt()); }
    if (!settings.value("skewX").isEmpty()) {setSkewX(settings.value("skewX").toDouble()); }
    if (!settings.value("skewY").isEmpty()) {setSkewY(settings.value("skewY").toDouble());}
    if (!settings.value("scaleX").isEmpty()) {setScaleX(settings.value("scaleX").toDouble());}
    if (!settings.value("scaleY").isEmpty()) {setScaleY(settings.value("scaleY").toDouble());}
    if (!settings.value("digitStyle").isEmpty()) {setDigitStyle(settings.value("digitStyle"));}
    if (!settings.value("cacheOption").isEmpty()) {setCacheOption(settings.value("cacheOption"));}

    d->m_highlighted = false;
    delete config;
    kDebug () << "Theme set to:" << desktopEntry.value("Name");
}

QPixmap KGameSvgDigits::display(const QString& display)
{
    QString characterToDisplay, str, cacheId;
    QChar currentCharacter;

    int i;
    int x = paddingLeft();
    int y = paddingTop();
    int width = 0;
    int height = 0;

    if (isPixmapCacheDirty())
    {
        refreshCache();
    }

    str = display;

    // If display string is empty, treat it as numberOfDigits() of "blank" digits
    if (str.isEmpty())
    {
        str.fill(' ', numberOfDigits());
    }

    if (numberOfDigits() != str.size())
    {
        kDebug () << "number of digits is set to" << numberOfDigits() << "but you tried to display" << str.size();
        setNumberOfDigits(str.size());
    }

    int t_width = static_cast<int> ((d->m_widthHint + letterSpacing()) * numberOfDigits() * 1.1);
    t_width += paddingLeft() + paddingRight();
    int t_height = static_cast<int> (d->m_heightHint * 1.1);
    t_height += paddingTop() + paddingBottom();

    QPixmap finalPixmap = QPixmap(t_width, t_height);
    finalPixmap.fill(d->m_backgroundColor);

    QPainter painter(&finalPixmap);

    kDebug () << "about to display: '" << str << "'";

    for (i=0; i<numberOfDigits(); ++i)
    {
        currentCharacter = str[0];
        characterToDisplay = str.left(1);
        str.remove(0,1);

        if (currentCharacter.isSpace()) {characterToDisplay = "blank";}

        if (d->m_highlighted)
        {
            cacheId = characterToDisplay + ".highlight";
        }
        else
        {
            cacheId = characterToDisplay;
        }

        QPixmap tmp_pixmap;

        // Generally, if it is cached, use it from the cache
        if (!d->m_digitsPixmapCache.value(cacheId).isNull())
        {
            tmp_pixmap = d->m_digitsPixmapCache.value(cacheId);
        }
        else if ((cacheOption() == CacheNone) || (cacheOption() == CachePreviouslyRendered))
        {
            // Digit not cached, so render the digit
            if (digitType() == IndividualDigit)
            {
                if (d->m_characterMap.contains(characterToDisplay))
                {
                    QString element = d->m_characterMap.value(characterToDisplay);
                    tmp_pixmap.QPixmap::operator=(d->renderIndividualDigit(element, cacheId));
                }
                else
                {
                    tmp_pixmap.QPixmap::operator=(QPixmap());
                }
            }
            else
            {
                if (d->m_punctuationMap.contains(characterToDisplay))
                {
                    tmp_pixmap.QPixmap::operator=(d->renderSegmentedDigit("punctuation", cacheId));
                }
                else if ((d->m_numbersMap.contains(characterToDisplay)) || (d->m_alphaMap.contains(characterToDisplay)))
                {
                    tmp_pixmap.QPixmap::operator=(d->renderSegmentedDigit("digit", cacheId));
                }
                else
                {
                    tmp_pixmap.QPixmap::operator=(QPixmap());
                }
            }
        }

        // Draw digit on final pixmap
        if (!tmp_pixmap.isNull())
        {
            x += width;

            // Add letter-spacing between digits
            if ((i > 0) && (i < numberOfDigits())) {x += letterSpacing();}
            width = static_cast<int> (tmp_pixmap.width());
            height = static_cast<int> (tmp_pixmap.height());

            painter.drawPixmap(x, y, tmp_pixmap);
        }
        else
        {
            kDebug () << "Couldn't find cached pixmap, or SVG element to render, for the character: "
                      << characterToDisplay << "Skipping.";
        }
    }
    painter.end();
    return finalPixmap.copy(0, 0, width + x + paddingRight(), height + paddingTop() + paddingBottom());
//     return finalPixmap.copy(0, 0, width + x, height);
}

void KGameSvgDigits::flash(int interval)
{
    if ((cacheOption() == CacheNone) && (interval < 10000))
    {
        interval = 10000;
        kDebug () << "Caching is disabled, and the flashing interval is less than 10 seconds.";
        kDebug () << "To limit use of CPU, I have reset the interval to 10 seconds.";
    }
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

void KGameSvgDigits::refreshCache()
{
    d->m_digitsPixmapCache.clear();
    kDebug () << "cache has been cleared";

    // Set new size hints
    if (digitType() != IndividualDigit)
    {
        QPixmap tmp = d->renderSegmentedDigit("digit", "blank");
        d->m_widthHint = tmp.width();
        d->m_heightHint = tmp.height();
    }
    else
    {
        QPixmap tmp = d->renderIndividualDigit("blank", "blank");
        d->m_widthHint = tmp.width();
        d->m_heightHint = tmp.height();
    }

    // Combine all characters, to facilitate later searches
    d->m_characterMap.unite(d->m_numbersMap);
    d->m_characterMap.unite(d->m_punctuationMap);
    d->m_characterMap.unite(d->m_alphaMap);

    if (cacheOption() == CacheAll)
    {
        switch (digitType())
        {
            case IndividualDigit:
                d->renderIndividualDigits();
                break;

            case SevenSegmentDigit:
                d->renderSegmentedDigits(QString("punctuation"), d->m_punctuationMap);
                d->renderSegmentedDigits(QString("digit"), d->m_numbersMap);
                d->renderSegmentedDigits(QString("digit"), d->m_alphaMap);
                break;

            case FourteenSegmentDigit:
                d->renderSegmentedDigits(QString("punctuation"), d->m_punctuationMap);
                d->renderSegmentedDigits(QString("digit"), d->m_numbersMap);
                d->renderSegmentedDigits(QString("digit"), d->m_alphaMap);
                break;
        }
    }

    if (cacheOption() == CacheNumeralsOnly)
    {
        switch (digitType())
        {
            case IndividualDigit:
                d->renderIndividualDigits();
                break;

            case SevenSegmentDigit:
                d->renderSegmentedDigits(QString("punctuation"), d->m_punctuationMap);
                d->renderSegmentedDigits(QString("digit"), d->m_numbersMap);
                break;

            case FourteenSegmentDigit:
                d->renderSegmentedDigits(QString("punctuation"), d->m_punctuationMap);
                d->renderSegmentedDigits(QString("digit"), d->m_numbersMap);
                break;
        }
    }

    setPixmapCacheDirty(false);
    kDebug () << "cache has been rebuilt";

    if (cacheOption() == CacheNone){return;}
    if (cacheOption() == CachePreviouslyRendered){return;}
}

//
// Setters and Accessors
//

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
    kDebug () << "Cache option set to:" << d->lookupCacheOptionName(d->m_cacheOption);

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
    if (type.toLower() == "individualdigit")
    {
        d->m_digitType = IndividualDigit;
    }
    else if (type.toLower() == "sevensegmentdigit")
    {
        d->m_digitType = SevenSegmentDigit;
    }
    else if (type.toLower() == "fourteensegmentdigit")
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
    kDebug () << "Digit style set to:" << d->lookupDigitStyleName(d->m_digitStyle);
}

void KGameSvgDigits::setDigitStyle(const QString& style)
{
    if (style.toLower() == "ledstyle")
    {
        setDigitStyle(LedStyle);
    }
    else if (style.toLower() == "lcdstyle")
    {
        setDigitStyle(LcdStyle);
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
    // Scale letter-spacing & padding as needed
    d->m_letterSpacing = qRound(d->m_letterSpacing * scale);
    d->m_paddingLeft = qRound(d->m_paddingLeft * scale);
    d->m_paddingRight = qRound(d->m_paddingRight * scale);

    d->m_pixmapCacheDirty = true;
}

double KGameSvgDigits::scaleX()
{
    return d->m_scaleX;
}

void KGameSvgDigits::setScaleY(double scale)
{
    d->m_scaleY = scale;
    // Scale padding as needed
    d->m_paddingTop = qRound(d->m_paddingTop * scale);
    d->m_paddingBottom = qRound(d->m_paddingBottom * scale);

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

void KGameSvgDigits::setLetterSpacing(const int spacing)
{
    d->m_letterSpacing = spacing;
}

int KGameSvgDigits::letterSpacing()
{
    return d->m_letterSpacing;
}

void KGameSvgDigits::setPaddingTop(const int padding)
{
    d->m_paddingTop = padding;
}

int KGameSvgDigits::paddingTop()
{
    return d->m_paddingTop;
}

void KGameSvgDigits::setPaddingRight(const int padding)
{
    d->m_paddingRight = padding;
}

int KGameSvgDigits::paddingRight()
{
    return d->m_paddingRight;
}

void KGameSvgDigits::setPaddingBottom(const int padding)
{
    d->m_paddingBottom = padding;
}

int KGameSvgDigits::paddingBottom()
{
    return d->m_paddingBottom;
}

void KGameSvgDigits::setPaddingLeft(const int padding)
{
    d->m_paddingLeft = padding;
}

int KGameSvgDigits::paddingLeft()
{
    return d->m_paddingLeft;
}

void KGameSvgDigits::setPadding(const int paddingTop, const int paddingRight,
                                const int paddingBottom, const int paddingLeft)
{
    setPaddingTop(paddingTop);
    setPaddingRight(paddingRight);
    setPaddingBottom(paddingBottom);
    setPaddingLeft(paddingLeft);
}

//
// SLOTS
//

void KGameSvgDigits::updateFlash()
{
    d->swapColors();
    emit signalDisplayDirty();
}


//
// KGameSvgDigitsPrivate definition
//

ulong KGameSvgDigitsPrivate::toBitmask(const QString& string)
{
    ulong tmpMask = 0;

    // Parse the string to give us an ulong
    for (int j = 0; j < string.count(); ++j)
    {
        if (string.data()[j] == '1')
        {
            tmpMask |= (1 << j);

        }
    }
    return tmpMask;
}

QString KGameSvgDigitsPrivate::lookupCacheOptionName(const int option)
{
    switch (option)
    {
        case 1:
            return "CacheNumeralsOnly";
            break;
        case 2:
            return "CachePreviouslyRendered";
            break;
        case 3:
            return "CacheNone";
            break;
        case 4:
            return "CacheAll";
            break;
        default:
            return "error";
    }
}

QString KGameSvgDigitsPrivate::lookupDigitStyleName(const int style)
{
    switch (style)
    {
        case 1:
            return "LedStyle";
            break;
        case 2:
            return "LcdStyle";
            break;
        default:
            return "error";
    }
}

QPixmap KGameSvgDigitsPrivate::renderDigit(const QDomNode& node, const QString& cacheId)
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

    // Use a transparent background for each digit
    QColor alpha = m_backgroundColor;
    alpha.setAlpha(0);
    pixmap.fill(alpha);

    QPainter painter (&pixmap);
    m_svgRenderer.render(&painter, m_elementId, viewport);
    painter.end();

    if (m_cacheOption != KGameSvgDigits::CacheNone)
    {
        m_digitsPixmapCache.insert(cacheId, pixmap);
        kDebug () << "caching digit:" << cacheId;
    }

    return pixmap;
}

QPixmap KGameSvgDigitsPrivate::renderIndividualDigit(const QString& element, const QString& cacheId)
{
    QDomNode containerElement;
    QDomNode faceElement;

    m_elementId = element;
    containerElement = m_svgDOM.elementById(m_elementId);
    faceElement = m_svgDOM.elementById(m_elementId + "Foreground");

    if(!faceElement.isNull() && (!m_elementId.contains("blank")) )
    {
        applyColor(faceElement, m_foregroundColor);
    }

    return renderDigit(containerElement, cacheId);
}

void KGameSvgDigitsPrivate::renderIndividualDigits()
{
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

            id += colorStyle;
            renderIndividualDigit(m_elementId, id);
        }
        swapColors();
        it.toFront();
    }
}

QPixmap KGameSvgDigitsPrivate::renderSegmentedDigit(const QString& element, const QString& cacheId)
{
    QDomNodeList faceElementPaths;
    QDomNode containerElement;
    QDomNode faceElement;
    QString id;

    m_elementId = element;
    containerElement = m_svgDOM.elementById(m_elementId);
    faceElement = QDomNode(m_svgDOM.elementById(m_elementId + "Face"));

    faceElementPaths = faceElement.childNodes();

    if (cacheId.contains(".highlight"))
    {
        id = cacheId.left(cacheId.indexOf(".highlight"));
    }
    else
    {
        id = cacheId;
    }

    m_bitmask = m_bitmasks.value(id);
    applyBitmask(faceElementPaths);
    return renderDigit(containerElement, cacheId);

}

void KGameSvgDigitsPrivate::renderSegmentedDigits(const QString& element, const QMap<QString, QString>& map)
{
    QString id;
    QString colorStyle = "";
    QMapIterator<QString, QString> it(map);

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

            id += colorStyle;
            renderSegmentedDigit(element, id);
        }

        swapColors();
        it.toFront();
        colorStyle = "";
    }

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

void KGameSvgDigitsPrivate::applyColor(const QDomNode &node, const QColor &color)
{
    /*  In SVG, the alpha channel is spec'd in the fill-opacity property, not
     *  in the fill property.  SVG fill-opacity must be spec'd as a number from 0 to 1
     */
    QString svgFillOpacity = QString::number(color.alpha() * 0.00392156862745); // multiply by 1/255
    QString svgFill = color.name();

    m_svgDOM.setCurrentNode(node);

    m_svgDOM.setStyleProperty("fill", svgFill);
    m_svgDOM.setStyleProperty("fill-opacity", svgFillOpacity);
}

void KGameSvgDigitsPrivate::applyBitmask(const QDomNodeList& nodes)
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
//          kDebug () << "segment on";
            applyColor(nodes.item(j), m_foregroundColor);
        }
        else
        {
//          kDebug () << "off";
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
