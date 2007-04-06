#include <QtTest>
#include <QtCore>

#include "kgamesvgdocumenttest.h"

void tst_KGameSvgDocument::initTestCase()
{
    m_svgDom.load("kgamesvgdocumenttest.svg");
}

void tst_KGameSvgDocument::cleanupTestCase()
{
    // Do nothing.
}

void tst_KGameSvgDocument::style()
{
    m_svgDom.elementById("digitBackground");
    QVERIFY(!m_svgDom.currentNode().isNull());

    // test style() set & get
    QString originalStyle = m_svgDom.style();
    m_svgDom.setStyle("test");
    QVERIFY(m_svgDom.style() == "test");

    m_svgDom.setStyle(originalStyle);
    QVERIFY(m_svgDom.style() == originalStyle);

    // test styleProperties() set & get
    QHash<QString, QString> originalStyleProperties = m_svgDom.styleProperties();
    m_svgDom.setStyle("test");
    QVERIFY(m_svgDom.style() == "test");

    m_svgDom.setStyleProperties(originalStyleProperties, KGameSvgDocument::UseInkscapeOrder);
    QVERIFY(m_svgDom.style() == originalStyle);

    // test styleProperty() set & get
    QString orgiginalStyleProperty = m_svgDom.styleProperty("fill");

    m_svgDom.setStyleProperty("fill", "ffffff");
    QVERIFY(m_svgDom.styleProperty("fill") == "ffffff");

    m_svgDom.setStyle(originalStyle);
    QVERIFY(m_svgDom.styleProperty("fill") == orgiginalStyleProperty);
}

void tst_KGameSvgDocument::scale()
{
    m_svgDom.elementById("0x11");
    QVERIFY(!m_svgDom.currentNode().isNull());

    QMatrix originalTransformMatrix = m_svgDom.transformMatrix();

    m_svgDom.scale(2, 2, KGameSvgDocument::ApplyToCurrentMatrix);

    QMatrix finalMatrix = m_svgDom.transformMatrix();

    double computedX = originalTransformMatrix.m11() * 2;
    double errorX = computedX - finalMatrix.m11();
    QVERIFY(errorX < 0.0000001);  // if our margin of error is small, we are probably ok

    double computedY = originalTransformMatrix.m22() * 2;
    double errorY = computedY - finalMatrix.m22();
    QVERIFY(errorY < 0.0000001);  // if our margin of error is small, we are probably ok

    // This should output a warning message
    m_svgDom.scale(1, 0, KGameSvgDocument::ApplyToCurrentMatrix);

}

QTEST_MAIN(tst_KGameSvgDocument)

#include "kgamesvgdocumenttest.moc"
