/*
    SPDX-FileCopyrightText: 2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgamesvgdocumenttest.h"
#include "kgamesvgdocument_p.h"

// own
#include <config-tests.h>
// Qt
#include <QRegularExpression>
#include <QTest>

void tst_KGameSvgDocument::initTestCase()
{
    m_svgDom.load(TESTS_PATH "kgamesvgdocumenttest.svg");
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
    QVERIFY(errorX < 0.0000001); // if our margin of error is small, we are probably ok

    double computedY = originalTransformMatrix.m22() * 2;
    double errorY = computedY - finalMatrix.m22();
    QVERIFY(errorY < 0.0000001); // if our margin of error is small, we are probably ok

    // This should output a warning message
    m_svgDom.scale(1, 0, KGameSvgDocument::ApplyToCurrentMatrix);
}

void tst_KGameSvgDocument::transformRegex()
{
    QRegularExpression rx;

    rx.setPattern(QRegularExpression::anchoredPattern(WSP));
    QVERIFY(rx.isValid());
    QVERIFY(rx.match(QLatin1String(" ")).hasMatch());
    QVERIFY(!rx.match("").hasMatch());

    rx.setPattern(QRegularExpression::anchoredPattern(WSP_ASTERISK));
    QVERIFY(rx.isValid());
    QVERIFY(rx.match(QLatin1String(" ")).hasMatch());
    QVERIFY(rx.match("").hasMatch());
    QVERIFY(rx.match("   ").hasMatch());
    QVERIFY(rx.match("       ").hasMatch());

    rx.setPattern(QRegularExpression::anchoredPattern(QString(COMMA)));
    QVERIFY(rx.isValid());
    QVERIFY(rx.match(",").hasMatch());
    QVERIFY(!rx.match("").hasMatch());
    QVERIFY(!rx.match(" , ").hasMatch());

    rx.setPattern(QRegularExpression::anchoredPattern(COMMA_WSP));
    QVERIFY(rx.isValid());
    QVERIFY(rx.match(" ,").hasMatch());
    QVERIFY(rx.match(" , ").hasMatch());
    QVERIFY(rx.match(QLatin1String(" ")).hasMatch());
    QVERIFY(rx.match(", ").hasMatch());
    QVERIFY(rx.match(",").hasMatch());
    QVERIFY(rx.match(" , ").hasMatch());
    QVERIFY(rx.match(" , ").hasMatch());

    rx.setPattern(QRegularExpression::anchoredPattern(NUMBER));
    QVERIFY(rx.isValid());
    QVERIFY(rx.match("6").hasMatch());
    QVERIFY(rx.match("64").hasMatch());
    QVERIFY(rx.match("5.2").hasMatch());
    QVERIFY(rx.match("0.003").hasMatch());
    QVERIFY(rx.match("5.1234e9").hasMatch());
    QVERIFY(rx.match("5.1234e-9").hasMatch());
    QVERIFY(rx.match("5.1234e-09").hasMatch());
    QVERIFY(rx.match("-6").hasMatch());
    QVERIFY(rx.match("+5.2").hasMatch());
    QVERIFY(rx.match("-0.003").hasMatch());
    QVERIFY(rx.match("-5.1234e9").hasMatch());
    QVERIFY(rx.match("5.186907e-2").hasMatch());
    QVERIFY(rx.match("-5.1234e-09").hasMatch());
    QVERIFY(rx.match("444.71799").hasMatch());
    QVERIFY(rx.match("16.30829").hasMatch());
    QVERIFY(rx.match("0").hasMatch());
    QVERIFY(rx.match("00002").hasMatch());
    QVERIFY(rx.match(".00002").hasMatch());

    rx.setPattern(QRegularExpression::anchoredPattern(OPEN_PARENS));
    QVERIFY(rx.isValid());
    QVERIFY(rx.match("(").hasMatch());
    QVERIFY(!rx.match("").hasMatch());
    QVERIFY(!rx.match("( ").hasMatch());

    rx.setPattern(QRegularExpression::anchoredPattern(CLOSE_PARENS));
    QVERIFY(rx.isValid());
    QVERIFY(rx.match(")").hasMatch());
    QVERIFY(!rx.match("").hasMatch());
    QVERIFY(!rx.match(") ").hasMatch());

    rx.setPattern(QRegularExpression::anchoredPattern(MATRIX));
    QVERIFY(rx.isValid());
    QVERIFY(rx.match("matrix(1,0,0,1,0,0)").hasMatch());
    QVERIFY(rx.match("matrix(5.186907e-2,0,0,5.186907e-2,444.71799,16.30829)").hasMatch());
    QVERIFY(rx.match("matrix ( 5.186907e-2, 0,0, 5.186907e-2,444.71799,16.30829)").hasMatch());
    QVERIFY(rx.match("matrix ( +5.186907e-2, 0 ,0, -5.186907e-2,444.71799,     16.30829)").hasMatch());
    QVERIFY(!rx.match("matrix ( -5.186907e-2, 0 , -5.186907e-2,444.71799,      16.30829)").hasMatch());
    QVERIFY(!rx.match("matrix ( +5.186907e-2, 0 ,0,, -5.186907e-2,444.71799,       16.30829)").hasMatch());
    QVERIFY(rx.match("matrix ( +.5186907e-2, 0 ,0, -5.186907e-2,444.71799,     16.30829)").hasMatch());

    rx.setPattern(QRegularExpression::anchoredPattern(TRANSLATE));
    QVERIFY(rx.isValid());
    QVERIFY(rx.match("translate(10)").hasMatch());
    QVERIFY(rx.match("translate(-10.2345)").hasMatch());
    QVERIFY(rx.match("translate(10, 23)").hasMatch());
    QVERIFY(rx.match("translate(10 , -.765)").hasMatch());
    QVERIFY(!rx.match("translate(10 , -.765.0)").hasMatch());
    QVERIFY(!rx.match("translate(10 , ,-.7650)").hasMatch());

    rx.setPattern(QRegularExpression::anchoredPattern(SCALE));
    QVERIFY(rx.isValid());
    QVERIFY(rx.match("scale(10)").hasMatch());
    QVERIFY(rx.match("scale(-10.2345)").hasMatch());
    QVERIFY(rx.match("scale(10, 23)").hasMatch());
    QVERIFY(rx.match("scale(10 , -.765)").hasMatch());
    QVERIFY(!rx.match("scale(10 , -.765.0)").hasMatch());
    QVERIFY(!rx.match("scale(10 , ,-.7650)").hasMatch());

    rx.setPattern(QRegularExpression::anchoredPattern(ROTATE));
    QVERIFY(rx.isValid());
    QVERIFY(rx.match("rotate(45)").hasMatch());
    QVERIFY(rx.match("rotate(-26.765)").hasMatch());
    QVERIFY(rx.match("rotate(-26.765, 10, 20)").hasMatch());
    QVERIFY(rx.match("rotate(-26.765, 60, .20)").hasMatch());
    QVERIFY(rx.match("rotate(-26.765, -10, 2.0)").hasMatch());
    QVERIFY(!rx.match("rotate(-26.765, -10)").hasMatch());

    rx.setPattern(QRegularExpression::anchoredPattern(SKEW_X));
    QVERIFY(rx.isValid());
    QVERIFY(rx.match("skewX(10)").hasMatch());
    QVERIFY(rx.match("skewX(-210)").hasMatch());
    QVERIFY(rx.match("skewX(+10.123456)").hasMatch());
    QVERIFY(rx.match("skewX(0.00000345)").hasMatch());

    rx.setPattern(QRegularExpression::anchoredPattern(SKEW_Y));
    QVERIFY(rx.isValid());
    QVERIFY(rx.match("skewY(10)").hasMatch());
    QVERIFY(rx.match("skewY(-210)").hasMatch());
    QVERIFY(rx.match("skewY(+10.12346e4)").hasMatch());
    QVERIFY(rx.match("skewY(-0.00000345)").hasMatch());

    rx.setPattern(QRegularExpression::anchoredPattern(TRANSFORM));
    QVERIFY(rx.isValid());
    QVERIFY(rx.match("matrix(5.186907e-2,0,0,5.186907e-2,444.71799,16.30829)").hasMatch());
    QVERIFY(rx.match("translate(10, 23)").hasMatch());
    QVERIFY(rx.match("scale(10 , -.765)").hasMatch());
    QVERIFY(rx.match("rotate(-26.765, 60, .20)").hasMatch());
    QVERIFY(rx.match("skewX(+10.123456)").hasMatch());
    QVERIFY(rx.match("skewY(-0.00000345)").hasMatch());
    QVERIFY(rx.match("skewY(-0.00000345)").hasMatch());

    rx.setPattern(QRegularExpression::anchoredPattern(TRANSFORMS));
    QVERIFY(rx.isValid());
    QVERIFY(rx.match("matrix(5.186907e-2,0,0,5.186907e-2,444.71799,16.30829)").hasMatch());
    QVERIFY(rx.match("rotate(-26.765, 60, .20)").hasMatch());
    QVERIFY(rx.match("translate(-10,-20) scale(2) rotate(45) translate(5,10)").hasMatch());
    QVERIFY(rx.match("translate(-10,-20), scale(2),rotate(45) translate(5,10), matrix(1,2,3,4,5,6)").hasMatch());
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

#include "moc_kgamesvgdocumenttest.cpp"
