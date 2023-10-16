/*
    SPDX-FileCopyrightText: 2007 Mark A. Taff <kde@marktaff.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kgamesvgdocument.h"
#include "kgamesvgdocument_p.h"

// own
#include <kdegamesprivate_logging.h>
// KF
#include <KCompressionDevice>
// Qt
#include <QBuffer>
#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QRegularExpression>
#include <QString>
// Std
#include <cmath>

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
    {
    }

    ~KGameSvgDocumentPrivate()
    {
    }

    /**
     * @brief Performs a preorder traversal of the DOM tree to find element matching @c attributeName & @c attributeValue
     *
     * @param attributeName The name of the attribute to find
     * @param attributeValue The value of the @p attributeName attribute to find
     * @param node The node to start the traversal from.
     * @returns the node with id of @c elementId.  If no node has that id, returns a null node.
     */
    QDomNode findElementById(const QString &attributeName, const QString &attributeValue, const QDomNode &node);

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
    : QDomDocument()
    , d(new KGameSvgDocumentPrivate)
{
}

KGameSvgDocument::KGameSvgDocument(const KGameSvgDocument &doc)
    : QDomDocument()
    , d(new KGameSvgDocumentPrivate(*doc.d))
{
}

KGameSvgDocument::~KGameSvgDocument() = default;

KGameSvgDocument &KGameSvgDocument::operator=(const KGameSvgDocument &doc)
{
    QDomDocument::operator=(doc);
    *d = *doc.d;
    return *this;
}

QDomNode KGameSvgDocument::elementByUniqueAttributeValue(const QString &attributeName, const QString &attributeValue)
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

QDomNode KGameSvgDocument::elementById(const QString &attributeValue)
{
    return elementByUniqueAttributeValue(QStringLiteral("id"), attributeValue);
}

void KGameSvgDocument::load()
{
    if (d->m_svgFilename.isEmpty()) {
        qCDebug(KDEGAMESPRIVATE_LOG) << "KGameSvgDocument::load(): Filename not specified.";
        return;
    }

    QFile file(d->m_svgFilename);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    QByteArray content = file.readAll();

    // If the file is compressed, decompress the contents before loading it.
    if (!content.startsWith("<?xml")) // krazy:exclude=strings
    {
        QBuffer buf(&content);
        KCompressionDevice flt(&buf, /*autoDeleteInputDevice*/ false, KCompressionDevice::GZip);
        if (!flt.open(QIODevice::ReadOnly)) {
            flt.close();
            return;
        }
        QByteArray ar = flt.readAll();
        flt.close();
        content = ar;
    }

    if (!setContent(content)) {
        file.close();
        qCDebug(KDEGAMESPRIVATE_LOG) << "DOM content not set.";
        return;
    }
    file.close();
}

void KGameSvgDocument::load(const QString &svgFilename)
{
    setSvgFilename(svgFilename);
    load();
}

void KGameSvgDocument::rotate(double degrees, MatrixOptions options)
{
    QTransform matrix;

    if (options == ApplyToCurrentMatrix) {
        matrix = transformMatrix().QTransform::rotate(degrees);
    } else {
        matrix = QTransform();
        matrix.QTransform::rotate(degrees);
    }
    setTransformMatrix(matrix, ReplaceCurrentMatrix);
}

void KGameSvgDocument::translate(int xPixels, int yPixels, MatrixOptions options)
{
    QTransform matrix;

    if (options == ApplyToCurrentMatrix) {
        matrix = transformMatrix().QTransform::translate(xPixels, yPixels);
    } else {
        matrix = QTransform();
        matrix.QTransform::translate(xPixels, yPixels);
    }
    setTransformMatrix(matrix, ReplaceCurrentMatrix);
}

void KGameSvgDocument::shear(double xRadians, double yRadians, MatrixOptions options)
{
    QTransform matrix;

    if (options == ApplyToCurrentMatrix) {
        matrix = transformMatrix().QTransform::shear(xRadians, yRadians);
    } else {
        matrix = QTransform();
        matrix.QTransform::shear(xRadians, yRadians);
    }
    setTransformMatrix(matrix, ReplaceCurrentMatrix);
}

void KGameSvgDocument::skew(double xDegrees, double yDegrees, MatrixOptions options)
{
    double xRadians = xDegrees * (M_PI / 180);
    double yRadians = yDegrees * (M_PI / 180);

    shear(xRadians, yRadians, options);
}

void KGameSvgDocument::scale(double xFactor, double yFactor, MatrixOptions options)
{
    QTransform matrix;
    if ((xFactor == 0) || (yFactor == 0)) {
        qCWarning(KDEGAMESPRIVATE_LOG) << "KGameSvgDocument::scale: You cannot scale by zero";
    }

    if (options == ApplyToCurrentMatrix) {
        matrix = transformMatrix().QTransform::scale(xFactor, yFactor);
    } else {
        matrix = QTransform();
        matrix.QTransform::scale(xFactor, yFactor);
    }
    setTransformMatrix(matrix, ReplaceCurrentMatrix);
}

QDomNode KGameSvgDocument::currentNode() const
{
    return d->m_currentNode;
}

void KGameSvgDocument::setCurrentNode(const QDomNode &node)
{
    d->m_currentNode = node;
    d->setCurrentElement();
}

QString KGameSvgDocument::svgFilename() const
{
    return d->m_svgFilename;
}

void KGameSvgDocument::setSvgFilename(const QString &svgFilename)
{
    d->m_svgFilename = svgFilename;
}

QString KGameSvgDocument::styleProperty(const QString &propertyName) const
{
    return styleProperties().value(propertyName);
}

void KGameSvgDocument::setStyleProperty(const QString &propertyName, const QString &propertyValue)
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
    QRegularExpression rx;

    currentNode().save(str, 1);
    xml = *str.string();

    // Find and add any required gradients or patterns
    pattern = QLatin1String("url") + WSP_ASTERISK + OPEN_PARENS + WSP_ASTERISK + QLatin1String("#(.*)") + WSP_ASTERISK + CLOSE_PARENS;
    rx.setPattern(pattern);
    if (rx.match(xml).hasMatch()) {
        QDomNode node, nodeBase;
        QString baseId;
        QDomNode n = def();

        QRegularExpressionMatchIterator i = rx.globalMatch(xml);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            const QString id = match.captured(1);
            if (!defsAdded.contains(id)) {
                node = d->findElementById(QStringLiteral("id"), id, n);
                node.save(str_t, 1);
                defsAdded.append(id);
            }

            // Find the gradient the above gradient is based on
            baseId = node.toElement().attribute(QStringLiteral("xlink:href")).mid(1);
            if (!defsAdded.contains(baseId)) {
                nodeBase = d->findElementById(QStringLiteral("id"), baseId, n);
                nodeBase.save(str_t, 1);
                defsAdded.append(baseId);
            }
        }
        defs = *str_t.string();
        defs = QLatin1String("<defs>") + defs + QLatin1String("</defs>");
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
    return d->m_currentElement.attribute(QStringLiteral("style"), QStringLiteral("Element has no style attribute."));
}

void KGameSvgDocument::setStyle(const QString &styleAttribute)
{
    d->m_currentElement.setAttribute(QStringLiteral("style"), styleAttribute);
}

QDomNodeList KGameSvgDocument::patterns() const
{
    return elementsByTagName(QStringLiteral("pattern"));
}

QDomNodeList KGameSvgDocument::linearGradients() const
{
    return elementsByTagName(QStringLiteral("linearGradient"));
}

QDomNodeList KGameSvgDocument::radialGradients() const
{
    return elementsByTagName(QStringLiteral("radialGradient"));
}

QDomNodeList KGameSvgDocument::defs() const
{
    return elementsByTagName(QStringLiteral("defs"));
}

QDomNode KGameSvgDocument::def() const
{
    return defs().at(0);
}

QString KGameSvgDocument::transform() const
{
    return d->m_currentElement.attribute(QStringLiteral("transform"), QStringLiteral("Element has no transform attribute."));
}

void KGameSvgDocument::setTransform(const QString &transformAttribute)
{
    d->m_currentElement.setAttribute(QStringLiteral("transform"), transformAttribute);
}

QHash<QString, QString> KGameSvgDocument::styleProperties() const
{
    QHash<QString, QString> stylePropertiesHash;
    QStringList styleProperties, keyValuePair;

    styleProperties = style().split(QLatin1Char(';'));

    /* The style attr may have a trailing semi-colon.  If it does, split()
     * gives us an empty final element.  Remove it or we get 'index out of range' errors
     */
    if (styleProperties.at((styleProperties.count() - 1)).isEmpty()) {
        styleProperties.removeAt((styleProperties.count() - 1));
        d->setStyleHasTrailingSemicolon(true);
    } else {
        d->setStyleHasTrailingSemicolon(false);
    }

    for (const QString &styleProperty : std::as_const(styleProperties)) {
        keyValuePair = styleProperty.split(QLatin1Char(':'));
        stylePropertiesHash.insert(keyValuePair.at(0), keyValuePair.at(1));
    }
    return stylePropertiesHash;
}

void KGameSvgDocument::setStyleProperties(const QHash<QString, QString> &_styleProperties, const StylePropertySortOptions &options)
{
    QHash<QString, QString> styleProperties = _styleProperties;
    QString styleBuffer;

    d->m_inkscapeOrder << QStringLiteral("fill") << QStringLiteral("fill-opacity") << QStringLiteral("fill-rule") << QStringLiteral("stroke")
                       << QStringLiteral("stroke-width") << QStringLiteral("stroke-linecap") << QStringLiteral("stroke-linejoin")
                       << QStringLiteral("stroke-miterlimit") << QStringLiteral("stroke-dasharray") << QStringLiteral("stroke-opacity");

    if (options == UseInkscapeOrder) {
        for (const QString &property : std::as_const(d->m_inkscapeOrder)) {
            if (styleProperties.contains(property)) {
                styleBuffer += property + QLatin1Char(':') + styleProperties.take(property) + QLatin1Char(';');
            } else {
                // Do Nothing
            }
        }
    }

    // Append any style properties
    if (!styleProperties.isEmpty()) {
        QHashIterator<QString, QString> it(styleProperties);
        while (it.hasNext()) {
            it.next();
            styleBuffer += it.key() + QLatin1Char(':') + it.value() + QLatin1Char(';');
        }
    }

    // Remove trailing semicolon if original didn't have one
    if (!d->styleHasTrailingSemicolon()) {
        styleBuffer.chop(1);
    }
    setStyle(styleBuffer);
}

QTransform KGameSvgDocument::transformMatrix() const
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
    QRegularExpression rx;
    QString transformAttribute;
    int i = 0;
    QTransform baseMatrix = QTransform();

    transformAttribute = transform();
    if (transformAttribute == QLatin1String("Element has no transform attribute.")) {
        return QTransform();
    }
    transformAttribute = transformAttribute.trimmed();

    rx.setPattern(TRANSFORMS);
    if (!rx.match(transformAttribute).hasMatch()) {
        qCWarning(KDEGAMESPRIVATE_LOG) << "Transform attribute seems to be invalid. Check your SVG file.";
        return QTransform();
    }

    rx.setPattern(TRANSFORM);

    while (transformAttribute.size() > 0 && i < 32) // 32 is an arbitrary limit for the number of transforms for a single node
    {
        QRegularExpressionMatch match = rx.match(transformAttribute);
        int result = match.capturedStart();
        if (result != -1) // Found left-most transform
        {
            if (match.captured(1) == QLatin1String("matrix")) {
                // If the first transform found is a matrix, use it as the base,
                // else we use a null matrix.
                if (i == 0) {
                    baseMatrix = QTransform(match.captured(2).toDouble(),
                                            match.captured(3).toDouble(),
                                            match.captured(4).toDouble(),
                                            match.captured(5).toDouble(),
                                            match.captured(6).toDouble(),
                                            match.captured(7).toDouble());
                } else {
                    baseMatrix = QTransform(match.captured(2).toDouble(),
                                            match.captured(3).toDouble(),
                                            match.captured(4).toDouble(),
                                            match.captured(5).toDouble(),
                                            match.captured(6).toDouble(),
                                            match.captured(7).toDouble())
                        * baseMatrix;
                }
            }

            if (match.captured(8) == QLatin1String("translate")) {
                double x = match.captured(9).toDouble();
                double y = match.captured(10).toDouble();
                if (match.captured(10).isEmpty()) // y defaults to zero per SVG standard
                {
                    y = 0;
                }
                baseMatrix = baseMatrix.translate(x, y);
            }

            if (match.captured(11) == QLatin1String("scale")) {
                double x = match.captured(12).toDouble();
                double y = match.captured(12).toDouble();
                if (match.captured(13).isEmpty()) // y defaults to x per SVG standard
                {
                    y = x;
                }
                baseMatrix = baseMatrix.scale(x, y);
            }

            if (match.captured(14) == QLatin1String("rotate")) {
                double a = match.captured(15).toDouble();
                double cx = match.captured(16).toDouble();
                double cy = match.captured(17).toDouble();

                if ((cx > 0) || (cy > 0)) // rotate around point (cx, cy)
                {
                    baseMatrix.translate(cx, cy);
                    baseMatrix.rotate(a);
                    baseMatrix.translate((cx * -1), (cy * -1));
                } else {
                    baseMatrix = baseMatrix.rotate(a); // rotate around origin
                }
            }

            if (match.captured(18) == QLatin1String("skewX")) {
                baseMatrix = baseMatrix.shear(match.captured(19).toDouble() * (M_PI / 180), 0);
            }

            if (match.captured(20) == QLatin1String("skewY")) {
                baseMatrix = baseMatrix.shear(0, match.captured(21).toDouble() * (M_PI / 180));
            }
        }
        transformAttribute = transformAttribute.mid(match.capturedLength() + result);
        i++;
    }

    return baseMatrix;
}

void KGameSvgDocument::setTransformMatrix(QTransform &matrix, MatrixOptions options)
{
    QString transformBuffer, tmp;
    QTransform null = QTransform();

    if (options == ApplyToCurrentMatrix) {
        matrix = transformMatrix() * matrix;
    }

    transformBuffer = QStringLiteral("matrix(");
    transformBuffer += tmp.setNum(matrix.m11(), 'g', 7) + QLatin1Char(',');
    transformBuffer += tmp.setNum(matrix.m12(), 'g', 7) + QLatin1Char(',');
    transformBuffer += tmp.setNum(matrix.m21(), 'g', 7) + QLatin1Char(',');
    transformBuffer += tmp.setNum(matrix.m22(), 'g', 7) + QLatin1Char(',');
    transformBuffer += tmp.setNum(matrix.dx(), 'g', 7) + QLatin1Char(',');
    transformBuffer += tmp.setNum(matrix.dy(), 'g', 7) + QLatin1Char(')');

    if ((transform() == QLatin1String("Element has no transform attribute.")) && (matrix == null)) {
        // Do not write a meaningless matrix to DOM
    } else {
        setTransform(transformBuffer);
    }
}

//
// Private
//

QDomNode KGameSvgDocumentPrivate::findElementById(const QString &attributeName, const QString &attributeValue, const QDomNode &node)
{
    QDomElement e = node.toElement(); // try to convert the node to an element.
    QString value = e.attribute(attributeName, QStringLiteral("Element has no attribute with that name."));

    if (value == attributeValue) {
        // We found our node.  Stop recursion and return it.
        return node;
    }

    if (!node.firstChild().isNull()) {
        QDomNode result = findElementById(attributeName, attributeValue, node.firstChild());
        /** We have recursed, now we need to have this recursion end when
         * the function call above returns
         */
        if (!result.isNull())
            return result; // If we found the node with id, then return it
    }
    if (!node.nextSibling().isNull()) {
        QDomNode result = findElementById(attributeName, attributeValue, node.nextSibling());
        /** We have recursed, now we need to have this recursion end when
         * the function call above returns */
        if (!result.isNull())
            return result;
    }
    if (!node.firstChild().isNull() && !node.nextSibling().isNull()) {
        // Do Nothing
        // qCDebug(KDEGAMESPRIVATE_LOG) << "No children or siblings.";
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
