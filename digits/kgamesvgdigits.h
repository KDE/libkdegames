/** @file
 * This file contains the KGameSvgDigits base class, used for displaying
 * a series of svg digits.
 */

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

#ifndef _KGAMESVGDIGITS_H_
#define _KGAMESVGDIGITS_H_

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtGui/QColor>
#include <QtGui/QPixmap>
#include <QtXml/QDomDocument>

#include <KGameSvgDocument>
#include <KSvgRenderer>

#include <libkdegames_export.h>

class KGameSvgDigitsPrivate;

/**
 * @brief Renders a string of digits using an SVG theme
 * 
 * @note To help limit rounding errors as the svg elements get scaled,
 *        make sure the svg container elements have whole integer widths and heights.
 * 
 * Basic use case:
 * @code
 * QString themePath = "kdegames/libkdegames/digits/themes/individual-digits-sample.svg";
 * digits = new KGameSvgDigits(themePath);
 * digits.refreshCache();
 * QPixmap pixmap = digits.display("12:16");
 * @endcode
 *
 * @author Mark A. Taff \<kde@marktaff.com\>
 */
class KDEGAMES_EXPORT KGameSvgDigits : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructor
     *
     * Create a KGameSvgDigits object with default values:
     * @li @c foregroundColor to ARGB #ffffaa00 orange
     * @li @c backgroundColor to ARGB #ff323232 dark gray
     * @li @c foregroundHighlightColor to ARGB #ffff0000 red
     * @li @c backgroundHighlightColor to ARGB #ffeeeeec very light gray
     * @li @c cache to @c CacheNumeralsOnly
     * @li @c style to @c LedStyle
     *
     * @param svgFile The path to the svg file to load
     * @returns nothing
     */
    explicit KGameSvgDigits(const QString& svgFile);

    /**
     * @overload
     * @brief Constructor
     *
     * Create a KGameSvgDigits object with default values:
     * @li @c foregroundColor to ARGB #ffffaa00 orange
     * @li @c backgroundColor to ARGB #ff323232 dark gray
     * @li @c foregroundHighlightColor to ARGB #ffff0000 red
     * @li @c backgroundHighlightColor to ARGB #ffeeeeec very light gray
     * @li @c cache to @c CacheNumeralsOnly
     * @li @c style to @c LedStyle
     *
     * @returns nothing
     */
    KGameSvgDigits();

    /**
     * @brief Destructor
     *
     * @returns nothing
     */
    virtual ~KGameSvgDigits();

    /**
     * @brief Loads a theme
     *
     * @returns nothing
     */
    void loadTheme(const QString& svgFile);

    /**
     * @brief Options for cacheing pixmaps of rendered digits
     */ 
    enum CacheOption {
        /**
        * Render & cache numerals & punctuation in advance
        */
        CacheNumeralsOnly = 0x01,
        /**
        * Do not cache in advance; only cache digits actually used
        */
        CachePreviouslyRendered = 0x02,
        /**
        * Do not cache at all.  Render directly. Could take a lot of resources.
        */
        CacheNone = 0x03,
        /**
        * Render & cache in advance all possible digits
        * This is only used if DigitType is IndividualDigit
        */
        CacheAll = 0x04
    };
    /** @brief Q_DECLARE_FLAGS macro confuses doxygen, so create typedef's manually */
    typedef QFlags<CacheOption> CacheOptions;

    /**
     * @brief Type of the svg digit
     */ 
    enum DigitType {
        /**
        * Any digit that is directly rendered.
        */
        IndividualDigit = 0x01,
        /**
        * A seven segment digit where each segment is on/off according to the bitmask passed in.
        */
        SevenSegmentDigit = 0x02,
        /**
        * A fourteen segment digit where each segment is on/off according to the bitmask passed in.
        */
        FourteenSegmentDigit = 0x03
    };
    /** @brief Q_DECLARE_FLAGS macro confuses doxygen, so create typedef's manually */
    typedef QFlags<DigitType> DigitTypes;

    /**
     * @brief Style of the svg digit
     */ 
    enum DigitStyle {
        /**
        * Draw the off segments faintly like an LED display.
        */
        LedStyle = 0x01,
        /**
        * Do not render the off segments, as in an monochrome LCD display.
        */
        LcdStyle = 0x02
    };
    /** @brief Q_DECLARE_FLAGS macro confuses doxygen, so create typedef's manually */
    typedef QFlags<DigitStyle> DigitStyles;

    /**
     * @brief Invalidate current pixmap cache and rebuild according to @c CacheOption
     * @returns returns nothing.
     */
    void refreshCache();

    /**
     * @brief Display the string
     * @param display The string to display
     *
     * @returns returns a pixmap showing @c display.
     */
    QPixmap display(const QString& display);

    /**
     * @brief Flash the display
     * @param interval The interval between flashes, in milli-seconds
     *
     * @returns returns nothing.
     */
    void flash(int interval);

    /**
     * @overload
     * @brief Flash the display with an interval of 800 milli-seconds
     *
     * @returns returns nothing.
     */
    void flash();

    /**
     * @brief Stop flashing the display
     *
     * @returns returns nothing.
     */
    void stopFlashing();

    /**
     * @brief Toggle highlighting the display
     *
     * @returns returns nothing.
     */
    void highlight();

    /**
     * @brief Convert a string binary representation to convert to a bitmask
     *
     * @param string The string binary representation, e.g. "01001111", to convert to a bitmask
     * @returns returns the bitmask
     */
    ulong toBitmask(QString string);


//
// Setters & Accessors
//

    /**
     * @brief Sets whether the pixmap cache is dirty
     *
     * @param isDirty Is the pixmap cache dirty?
     * @returns returns nothing.
     */
    void setPixmapCacheDirty(bool isDirty);

    /**
     * @brief Returns whether the pixmap cache is dirty
     *
     * @returns whether the pixmap cache is dirty
     */
    bool isPixmapCacheDirty();

    /**
     * @brief Sets whether this digit is an LED or LCD
     *
     * @param style Is this digit and LED
     * @returns returns nothing.
     */
    void setDigitStyle(const DigitStyle& style);

    /**
     * @overload
     * @brief Sets whether this digit is an LED or LCD
     *
     * @param style Is this digit and LED
     * @returns returns nothing.
     */
    void setDigitStyle(const QString& style);

    /**
     * @brief Returns whether this digit is an LED or LCD
     *
     * @returns whether this digit is an LED or LCD
     */
    int digitStyle();

    /**
     * @brief Sets the number of digits
     *
     * @param numberOfDigits The number of digits
     * @returns returns nothing.
     */
    void setNumberOfDigits(int numberOfDigits);

    /**
     * @brief Returns the number of digits
     *
     * @returns Returns the number of digits
     */
    int numberOfDigits();

    /**
     * @brief Sets the cache strategy
     *
     * @param option the cache strategy
     * @returns returns nothing.
     */
    void setCacheOption(CacheOptions option);

    /**
     * @overload
     * @brief Sets the cache strategy
     *
     * @param option the cache strategy
     * @returns returns nothing.
     */
    void setCacheOption(const QString& option);

    /**
     * @brief Returns the cache strategy
     *
     * @returns Returns the cache strategy
     */
    int cacheOption();

    /**
     * @brief Sets the type of the digit, individual|seven|fourteen
     *
     * @param type the type of the digit, individual|seven|fourteen
     * @returns returns nothing.
     */
    void setDigitType(DigitTypes type);

    /**
     * @overload
     * @brief Sets the type of the digit, IndividualDigit|SevenSegmentDigit|FourteenSegmentDigit
     *
     * @param type the type of the digit, individual|seven|fourteen
     * @returns returns nothing.
     */
    void setDigitType(const QString& type);

    /**
     * @brief Returns the type of the digit, individual|seven|fourteen
     *
     * @returns Returns the type of the digit, individual|seven|fourteen
     */
    int digitType();

    /**
     * @brief Sets the foreground color
     *
     * @param foregroundColor The color for the foreground
     * @returns returns nothing.
     */
    void setForegroundColor(const QColor& foregroundColor);

    /**
     * @brief Returns the foreground color
     *
     * @returns Returns the foreground color
     */
    QColor foregroundColor();

    /**
     * @brief Sets the background color
     *
     * @param backgroundColor The color for the background
     * @returns returns nothing.
     */
    void setBackgroundColor(const QColor& backgroundColor);

    /**
     * @brief Returns the background color
     *
     * @returns Returns the background color
     */
    QColor backgroundColor();

    /**
     * @brief Sets the foreground highlight color
     *
     * @param foregroundHighlightColor The color for the foreground highlight
     * @returns returns nothing.
     */
    void setForegroundHighlightColor(const QColor& foregroundHighlightColor);

    /**
     * @brief Returns the foreground highlight color
     *
     * @returns Returns the foreground highlight color
     */
    QColor foregroundHighlightColor();

    /**
     * @brief Sets the background highlight color
     *
     * @param backgroundHighlightColor The color for the background highlight
     * @returns returns nothing.
     */
    void setBackgroundHighlightColor(const QColor& backgroundHighlightColor);

    /**
     * @brief Returns the background highlight color
     *
     * @returns Returns the background highlight color
     */
    QColor backgroundHighlightColor();

    /**
     * @brief Sets the factor to scale the x-axis by.
     *
     * @param scale The factor to scale the x-axis by.
     * @returns returns nothing.
     */
    void setScaleX(double scale);

    /**
     * @brief Returns the factor to scale the x-axis by.
     *
     * @returns factor to scale the x-axis by.
     */
    double scaleX();

    /**
     * @brief Sets the factor to scale the y-axis by.
     *
     * @param scale The factor to scale the y-axis by.
     * @returns returns nothing.
     */
    void setScaleY(double scale);

    /**
     * @brief Returns the factor to scale the y-axis by.
     *
     * @returns Returns factor to scale the y-axis by.
     */
    double scaleY();

    /**
     * @brief Sets the factor to skew the x-axis by.
     *
     * @param skew The factor to skew the x-axis by.
     * @returns returns nothing.
     */
    void setSkewX(double skew);

    /**
     * @brief Returns the factor to skew the x-axis by.
     *
     * @returns factor to skew the x-axis by.
     */
    double skewX();

    /**
     * @brief Sets the factor to skew the y-axis by.
     *
     * @param skew The factor to skew the y-axis by.
     * @returns returns nothing.
     */
    void setSkewY(double skew);

    /**
     * @brief Returns the factor to skew the y-axis by.
     *
     * @returns Returns factor to skew the y-axis by.
     */
    double skewY();

    /**
     * @brief Sets the alpha channel level for "off" segments of LED's
     *
     * @param alphaLevel the alpha channel level
     * @returns returns nothing.
     */
    void setLedOffSegmentAlphaLevel(int alphaLevel);

    /**
     * @brief Returns the alpha channel level for "off" segments of LED's
     *
     * @returns Returns the alpha channel level
     */
    int ledOffSegmentAlphaLevel();

    /**
     * @brief Sets the bitmask to render
     *
     * @param mask The bitmask to set.
     * @returns returns nothing.
     */
    void setBitmask(ulong mask);

    /**
     * @brief Returns the bitmask to render.
     *
     * @returns Returns the bitmask to render.
     */
    ulong bitmask();

    /**
     * @brief Sets the digit to render
     *
     * @param digit The digit to set.
     * @returns returns nothing.
     */
    void setDigit(const QString& digit);

    /**
     * @brief Returns the digit to render.
     *
     * @returns Returns the digit to render.
     */
    QString digit();

    /**
     * @brief Sets elementId of the digit to render
     *
     * @param elementId The elementId of the digit to set.
     * @returns returns nothing.
     */
    void setElementId(const QString& elementId);

    /**
     * @brief Returns the elementId of the digit to render.
     *
     * @returns Returns the elementId of the digit to render.
     */
    QString elementId();



//
// Properties
//


private:

    /**
     * @brief d-pointer
     */
    KGameSvgDigitsPrivate * const d;

signals:

    /**
    * Use this signal to change the content of the display
    */
    void signalDisplayDirty();

private slots:

    void updateFlash();

};
Q_DECLARE_OPERATORS_FOR_FLAGS(KGameSvgDigits::CacheOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(KGameSvgDigits::DigitTypes)
Q_DECLARE_OPERATORS_FOR_FLAGS(KGameSvgDigits::DigitStyles)

#endif // _KGAMESVGDIGITS_H_
