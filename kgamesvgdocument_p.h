/** @file
 * This file contains the regexs for parsing the transform attribute of
 * an SVG file using DOM.
 *
 * @see: http://www.w3.org/TR/SVG/coords.html#TransformAttribute
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

#ifndef _KGAMESVGDOCUMENT_P_H_
#define _KGAMESVGDOCUMENT_P_H_


/**
* @brief A regex that matches a single whitespace
*/
static const QString WSP = QString("\\s");

/**
* @brief A regex that matches zero or more whitespace
*/
static const QString WSP_ASTERISK = QString(WSP + '*');

/**
* @brief A regex that matches a comma
*/
static const char COMMA = ',';

/**
* @brief A regex that matches a comma or whitespace
*/
static const QString COMMA_WSP = QString("(?:(?:" + WSP + '+' + COMMA + '?' + 
                WSP + "*)|(?:" + COMMA + WSP + "*))");

/**
* @brief A regex that matches a number
*/
static const QString NUMBER = QString("(?:(?:[-|\\+]?\\d+(?:\\.)*\\d*(?:e)?[-|\\+]?\\d*)|(?:[-|\\+]?(?:\\.)+\\d*(?:e)?[-|\\+]?\\d*))");
// Do not wrap the above line!

/**
* @brief A regex that matches opening parenthesis
*/
static const QString OPEN_PARENS = QString("\\(");

/**
* @brief A regex that matches closing parenthesis
*/
static const QString CLOSE_PARENS = QString("\\)");

/**
* @brief A regex that matches a matrix transform
*/
static const QString MATRIX = QString("(matrix)" + WSP_ASTERISK + OPEN_PARENS + WSP_ASTERISK + 
                '(' + NUMBER + ')' + COMMA_WSP + 
                '(' + NUMBER + ')' + COMMA_WSP + 
                '(' + NUMBER + ')' + COMMA_WSP + 
                '(' + NUMBER + ')' + COMMA_WSP + 
                '(' + NUMBER + ')' + COMMA_WSP + 
                '(' + NUMBER + ')' + WSP_ASTERISK + CLOSE_PARENS);

/**
* @brief A regex that matches a translate transform
*/
static const QString TRANSLATE = QString("(translate)" + WSP_ASTERISK + OPEN_PARENS + WSP_ASTERISK + 
                '(' + NUMBER + ')' + 
                "(?:" + COMMA_WSP + '(' + NUMBER + ')' + ")?" + WSP_ASTERISK + CLOSE_PARENS);

/**
* @brief A regex that matches scale transform
*/
static const QString SCALE = QString("(scale)" + WSP_ASTERISK + OPEN_PARENS + WSP_ASTERISK + 
                '(' + NUMBER + ')' + 
                "(?:" + COMMA_WSP + '(' + NUMBER + ')' + ")?" + WSP_ASTERISK + CLOSE_PARENS);

/**
* @brief A regex that matches rotate transform
*/
static const QString ROTATE = QString("(rotate)" + WSP_ASTERISK + OPEN_PARENS + WSP_ASTERISK + 
                '(' + NUMBER + ')' + "(?:" + COMMA_WSP + 
                '(' + NUMBER + ')' + COMMA_WSP + 
                '(' + NUMBER + ')' +  ")?" + WSP_ASTERISK + CLOSE_PARENS);

/**
* @brief A regex that matches skewX transform
*/
static const QString SKEW_X = QString("(skewX)" + WSP_ASTERISK + OPEN_PARENS + WSP_ASTERISK + 
                '(' + NUMBER + ')' + WSP_ASTERISK + CLOSE_PARENS);

/**
* @brief A regex that matches skewY transform
*/
static const QString SKEW_Y = QString("(skewY)" + WSP_ASTERISK + OPEN_PARENS + WSP_ASTERISK + 
                '(' + NUMBER + ')' + WSP_ASTERISK + CLOSE_PARENS);

/**
* @brief A regex that matches any single transform
*/
static const QString TRANSFORM = QString("(?:" + MATRIX + "|" + TRANSLATE + "|" + SCALE + "|" + 
                ROTATE + "|" + SKEW_X + "|" + SKEW_Y + ")");

/**
* @brief A regex that matches the entire transform attribute
*/
static const QString TRANSFORMS = QString("(?:" + TRANSFORM + "|" + "(?:" + TRANSFORM + 
                COMMA_WSP + "+)*" + TRANSFORM + ")");

#endif // _KGAMESVGDOCUMENT_P_H_
