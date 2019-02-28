/***************************************************************************
 *   Copyright 2012 Stefan Majewsky <majewsky@gmx.net>                     *
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

#include "kgamesvgdocumenttest.h"
#include "kgamesvgdocument_p.h"

#include <config-tests.h>

#include <QtCore>
#include <QtTest>

void tst_KGameSvgDocument::initTestCase()
{
    m_svgDom.load(TESTS_PATH"kgamesvgdocumenttest.svg");
}

void tst_KGameSvgDocument::cleanupTestCase()
{
    // Do nothing.
}

void tst_KGameSvgDocument::style()
{
    m_svgDom.elementById(QStringLiteral("digitBackground"));
    QVERIFY(!m_svgDom.currentNode().isNull());

    // test style() set & get
    QString originalStyle = m_svgDom.style();
    m_svgDom.setStyle(QStringLiteral("test"));
    QVERIFY(m_svgDom.style() == "test");

    m_svgDom.setStyle(originalStyle);
    QVERIFY(m_svgDom.style() == originalStyle);

    // test styleProperties() set & get
    QHash<QString, QString> originalStyleProperties = m_svgDom.styleProperties();
    m_svgDom.setStyle(QStringLiteral("test"));
    QVERIFY(m_svgDom.style() == "test");

    m_svgDom.setStyleProperties(originalStyleProperties, KGameSvgDocument::UseInkscapeOrder);
    QVERIFY(m_svgDom.style() == originalStyle);

    // test styleProperty() set & get
    QString orgiginalStyleProperty = m_svgDom.styleProperty(QStringLiteral("fill"));

    m_svgDom.setStyleProperty(QStringLiteral("fill"), QStringLiteral("ffffff"));
    QVERIFY(m_svgDom.styleProperty("fill") == "ffffff");

    m_svgDom.setStyle(originalStyle);
    QVERIFY(m_svgDom.styleProperty("fill") == orgiginalStyleProperty);
}

void tst_KGameSvgDocument::scale()
{
    m_svgDom.elementById(QStringLiteral("0x11"));
    QVERIFY(!m_svgDom.currentNode().isNull());

    QTransform originalTransformMatrix = m_svgDom.transformMatrix();

    m_svgDom.scale(2, 2, KGameSvgDocument::ApplyToCurrentMatrix);

    QTransform finalMatrix = m_svgDom.transformMatrix();

    double computedX = originalTransformMatrix.m11() * 2;
    double errorX = computedX - finalMatrix.m11();
    QVERIFY(errorX < 0.0000001);  // if our margin of error is small, we are probably ok

    double computedY = originalTransformMatrix.m22() * 2;
    double errorY = computedY - finalMatrix.m22();
    QVERIFY(errorY < 0.0000001);  // if our margin of error is small, we are probably ok

    // This should output a warning message
    m_svgDom.scale(1, 0, KGameSvgDocument::ApplyToCurrentMatrix);

}

void tst_KGameSvgDocument::transformRegex()
{

    QRegExp rx;

    rx.setPattern(WSP);
    QVERIFY(rx.isValid());
    QVERIFY(rx.exactMatch(QLatin1String( " " )));
    QVERIFY(!rx.exactMatch(""));

    rx.setPattern(WSP_ASTERISK);
    QVERIFY(rx.isValid());
    QVERIFY(rx.exactMatch(QLatin1String( " " )));
    QVERIFY(rx.exactMatch(""));
    QVERIFY(rx.exactMatch("   "));
    QVERIFY(rx.exactMatch("       "));

    rx.setPattern(QString(COMMA));
    QVERIFY(rx.isValid());
    QVERIFY(rx.exactMatch(","));
    QVERIFY(!rx.exactMatch(""));
    QVERIFY(!rx.exactMatch(" , "));

    rx.setPattern(COMMA_WSP);
    QVERIFY(rx.isValid());
    QVERIFY(rx.exactMatch(" ,"));
    QVERIFY(rx.exactMatch(" , "));
    QVERIFY(rx.exactMatch(QLatin1String( " " )));
    QVERIFY(rx.exactMatch(", "));
    QVERIFY(rx.exactMatch(","));
    QVERIFY(rx.exactMatch(" , "));
    QVERIFY(rx.exactMatch(" , "));

    rx.setPattern(NUMBER);
    QVERIFY(rx.isValid());
    QVERIFY(rx.exactMatch("6"));
    QVERIFY(rx.exactMatch("64"));
    QVERIFY(rx.exactMatch("5.2"));
    QVERIFY(rx.exactMatch("0.003"));
    QVERIFY(rx.exactMatch("5.1234e9"));
    QVERIFY(rx.exactMatch("5.1234e-9"));
    QVERIFY(rx.exactMatch("5.1234e-09"));
    QVERIFY(rx.exactMatch("-6"));
    QVERIFY(rx.exactMatch("+5.2"));
    QVERIFY(rx.exactMatch("-0.003"));
    QVERIFY(rx.exactMatch("-5.1234e9"));
    QVERIFY(rx.exactMatch("5.186907e-2"));
    QVERIFY(rx.exactMatch("-5.1234e-09"));
    QVERIFY(rx.exactMatch("444.71799"));
    QVERIFY(rx.exactMatch("16.30829"));
    QVERIFY(rx.exactMatch("0"));
    QVERIFY(rx.exactMatch("00002"));
    QVERIFY(rx.exactMatch(".00002"));

    rx.setPattern(OPEN_PARENS);
    QVERIFY(rx.isValid());
    QVERIFY(rx.exactMatch("("));
    QVERIFY(!rx.exactMatch(""));
    QVERIFY(!rx.exactMatch("( "));

    rx.setPattern(CLOSE_PARENS);
    QVERIFY(rx.isValid());
    QVERIFY(rx.exactMatch(")"));
    QVERIFY(!rx.exactMatch(""));
    QVERIFY(!rx.exactMatch(") "));

    rx.setPattern(MATRIX);
    QVERIFY(rx.isValid());
    QVERIFY(rx.exactMatch("matrix(1,0,0,1,0,0)"));
    QVERIFY(rx.exactMatch("matrix(5.186907e-2,0,0,5.186907e-2,444.71799,16.30829)"));
    QVERIFY(rx.exactMatch("matrix ( 5.186907e-2, 0,0, 5.186907e-2,444.71799,16.30829)"));
    QVERIFY(rx.exactMatch("matrix ( +5.186907e-2, 0 ,0, -5.186907e-2,444.71799,     16.30829)"));
    QVERIFY(!rx.exactMatch("matrix ( -5.186907e-2, 0 , -5.186907e-2,444.71799,      16.30829)"));
    QVERIFY(!rx.exactMatch("matrix ( +5.186907e-2, 0 ,0,, -5.186907e-2,444.71799,       16.30829)"));
    QVERIFY(rx.exactMatch("matrix ( +.5186907e-2, 0 ,0, -5.186907e-2,444.71799,     16.30829)"));

    rx.setPattern(TRANSLATE);
    QVERIFY(rx.isValid());
    QVERIFY(rx.exactMatch("translate(10)"));
    QVERIFY(rx.exactMatch("translate(-10.2345)"));
    QVERIFY(rx.exactMatch("translate(10, 23)"));
    QVERIFY(rx.exactMatch("translate(10 , -.765)"));
    QVERIFY(!rx.exactMatch("translate(10 , -.765.0)"));
    QVERIFY(!rx.exactMatch("translate(10 , ,-.7650)"));

    rx.setPattern(SCALE);
    QVERIFY(rx.isValid());
    QVERIFY(rx.exactMatch("scale(10)"));
    QVERIFY(rx.exactMatch("scale(-10.2345)"));
    QVERIFY(rx.exactMatch("scale(10, 23)"));
    QVERIFY(rx.exactMatch("scale(10 , -.765)"));
    QVERIFY(!rx.exactMatch("scale(10 , -.765.0)"));
    QVERIFY(!rx.exactMatch("scale(10 , ,-.7650)"));

    rx.setPattern(ROTATE);
    QVERIFY(rx.isValid());
    QVERIFY(rx.exactMatch("rotate(45)"));
    QVERIFY(rx.exactMatch("rotate(-26.765)"));
    QVERIFY(rx.exactMatch("rotate(-26.765, 10, 20)"));
    QVERIFY(rx.exactMatch("rotate(-26.765, 60, .20)"));
    QVERIFY(rx.exactMatch("rotate(-26.765, -10, 2.0)"));
    QVERIFY(!rx.exactMatch("rotate(-26.765, -10)"));

    rx.setPattern(SKEW_X);
    QVERIFY(rx.isValid());
    QVERIFY(rx.exactMatch("skewX(10)"));
    QVERIFY(rx.exactMatch("skewX(-210)"));
    QVERIFY(rx.exactMatch("skewX(+10.123456)"));
    QVERIFY(rx.exactMatch("skewX(0.00000345)"));

    rx.setPattern(SKEW_Y);
    QVERIFY(rx.isValid());
    QVERIFY(rx.exactMatch("skewY(10)"));
    QVERIFY(rx.exactMatch("skewY(-210)"));
    QVERIFY(rx.exactMatch("skewY(+10.12346e4)"));
    QVERIFY(rx.exactMatch("skewY(-0.00000345)"));

    rx.setPattern(TRANSFORM);
    QVERIFY(rx.isValid());
    QVERIFY(rx.exactMatch("matrix(5.186907e-2,0,0,5.186907e-2,444.71799,16.30829)"));
    QVERIFY(rx.exactMatch("translate(10, 23)"));
    QVERIFY(rx.exactMatch("scale(10 , -.765)"));
    QVERIFY(rx.exactMatch("rotate(-26.765, 60, .20)"));
    QVERIFY(rx.exactMatch("skewX(+10.123456)"));
    QVERIFY(rx.exactMatch("skewY(-0.00000345)"));
    QVERIFY(rx.exactMatch("skewY(-0.00000345)"));

    rx.setPattern(TRANSFORMS);
    QVERIFY(rx.isValid());
    QVERIFY(rx.exactMatch("matrix(5.186907e-2,0,0,5.186907e-2,444.71799,16.30829)"));
    QVERIFY(rx.exactMatch("rotate(-26.765, 60, .20)"));
    QVERIFY(rx.exactMatch("translate(-10,-20) scale(2) rotate(45) translate(5,10)"));
    QVERIFY(rx.exactMatch("translate(-10,-20), scale(2),rotate(45) translate(5,10), matrix(1,2,3,4,5,6)"));
}

void tst_KGameSvgDocument::transform()
{
    // Test transform set & get
    QString originalTransform = m_svgDom.transform();
    m_svgDom.setTransform(QStringLiteral("test"));
    QVERIFY(m_svgDom.transform() == "test");

    m_svgDom.setTransform(originalTransform);
    QVERIFY(m_svgDom.transform() == originalTransform);

    // test transformMartix() set & get
    QTransform originalMatrix = m_svgDom.transformMatrix();
    QTransform null = QTransform();
    m_svgDom.setTransformMatrix(null, KGameSvgDocument::ReplaceCurrentMatrix);
    QVERIFY(m_svgDom.transformMatrix() == null);

    m_svgDom.setTransformMatrix(originalMatrix, KGameSvgDocument::ReplaceCurrentMatrix);
    QVERIFY(m_svgDom.transformMatrix() == originalMatrix);

}
QTEST_MAIN(tst_KGameSvgDocument)

