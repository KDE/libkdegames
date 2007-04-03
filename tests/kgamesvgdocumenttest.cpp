#include <QtTest>
#include <QtCore>

#include "kgamesvgdocumenttest.h"


/// @brief We can't scale an element into nonexistance
void tst_KGameSvgDocument::noScalingByZero()
{
	KGameSvgDocument svgDom;
	svgDom.load("kgamesvgdocumenttest.svg");
	svgDom.scale(0, 0, KGameSvgDocument::ApplyToCurrentMatrix);
	// Performs no tests, just skeleton for now
}


QTEST_KDEMAIN(tst_KGameSvgDocument, NoGUI)

#include "kgamesvgdocumenttest.moc"
