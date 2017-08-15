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

#include "kgamesvgdocument.h"
#include "kgamesvgdocument_p.h"

#include <kfilterdev.h>

#include <QBuffer>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QDomElement>
#include <QDomNode>
#include <QDebug>

#include <math.h>

//
// Public
//

/**
 * @brief A class holding private members for KGameSvgDocument
 *
 * @see KGameSvgDocument
 * @author Mark A. Taff \<kde@marktaff.com\>
 * @version 0.1
 */
class KGameSvgDocumentPrivate
{
    public:

    /**
     * @brief Instantiates a KGameSvgDocumentPrivate object
     */
    KGameSvgDocumentPrivate()
    {}

    ~KGameSvgDocumentPrivate()
    {}

    /**
     * @brief Performs a preorder traversal of the DOM tree to find element matching @c attributeName & @c attributeValue
     *
     * @param attributeName The name of the attribute to find
     * @param attributeValue The value of the @p attributeName attribute to find
     * @param node The node to start the traversal from.
     * @returns the node with id of @c elementId.  If no node has that id, returns a null node.
     */
    QDomNode findElementById(const QString& attributeName, const QString& attributeValue, const QDomNode& node);

    /**
     * @brief Returns the current element
     * @returns The current element
     */
    QDomElement currentElement() const;

    /**
     * @brief Sets the current element
     *
     * @returns nothing
     */
    void setCurrentElement();

    /**
     * @brief Returns whether the original style attribute has a trailing semicolon
     * @returns whether the original style attribute has a trailing semicolon
     */
    bool styleHasTrailingSemicolon() const;

    /**
     * @brief Sets whether the original style attribute has a trailing semicolon
     *
     * @param hasSemicolon whether the original style attribute has a trailing semicolon
     * @returns nothing
     */
    void setStyleHasTrailingSemicolon(bool hasSemicolon);

    /**
     * @brief The last node found by elementById, or a null node if not found.
     */
    QDomNode m_currentNode;

    /**
     * @brief The current node turned into an element.
     */
    QDomElement m_currentElement;

    /**
     * @brief The order Inkscape write properties in the style attribute of an element.
     *
     * Inkscape order is defined as:
     * "fill", "fill-opacity", "fill-rule", "stroke", "stroke-width", "stroke-linecap",
     * "stroke-linejoin", "stroke-miterlimit", "stroke-dasharray", "stroke-opacity"
     */
    QStringList m_inkscapeOrder;

    /**
     * @brief The xml that must be prepended to a node to make it a valid svg document
     *
     * Defined as: <?xml version="1.0" encoding="UTF-8" standalone="no"?>\<svg\>
     */
    static const QString SVG_XML_PREPEND;

    /**
     * @brief The xml that must be appended to a node to make it a valid svg document
     *
     * Defined as: \</svg\>
     */
    static const QString SVG_XML_APPEND;

    /**
     * @brief The filename of the SVG file to open.
     */
    QString m_svgFilename;

    /**
     * @brief Whether the style attribute has a trailing semicolon
     */
    bool m_hasSemicolon;


};

const QString KGameSvgDocumentPrivate::SVG_XML_PREPEND = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?><svg>");
const QString KGameSvgDocumentPrivate::SVG_XML_APPEND = QStringLiteral("</svg>");

KGameSvgDocument::KGameSvgDocument()
    : QDomDocument(), d(new KGameSvgDocumentPrivate)
{}

KGameSvgDocument::KGameSvgDocument(const KGameSvgDocument &doc)
    : QDomDocument(), d(new KGameSvgDocumentPrivate(*doc.d))
{
}

KGameSvgDocument::~KGameSvgDocument()
{
    delete d;
}

KGameSvgDocument& KGameSvgDocument::operator=(const KGameSvgDocument &doc)
{
    QDomDocument::operator=(doc);
    *d = *doc.d;
    return *this;
}

QDomNode KGameSvgDocument::elementByUniqueAttributeValue(const QString& attributeName, const QString& attributeValue)
{
    /* DOM is always "live", so there maybe a new root node.  We always have to ask for the
     * root node instead of keeping a pointer to it.
     */
    QDomElement docElem = documentElement();
    QDomNode n = docElem.firstChild();

    QDomNode node = d->findElementById(attributeName, attributeValue, n);
    setCurrentNode(node);
    return node;
}

QDomNode KGameSvgDocument::elementById(const QString& attributeValue)
{
    return elementByUniqueAttributeValue(QStringLiteral( "id" ), attributeValue);
}

void KGameSvgDocument::load()
{
    if (d->m_svgFilename.isNull())
    {
        qCDebug(GAMES_LIB) << "KGameSvgDocument::load(): Filename not specified.";
        return;
    }

    QFile file(d->m_svgFilename);
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }
    QByteArray content = file.readAll();

    // If the file is compressed, decompress the contents before loading it.
    if (!content.startsWith("<?xml"))
    {
        QBuffer buf(&content);
        KCompressionDevice::CompressionType type = KFilterDev::compressionTypeForMimeType(QStringLiteral("application/x-gzip"));
	KCompressionDevice flt(&buf, false, type);
	if (!flt.open(QIODevice::ReadOnly))
        {
            flt.close();
            return;
        }
        QByteArray ar = flt.readAll();
        flt.close();
        content = ar;
    }

    if (!setContent(content))
    {
        file.close();
        qCDebug(GAMES_LIB) << "DOM content not set.";
        return;
    }
    file.close();
}

void KGameSvgDocument::load(const QString& svgFilename)
{
    setSvgFilename(svgFilename);
    load();
}

void KGameSvgDocument::rotate(double degrees, const MatrixOptions& options)
{
    QMatrix matrix;

    if (options == ApplyToCurrentMatrix)
    {
        matrix = transformMatrix().QMatrix::rotate(degrees);
    }
    else
    {
        matrix = QMatrix();
        matrix.QMatrix::rotate(degrees);
    }
    setTransformMatrix(matrix, ReplaceCurrentMatrix);
}

void KGameSvgDocument::translate(int xPixels, int yPixels, const MatrixOptions& options)
{
    QMatrix matrix;

    if (options == ApplyToCurrentMatrix)
    {
        matrix = transformMatrix().QMatrix::translate(xPixels, yPixels);
    }
    else
    {
        matrix = QMatrix();
        matrix.QMatrix::translate(xPixels, yPixels);
    }
    setTransformMatrix(matrix, ReplaceCurrentMatrix);
}

void KGameSvgDocument::shear(double xRadians, double yRadians, const MatrixOptions& options)
{
    QMatrix matrix;

    if (options == ApplyToCurrentMatrix)
    {
        matrix = transformMatrix().QMatrix::shear(xRadians, yRadians);
    }
    else
    {
        matrix = QMatrix();
        matrix.QMatrix::shear(xRadians, yRadians);
    }
    setTransformMatrix(matrix, ReplaceCurrentMatrix);
}

void KGameSvgDocument::skew(double xDegrees, double yDegrees, const MatrixOptions& options)
{
    double xRadians = xDegrees * (M_PI / 180);
    double yRadians = yDegrees * (M_PI / 180);

    shear(xRadians, yRadians, options);
}

void KGameSvgDocument::scale(double xFactor, double yFactor, const MatrixOptions& options)
{
    QMatrix matrix;
    if ((xFactor == 0) || (yFactor == 0))
    {
        qWarning () << "KGameSvgDocument::scale: You cannnot scale by zero";
    }

    if (options == ApplyToCurrentMatrix)
    {
        matrix = transformMatrix().QMatrix::scale(xFactor, yFactor);
    }
    else
    {
        matrix = QMatrix();
        matrix.QMatrix::scale(xFactor, yFactor);
    }
    setTransformMatrix(matrix, ReplaceCurrentMatrix);
}

QDomNode KGameSvgDocument::currentNode() const
{
    return d->m_currentNode;
}

void KGameSvgDocument::setCurrentNode(const QDomNode& node)
{
    d->m_currentNode = node;
    d->setCurrentElement();
}

QString KGameSvgDocument::svgFilename() const
{
    return d->m_svgFilename;
}

void KGameSvgDocument::setSvgFilename(const QString& svgFilename)
{
    d->m_svgFilename = svgFilename;
}

QString KGameSvgDocument::styleProperty(const QString& propertyName) const
{
    return styleProperties().value(propertyName);
}

void KGameSvgDocument::setStyleProperty(const QString& propertyName, const QString& propertyValue)
{
    QHash<QString, QString> properties;

    properties = styleProperties();
    properties.insert(propertyName, propertyValue);

    setStyleProperties(properties, UseInkscapeOrder);
}

QString KGameSvgDocument::nodeToSvg() const
{
    QString s, t, xml, defs, pattern;
    QTextStream str(&s);
    QTextStream str_t(&t);
    QStringList defsAdded;
    int result = 0;
    QRegExp rx;

    currentNode().save(str, 1);
    xml = *str.string();

    // Find and add any required gradients or patterns
    pattern = QLatin1String( "url" ) + WSP_ASTERISK + OPEN_PARENS + WSP_ASTERISK + QLatin1String( "#(.*)" ) + WSP_ASTERISK + CLOSE_PARENS;
    rx.setPattern(pattern);
    if (rx.indexIn(xml, result) != -1)
    {
        QDomNode node, nodeBase;
        QString baseId;
        QDomNode n = def();

        result = 0;
        while ((result = rx.indexIn(xml, result)) != -1)
        {
            // Find the pattern or gradient referenced
            result += rx.matchedLength();
            if (!defsAdded.contains(rx.cap(1)))
            {
                node = d->findElementById(QStringLiteral( "id" ), rx.cap(1), n);
                node.save(str_t, 1);
                defsAdded.append(rx.cap(1));
            }

            // Find the gradient the above gradient is based on
            baseId = node.toElement().attribute(QStringLiteral( "xlink:href" )).mid(1);
            if (!defsAdded.contains(baseId))
            {
                nodeBase = d->findElementById(QStringLiteral( "id" ), baseId, n);
                nodeBase.save(str_t, 1);
                defsAdded.append(baseId);
            }
        }
        defs = *str_t.string();
        defs = QLatin1String( "<defs>" ) + defs + QLatin1String( "</defs>" );
    }

    // Need to make node be a real svg document, so prepend and append required tags.
    xml = d->SVG_XML_PREPEND + defs + xml + d->SVG_XML_APPEND;
    return xml;
}

QByteArray KGameSvgDocument::nodeToByteArray() const
{
    return nodeToSvg().toUtf8();
}

QString KGameSvgDocument::style() const
{
    return d->m_currentElement.attribute( QStringLiteral( "style" ), QStringLiteral( "Element has no style attribute." ));
}

void KGameSvgDocument::setStyle(const QString& styleAttribute)
{
    d->m_currentElement.setAttribute(QStringLiteral( "style" ), styleAttribute);
}

QDomNodeList KGameSvgDocument::patterns() const
{
    return elementsByTagName(QStringLiteral( "pattern" ));
}

QDomNodeList KGameSvgDocument::linearGradients() const
{
    return elementsByTagName(QStringLiteral( "linearGradient" ));
}

QDomNodeList KGameSvgDocument::radialGradients() const
{
    return elementsByTagName(QStringLiteral( "radialGradient" ));
}

QDomNodeList KGameSvgDocument::defs() const
{
    return elementsByTagName(QStringLiteral( "defs" ));
}

QDomNode KGameSvgDocument::def() const
{
    return defs().at(0);
}

QString KGameSvgDocument::transform() const
{
    return d->m_currentElement.attribute( QStringLiteral( "transform" ), QStringLiteral( "Element has no transform attribute." ) );
}

void KGameSvgDocument::setTransform(const QString& transformAttribute)
{
    d->m_currentElement.setAttribute(QStringLiteral( "transform" ), transformAttribute);
}

QHash<QString, QString> KGameSvgDocument::styleProperties() const
{
    QHash<QString, QString> stylePropertiesHash;
    QStringList styleProperties, keyValuePair;
    QString styleProperty;

    styleProperties = style().split(QLatin1Char( ';' ));

    /* The style attr may have a trailing semi-colon.  If it does, split()
     * gives us an empty final element.  Remove it or we get 'index out of range' errors
     */
    if (styleProperties.at((styleProperties.count()-1)).isEmpty())
    {
        styleProperties.removeAt((styleProperties.count()-1));
        d->setStyleHasTrailingSemicolon(true);
    }
    else {d->setStyleHasTrailingSemicolon(false);}

    for (int i = 0; i < styleProperties.size(); i++)
    {
        styleProperty = styleProperties.at(i);
        keyValuePair = styleProperty.split(QLatin1Char( ':' ));
        stylePropertiesHash.insert(keyValuePair.at(0), keyValuePair.at(1));
    }
    return stylePropertiesHash;
}

void KGameSvgDocument::setStyleProperties(const QHash<QString, QString>& _styleProperties, const StylePropertySortOptions& options)
{
    QHash<QString, QString> styleProperties = _styleProperties;
    QString styleBuffer, property;

    d->m_inkscapeOrder << QStringLiteral( "fill" ) << QStringLiteral( "fill-opacity" ) << QStringLiteral( "fill-rule" ) << QStringLiteral( "stroke" ) << QStringLiteral( "stroke-width" ) << QStringLiteral( "stroke-linecap" )
                       << QStringLiteral( "stroke-linejoin" ) << QStringLiteral( "stroke-miterlimit" ) << QStringLiteral( "stroke-dasharray" ) << QStringLiteral( "stroke-opacity" );

    if (options == UseInkscapeOrder)
    {
        for (int i = 0; i < d->m_inkscapeOrder.size(); i++)
        {
            property = d->m_inkscapeOrder.at(i);
            if (styleProperties.contains(property))
            {
                styleBuffer += property + QLatin1Char( ':' ) + styleProperties.take(property) + QLatin1Char( ';' );
            }
            else
            {
                // Do Nothing
            }
        }
    }

    // Append any style properties
    if (!styleProperties.isEmpty())
    {
        QHashIterator<QString,  QString> it(styleProperties);
        while (it.hasNext())
        {
            it.next();
            styleBuffer += it.key() + QLatin1Char( ':' ) + it.value() + QLatin1Char( ';' );
        }
    }

    // Remove trailing semicolon if original didn't have one
    if (!d->styleHasTrailingSemicolon()) {styleBuffer.chop(1);}
    setStyle(styleBuffer);
}

QMatrix KGameSvgDocument::transformMatrix() const
{
    /*
     * Transform attributes can be quite complex.  Here, we assemble this tangled web of
     * complexity into an single matrix.
     *
     * The regex's that make this bearable live in kgamesvgdocument_p.h.  As these regex's
     * get quite complex, we have some code in tests/kgamesvgdocumenttest.cpp to help verify
     * they are still correct after being edited.
     *
     * Warning: This code depends on the capturing parenthesis in the regex's not changing.
     *
     * For all the gory details, see http://www.w3.org/TR/SVG/coords.html#TransformAttribute
     */
    QRegExp rx;
    QString transformAttribute;
    int result;
    int i = 0;
    QMatrix baseMatrix = QMatrix();

    transformAttribute = transform();
    if (transformAttribute == QLatin1String( "Element has no transform attribute." ))
    {
        return QMatrix();
    }
    transformAttribute.trimmed();

    rx.setPattern(TRANSFORMS);
    if (!rx.exactMatch(transformAttribute))
    {
        qWarning () << "Transform attribute seems to be invalid. Check your SVG file.";
        return QMatrix();
    }

    rx.setPattern(TRANSFORM);

    while (transformAttribute.size() > 0 && i < 32) // 32 is an arbitrary limit for the number of transforms for a single node
    {
        result = rx.indexIn(transformAttribute);
        if (result != -1) // Found left-most transform
        {
            if (rx.cap(1) == QLatin1String( "matrix" ))
            {
                // If the first transform found is a matrix, use it as the base,
                // else we use a null matrix.
                if (i == 0)
                {
                    baseMatrix = QMatrix(rx.cap(2).toDouble(), rx.cap(3).toDouble(), rx.cap(4).toDouble(),
                                         rx.cap(5).toDouble(), rx.cap(6).toDouble(), rx.cap(7).toDouble());
                }
                else
                {
                    baseMatrix = QMatrix(rx.cap(2).toDouble(), rx.cap(3).toDouble(), rx.cap(4).toDouble(),
                                         rx.cap(5).toDouble(), rx.cap(6).toDouble(), rx.cap(7).toDouble()) * baseMatrix;
                }
            }

            if (rx.cap(8) == QLatin1String( "translate" ))
            {
                double x = rx.cap(9).toDouble();
                double y = rx.cap(10).toDouble();
                if (rx.cap(10).isEmpty()) // y defaults to zero per SVG standard
                {
                    y = 0;
                }
                baseMatrix = baseMatrix.translate(x, y);
            }

            if (rx.cap(11) == QLatin1String( "scale" ))
            {
                double x = rx.cap(12).toDouble();
                double y = rx.cap(12).toDouble();
                if (rx.cap(13).isEmpty()) // y defaults to x per SVG standard
                {
                    y = x;
                }
                baseMatrix = baseMatrix.scale(x, y);
            }

            if (rx.cap(14) == QLatin1String( "rotate" ))
            {
                double a = rx.cap(15).toDouble();
                double cx = rx.cap(16).toDouble();
                double cy = rx.cap(17).toDouble();

                if ((cx > 0) || (cy > 0)) // rotate around point (cx, cy)
                {
                    baseMatrix.translate(cx, cy);
                    baseMatrix.rotate(a);
                    baseMatrix.translate((cx * -1), (cy * -1));
                }
                else
                {
                    baseMatrix = baseMatrix.rotate(a); // rotate around origin
                }
            }

            if (rx.cap(18) == QLatin1String( "skewX" ))
            {
                baseMatrix = baseMatrix.shear(rx.cap(19).toDouble() * (M_PI / 180), 0);
            }

            if (rx.cap(20) == QLatin1String( "skewY" ))
            {
                baseMatrix = baseMatrix.shear(0, rx.cap(21).toDouble() * (M_PI / 180));
            }
        }
        transformAttribute = transformAttribute.mid(rx.matchedLength() + result);
        i++;
    }

    return baseMatrix;
}

void KGameSvgDocument::setTransformMatrix(QMatrix& matrix, const MatrixOptions& options)
{
    QString transformBuffer, tmp;
    QMatrix null = QMatrix();

    if (options == ApplyToCurrentMatrix)
    {
        matrix = transformMatrix() * matrix;
    }

    transformBuffer = QStringLiteral( "matrix(" );
    transformBuffer += tmp.setNum(matrix.m11(),'g',7) + QLatin1Char( ',' );
    transformBuffer += tmp.setNum(matrix.m12(),'g',7) + QLatin1Char( ',' );
    transformBuffer += tmp.setNum(matrix.m21(),'g',7) + QLatin1Char( ',' );
    transformBuffer += tmp.setNum(matrix.m22(),'g',7) + QLatin1Char( ',' );
    transformBuffer += tmp.setNum(matrix.dx(),'g',7) + QLatin1Char( ',' );
    transformBuffer += tmp.setNum(matrix.dy(),'g',7) + QLatin1Char( ')' );

    if ((transform() == QLatin1String( "Element has no transform attribute." )) && (matrix == null))
    {
        // Do not write a meaningless matrix to DOM
    }
    else
    {
        setTransform(transformBuffer);
    }
}


//
// Private
//

QDomNode KGameSvgDocumentPrivate::findElementById(const QString& attributeName, const QString& attributeValue, const QDomNode& node)
{
    QDomElement e = node.toElement(); // try to convert the node to an element.
    QString value = e.attribute( attributeName, QStringLiteral( "Element has no attribute with that name." ));

    if (value == attributeValue)
    {
        // We found our node.  Stop recursion and return it.
        return node;
    }

    if (!node.firstChild().isNull())
    {
        QDomNode result = findElementById(attributeName, attributeValue, node.firstChild());
        /** We have recursed, now we need to have this recursion end when
         * the function call above returns
         */
        if (!result.isNull()) return result; // If we found the node with id, then return it
    }
    if (!node.nextSibling().isNull())
    {
        QDomNode result = findElementById(attributeName, attributeValue, node.nextSibling());
        /** We have recursed, now we need to have this recursion end when
         * the function call above returns */
        if (!result.isNull()) return result;
    }
    if (!node.firstChild().isNull() && !node.nextSibling().isNull())
    {
        // Do Nothing
        //qCDebug(GAMES_LIB) << "No children or siblings.";
    }

    // Matching node not found, so return a null node.
    return QDomNode();
}

QDomElement KGameSvgDocumentPrivate::currentElement() const
{
    return m_currentElement;
}

void KGameSvgDocumentPrivate::setCurrentElement()
{
    m_currentElement = m_currentNode.toElement();
}

bool KGameSvgDocumentPrivate::styleHasTrailingSemicolon() const
{
    return m_hasSemicolon;
}

void KGameSvgDocumentPrivate::setStyleHasTrailingSemicolon(bool hasSemicolon)
{
    m_hasSemicolon = hasSemicolon;
}

