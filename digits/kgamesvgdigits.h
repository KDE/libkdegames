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

#include <QtCore/QObject>
#include <QtGui/QColor>
#include <QtGui/QPixmap>

#include <kgamesvgdocument.h>
#include <ksvgrenderer.h>

#include <libkdegames_export.h>

class KGameSvgDigitsPrivate;

/**
 * \class KGameSvgDigits kgamesvgdigits.h <KGameSvgDigits>
 * 
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
 * You can overide any values specified in the theme if you need to. Make your changes after
 * the theme is loaded, and before you call @c refreshCache(). Make sure you
 * document any such overides for theme authors.
 *
 * To change the theme:
 * @code
 * QString newThemePath = "kdegames/libkdegames/digits/themes/fourteen-segment-sample.svg";
 * digits.loadTheme(newThemePath);
 * digits.refreshCache();
 * QPixmap pixmap = digits.display(" 12:16.7");
 * @endcode
 * 
 * To enable the display to update, you need to connect the signal @c signalDisplayDirty
 * to an appropriate slot in your application, such as:
 * @code
 * connect (digits, SIGNAL(signalDisplayDirty()), 
 * this, SLOT(display()));
 * @endcode
 * 
 * To toggle between the highlighted colors and the regular colors, call @c highlight():
 * @code
 * // Use highlight colors
 * digits.highlight();
 * 
 * // Revert to normal colors
 * digits.highlight();
 * @endcode
 * 
 * To flash the display, call @c flash():
 * @code
 * // Flash to display every 800 milli-seconds
 * digits.flash();
 * 
 * // Flash the display every 2 seconds
 * digits.flash(2000);
 * @endcode
 * 
 * To stop flashing the display, call @c stopFlashing():
 * @code
 * // Stop flashing the display
 * digits.stopFlashing();
 * @endcode
 * 
 * For caching options, you will most likely find either \ref CachePreviouslyRendered
 * or \ref CacheNumeralsOnly to fit your needs. \ref CacheNone and \ref CacheAll
 * are probably only useful in a few edge cases.
 * 
 * There are two basic digit types: individual digits and segmented digits. Individual digits
 * are have a foreground, a backgound, and a container.  Segemented digits have their segments
 * turned on or off according to a bitmask, like a real segmented display.
 * 
 * A theme for an \ref IndividualDigit type must contain a glyph for every charcater to be
 * rendered.  A theme for a \ref SevenSegmentDigit or \ref FourteenSegmentDigit omly
 * needs to contain a glyph for the digit, and a glyph for the punctuation.
 * 
 * \ref IndividualDigit type themes require more work from the artist, but they render
 * much faster (generally) at runtime.  The inverse is true for segmented digits.
 * 
 * Note that an artist can create a seven, fourteen, or grid display as a \ref IndividualDigit
 * type.  This is probably the way to go from a performance perspective.
 * 
 * We ignore and skip displaying any invalid digits in the string passed in to \ref display().
 * 
 * Characters that <b>must</b> be in every theme for it to be valid are a blank space, the numbers 0-9, the 
 * punctuation characters comma, period, colon, semi-colon, and minus sign.
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
     * @li @c foregroundColor to ARGB \#ffffaa00 orange
     * @li @c backgroundColor to ARGB \#ff323232 dark gray
     * @li @c foregroundHighlightColor to ARGB \#ffff0000 red
     * @li @c backgroundHighlightColor to ARGB \#ffeeeeec very light gray
     * @li @c cache to @c CacheNumeralsOnly
     * @li @c style to @c LedStyle
     *
     * @param themeFile The path to the theme's *.desktop file to load
     * @returns nothing
     */
    explicit KGameSvgDigits(const QString& themeFile);

    /**
     * @overload
     * @brief Constructor
     *
     * Create a KGameSvgDigits object with default values:
     * @li @c foregroundColor to ARGB \#ffffaa00 orange
     * @li @c backgroundColor to ARGB \#ff323232 dark gray
     * @li @c foregroundHighlightColor to ARGB \#ffff0000 red
     * @li @c backgroundHighlightColor to ARGB \#ffeeeeec very light gray
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
     * @param themeFile The path to the theme's *.desktop file to load
     * @returns nothing
     */
    void loadTheme(const QString& themeFile);

    /**
     * @brief Options for caching pixmaps of rendered digits
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
     * @returns nothing
     */
    void refreshCache();

    /**
     * @brief Display the string
     * @param display The string to display
     * 
     * We skip and ignore any invalid characters passed in, where 'invalid' means any character
     * that isn't already cached, can't be produced, or isn't in the svg file.
     * 
     * We can display leading and trailing spaces, so pad the string passed in so that
     * the digits occupy the value position you want.
     * 
     * @returns a pixmap showing @c display.
     */
    QPixmap display(const QString& display);

    /**
     * @brief Flash the display
     * @param interval The interval between flashes, in milli-seconds
     *
     * @returns nothing
     */
    void flash(int interval);

    /**
     * @overload
     * @brief Flash the display with an interval of 800 milli-seconds
     *
     * @returns nothing
     */
    void flash();

    /**
     * @brief Stop flashing the display
     *
     * @returns nothing
     */
    void stopFlashing();

    /**
     * @brief Toggle highlighting the display
     *
     * @returns nothing
     */
    void highlight();

//
// Setters & Accessors
//

    /**
     * @brief Sets whether the pixmap cache is dirty
     * 
     * @param isDirty Is the pixmap cache dirty?
     * @returns nothing
     * @see isPixmapCacheDirty()
     */
    void setPixmapCacheDirty(bool isDirty);

    /**
     * @brief Returns whether the pixmap cache is dirty
     *
     * @returns whether the pixmap cache is dirty
     * @see setPixmapCacheDirty()
     */
    bool isPixmapCacheDirty();

    /**
     * @brief Sets whether this digit is an LED or LCD
     *
     * @param style Is this digit and LED
     * @returns nothing
     * @see digitStyle()
     */
    void setDigitStyle(const DigitStyle& style);

    /**
     * @overload
     * @brief Sets whether this digit is an LED or LCD
     *
     * @param style Is this digit and LED
     * @returns nothing
     * @see digitStyle()
     */
    void setDigitStyle(const QString& style);

    /**
     * @brief Returns whether this digit is an LED or LCD
     *
     * @returns whether this digit is an LED or LCD
     * @see setDigitStyle()
     */
    int digitStyle();

    /**
     * @brief Sets the number of digits
     *
     * @param numberOfDigits The number of digits
     * @returns nothing
     * @see numberOfDigits()
     */
    void setNumberOfDigits(int numberOfDigits);

    /**
     * @brief Returns the number of digits
     *
     * @returns the number of digits
     * @see setNumberOfDigits()
     */
    int numberOfDigits();

    /**
     * @brief Sets the cache strategy
     *
     * @param option the cache strategy
     * @returns nothing
     * @see cacheOption()
     */
    void setCacheOption(CacheOptions option);

    /**
     * @overload
     * @brief Sets the cache strategy
     *
     * @param option the cache strategy
     * @returns nothing
     * @see cacheOption()
     */
    void setCacheOption(const QString& option);

    /**
     * @brief Returns the cache strategy
     *
     * @returns the cache strategy
     * @see setDigitType()
     */
    int cacheOption();

    /**
     * @brief Sets the type of the digit, individual|seven|fourteen
     *
     * @param type the type of the digit, individual|seven|fourteen
     * @returns nothing
     * @see digitType()
     */
    void setDigitType(DigitTypes type);

    /**
     * @overload
     * @brief Sets the type of the digit, IndividualDigit|SevenSegmentDigit|FourteenSegmentDigit
     *
     * @param type the type of the digit, individual|seven|fourteen
     * @returns nothing
     * @see digitType()
     */
    void setDigitType(const QString& type);

    /**
     * @brief Returns the type of the digit, individual|seven|fourteen
     *
     * @returns the type of the digit, individual|seven|fourteen
     * @see setDigitType()
     */
    int digitType();

    /**
     * @brief Sets the foreground color
     *
     * @param foregroundColor The color for the foreground
     * @returns nothing
     * @see foregroundColor(), backgroundColor(), setBackgroundColor()
     */
    void setForegroundColor(const QColor& foregroundColor);

    /**
     * @brief Returns the foreground color
     *
     * @returns the foreground color
     * @see setForegroundColor(), backgroundColor(), setBackgroundColor()
     */
    QColor foregroundColor();

    /**
     * @brief Sets the background color
     *
     * @param backgroundColor The color for the background
     * @returns nothing
     * @see backgroundColor(), foregroundColor(), setForegroundColor()
     */
    void setBackgroundColor(const QColor& backgroundColor);

    /**
     * @brief Returns the background color
     *
     * @returns the background color
     * @see setBackgroundColor(), foregroundColor(), setForegroundColor()
     */
    QColor backgroundColor();

    /**
     * @brief Sets the foreground highlight color
     *
     * @param foregroundHighlightColor The color for the foreground highlight
     * @returns nothing
     * @see foregroundHighlightColor(), backgroundHighlightColor(), setBackgroundHighlightColor()
     */
    void setForegroundHighlightColor(const QColor& foregroundHighlightColor);

    /**
     * @brief Returns the foreground highlight color
     *
     * @returns the foreground highlight color
     * @see setForegroundHighlightColor(), backgroundHighlightColor(), setBackgroundHighlightColor()
     */
    QColor foregroundHighlightColor();

    /**
     * @brief Sets the background highlight color
     *
     * @param backgroundHighlightColor The color for the background highlight
     * @returns nothing
     * @see backgroundHighlightColor(), foregroundHighlightColor(), setForegroundHighlightColor()
     */
    void setBackgroundHighlightColor(const QColor& backgroundHighlightColor);

    /**
     * @brief Returns the background highlight color
     *
     * @returns the background highlight color
     * @see setBackgroundHighlightColor(), foregroundHighlightColor(), setForegroundHighlightColor()
     */
    QColor backgroundHighlightColor();

    /**
     * @brief Sets the factor to scale the x-axis by.
     *
     * @param scale The factor to scale the x-axis by.
     * @returns nothing
     * @see scaleX(), scaleY(), setScaleY()
     */
    void setScaleX(double scale);

    /**
     * @brief Returns the factor to scale the x-axis by.
     *
     * @returns factor to scale the x-axis by.
     * @see setScaleX(), scaleY(), setScaleY()
     */
    double scaleX();

    /**
     * @brief Sets the factor to scale the y-axis by.
     *
     * @param scale The factor to scale the y-axis by.
     * @returns nothing
     * @see scaleY(), scaleX(), setScaleX()
     */
    void setScaleY(double scale);

    /**
     * @brief Returns the factor to scale the y-axis by.
     *
     * @returns factor to scale the y-axis by.
     * @see setScaleY(), scaleX(), setScaleX()
     */
    double scaleY();

    /**
     * @brief Sets the factor to skew the x-axis by.
     *
     * @param skew The factor to skew the x-axis by.
     * @returns nothing
     * @see skewX(), skewY(), setSkewY()
     */
    void setSkewX(double skew);

    /**
     * @brief Returns the factor to skew the x-axis by.
     *
     * @returns factor to skew the x-axis by.
     * @see setSkewX(), skewY(), setSkewY()
     */
    double skewX();

    /**
     * @brief Sets the factor to skew the y-axis by.
     *
     * @param skew The factor to skew the y-axis by.
     * @returns nothing
     * @see skewY(), skewX(), setSkewX()
     */
    void setSkewY(double skew);

    /**
     * @brief Returns the factor to skew the y-axis by.
     *
     * @returns factor to skew the y-axis by.
     * @see setSkewY(), skewX(), setSkewX()
     */
    double skewY();

    /**
     * @brief Sets the alpha channel level for "off" segments of LED's
     *
     * @param alphaLevel the alpha channel level
     * @returns nothing
     * @see ledOffSegmentAlphaLevel()
     */
    void setLedOffSegmentAlphaLevel(int alphaLevel);

    /**
     * @brief Returns the alpha channel level for "off" segments of LED's
     *
     * @returns the alpha channel level
     * @see setLedOffSegmentAlphaLevel()
     */
    int ledOffSegmentAlphaLevel();

    /**
     * @brief Sets the bitmask to render
     *
     * @param mask The bitmask to set.
     * @returns nothing
     * @see bitmask()
     */
    void setBitmask(ulong mask);

    /**
     * @brief Returns the bitmask to render.
     *
     * @returns the bitmask to render.
     * @see setBitmask()
     */
    ulong bitmask();

    /**
     * @brief Sets the digit to render
     *
     * @param digit The digit to set.
     * @returns nothing
     * @see digit()
     */
    void setDigit(const QString& digit);

    /**
     * @brief Returns the digit to render.
     *
     * @returns the digit to render.
     * @see setDigit()
     */
    QString digit();

    /**
     * @brief Sets elementId of the digit to render
     *
     * @param elementId The elementId of the digit to set.
     * @returns nothing
     * @see elementId()
     */
    void setElementId(const QString& elementId);

    /**
     * @brief Returns the elementId of the digit to render.
     *
     * @returns the elementId of the digit to render.
     * @see setElementId()
     */
    QString elementId();

    /**
     * @brief Sets the horizontal spacing in pixels between the digits
     * 
     * The spacing in pixels to use when scaleX == 1.  Letter-spacing is scaled the same as
     * the digits themselves.
     *
     * @param spacing The horizontal spacing in pixels between the digits
     * @returns nothing
     * @see letterSpacing()
     */
    void setLetterSpacing(const int spacing);

    /**
     * @brief Returns the horizontal spacing in pixels between the digits.
     *
     * @returns the horizontal spacing in pixels between the digits.
     * @see setLetterSpacing()
     */
    int letterSpacing();

    /**
     * @brief Sets the padding in pixels between the top of the digits and the top of the display.
     * 
     * The padding in pixels to use when scaleX == 1.  padding-top is scaled the same as
     * the digits themselves.
     *
     * @param padding The padding in pixels between the top of the digits and the top of the display
     * @returns nothing
     * @see paddingTop(), setPadding()
     */
    void setPaddingTop(const int padding);

    /**
     * @brief Returns the padding in pixels between the top of the digits and the top of the display.
     *
     * @returns the padding in pixels between the top of the digits and the top of the display.
     * @see setPaddingTop()
     */
    int paddingTop();

    /**
     * @brief Sets the padding in pixels between the right-most digit and the right edge of the display.
     * 
     * The padding in pixels to use when scaleX == 1.  padding-right is scaled the same as
     * the digits themselves.
     *
     * @param padding The padding in pixels between the right-most digit and the right edge of the display
     * @returns nothing
     * @see paddingRight(), setPadding()
     */
    void setPaddingRight(const int padding);

    /**
     * @brief Returns the padding in pixels between the right-most digit and the right edge of the display.
     *
     * @returns the padding in pixels between the right-most digit and the right edge of the display
     * @see setPaddingRight()
     */
    int paddingRight();

    /**
     * @brief Sets the padding in pixels between the bottom of the digits and the bottom of the display.
     * 
     * The padding in pixels to use when scaleX == 1.  padding-bottom is scaled the same as
     * the digits themselves.
     *
     * @param padding The padding in pixels between the bottom of the digits and the bottom of the display
     * @returns nothing
     * @see paddingBottom(), setPadding()
     */
    void setPaddingBottom(const int padding);

    /**
     * @brief Returns the padding in pixels between the bottom of the digits and the bottom of the display.
     *
     * @returns the padding in pixels between the bottom of the digits and the bottom of the display
     * @see setPaddingBottom()
     */
    int paddingBottom();

    /**
     * @brief Sets the padding in pixels between the leftmost digit and the left edge of the display.
     * 
     * The padding in pixels to use when scaleX == 1.  padding-left is scaled the same as
     * the digits themselves.
     *
     * @param padding The padding in pixels between the leftmost digit and the left edge of the display
     * @returns nothing
     * @see paddingLeft(), setPadding()
     */
    void setPaddingLeft(const int padding);

    /**
     * @brief Returns the padding in pixels between the leftmost digit and the left edge of the display.
     *
     * @returns the padding in pixels between the leftmost digit and the left edge of the display
     * @see setPaddingLeft()
     */
    int paddingLeft();

    /**
     * @brief Sets the padding in pixels.
     * 
     * The padding in pixels to use when scaleX == 1.  padding is scaled the same as
     * the digits themselves.
     *
     * @param paddingTop The padding-top in pixels
     * @param paddingRight The padding-right in pixels
     * @param paddingBottom The padding-bottom in pixels
     * @param paddingLeft The padding-left in pixels
     * @returns nothing
     * @see setPaddingTop(), setPaddingRight(), setPaddingBottom(), setPaddingLeft()
     */
    void setPadding(const int paddingTop, const int paddingRight,
                    const int paddingBottom, const int paddingLeft);

//
// Properties
//

private:

    /**
     * @brief d-pointer
     */
    KGameSvgDigitsPrivate * const d;

Q_SIGNALS:

    /**
    * Use this signal to change the content of the display
    */
    void signalDisplayDirty();

private Q_SLOTS:

    void updateFlash();

};
Q_DECLARE_OPERATORS_FOR_FLAGS(KGameSvgDigits::CacheOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(KGameSvgDigits::DigitTypes)
Q_DECLARE_OPERATORS_FOR_FLAGS(KGameSvgDigits::DigitStyles)

#endif // _KGAMESVGDIGITS_H_
