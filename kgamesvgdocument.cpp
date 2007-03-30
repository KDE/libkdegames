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

#include <kfilterdev.h>

#include <QDebug>
#include <QFile>
#include <QDomNode>
#include <QStringList>
#include <QDomElement>
#include <QString>

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
     * @brief Instantiates a KGameSvgDocument object
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
     * @brief The ratio of a circle's radius to its circumference
     */
    static const double PI;

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
     * @brief The the filename of the SVG file to open.
     */
    QString m_svgFilename;

};

const double KGameSvgDocumentPrivate::PI = 3.14159265;
const QString KGameSvgDocumentPrivate::SVG_XML_PREPEND = QString("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?><svg>");
const QString KGameSvgDocumentPrivate::SVG_XML_APPEND = QString("</svg>");

KGameSvgDocument::KGameSvgDocument() : d(new KGameSvgDocumentPrivate)
{}

KGameSvgDocument::~KGameSvgDocument()
{
    delete d;
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
    return elementByUniqueAttributeValue("id", attributeValue);
}

void KGameSvgDocument::load()
{
    if (d->m_svgFilename.isNull()) {qDebug () << "Filename not specified.";}

    QFile file(d->m_svgFilename);
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }

    // Reads file whether it is compressed or not
    QIODevice *filter = KFilterDev::device( &file, QString::fromUtf8("application/x-gzip"), false);
    if (!filter)
    {
        return;
    }
    delete filter;

    if (!setContent(&file))
    {
        file.close();
        qDebug () << "DOM content not set.";
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
    double xRadians = xDegrees * (d->PI / 180);
    double yRadians = yDegrees * (d->PI / 180);

    shear(xRadians, yRadians, options);
}

void KGameSvgDocument::scale(double xFactor, double yFactor, const MatrixOptions& options)
{
    QMatrix matrix;

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
    QString s, xml;
    QTextStream str(&s);

    currentNode().save(str, 1);
    xml = *str.string();

    // Need to make node be a real svg document, so prepend and append required tags.
    xml = d->SVG_XML_PREPEND + xml + d->SVG_XML_APPEND;
    return xml;
}

QByteArray KGameSvgDocument::nodeToByteArray() const
{
    return nodeToSvg().toUtf8();
}

QString KGameSvgDocument::style() const
{
    return d->m_currentElement.attribute( "style", "Element has no style attribute.");
}

void KGameSvgDocument::setStyle(const QString& styleAttribute)
{
    d->m_currentElement.setAttribute("style", styleAttribute);
}

QString KGameSvgDocument::transform() const
{
    return d->m_currentElement.attribute( "transform", "Element has no transform attribute.");
}

void KGameSvgDocument::setTransform(const QString& transformAttribute)
{
    d->m_currentElement.setAttribute("transform", transformAttribute);
}

QHash<QString, QString> KGameSvgDocument::styleProperties() const
{
    QHash<QString, QString> stylePropertiesHash;
    QStringList styleProperties, keyValuePair;
    QString styleProperty;

    styleProperties = style().split(";");

    /* The style attr may have a trailing semi-colon.  If it does, split()
     * gives us an empty final element.  Remove it or we get 'index out of range' errors
     */
    if (styleProperties.at((styleProperties.count()-1)).isEmpty())
    {
        styleProperties.removeAt((styleProperties.count()-1));
    }

    for (int i = 0; i < styleProperties.size(); i++)
    {
        styleProperty = styleProperties.at(i);
        keyValuePair = styleProperty.split(":");
        stylePropertiesHash.insert(keyValuePair.at(0), keyValuePair.at(1));
    }
    return stylePropertiesHash;
}

void KGameSvgDocument::setStyleProperties(QHash<QString, QString> styleProperties, const StylePropertySortOptions& options)
{
    QString styleBuffer, property;

    d->m_inkscapeOrder << "fill" << "fill-opacity" << "fill-rule" << "stroke" << "stroke-width" << "stroke-linecap"
                       << "stroke-linejoin" << "stroke-miterlimit" << "stroke-dasharray" << "stroke-opacity";

    if (options == UseInkscapeOrder)
    {
        for (int i = 0; i < d->m_inkscapeOrder.size(); i++)
        {
            property = d->m_inkscapeOrder.at(i);
            if (styleProperties.contains(property))
            {
                styleBuffer += property + ':' + styleProperties.take(property) + ';';
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
            styleBuffer += it.key() + ':' + it.value() + ';';
        }
    }

    setStyle(styleBuffer);
}

QMatrix KGameSvgDocument::transformMatrix() const
{
    QString transformAttribute;

    transformAttribute = transform();

    QRegExp rx("matrix\\((.*),(.*),(.*),(.*),(.*),(.*)\\)");
    int result = rx.indexIn(transformAttribute);

    if (result == -1) // Matrix not found
    {
        return QMatrix();
    }
    else
    {
        return QMatrix(
            rx.cap(1).toDouble(), rx.cap(2).toDouble(), rx.cap(3).toDouble(),
            rx.cap(4).toDouble(), rx.cap(5).toDouble(), rx.cap(6).toDouble());
    }
}
void KGameSvgDocument::setTransformMatrix(QMatrix& matrix, const MatrixOptions& options)
{
    QString transformBuffer, tmp;

    if (options == ApplyToCurrentMatrix)
    {
        matrix = transformMatrix() * matrix;
    }

    transformBuffer = "matrix(";
    transformBuffer += tmp.setNum(matrix.m11()) + ',';
    transformBuffer += tmp.setNum(matrix.m12()) + ',';
    transformBuffer += tmp.setNum(matrix.m21()) + ',';
    transformBuffer += tmp.setNum(matrix.m22()) + ',';
    transformBuffer += tmp.setNum(matrix.dx()) + ',';
    transformBuffer += tmp.setNum(matrix.dy()) + ')';

    setTransform(transformBuffer);
}


//
// Private
//

QDomNode KGameSvgDocumentPrivate::findElementById(const QString& attributeName, const QString& attributeValue, const QDomNode& node)
{
    QDomElement e = node.toElement(); // try to convert the node to an element.
    QString value = e.attribute( attributeName, "Element has no attribute with that name.");

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
        //qDebug () << "No children or siblings.";
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

#include "kgamesvgdocument.moc"
