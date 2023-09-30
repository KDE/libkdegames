/*
    SPDX-FileCopyrightText: 2007 Mark A. Taff <kde@marktaff.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef _KGAMESVGDOCUMENT_H_
#define _KGAMESVGDOCUMENT_H_

// own
#include "kdegamesprivate_export.h"
// Qt
#include <QDomDocument>
#include <QHash>
#include <QLoggingCategory>
#include <QTransform>
// Std
#include <memory>

Q_DECLARE_LOGGING_CATEGORY(GAMES_LIB)

class KGameSvgDocumentPrivate;

/**
 * \class KGameSvgDocument kgamesvgdocument.h <KGameSvgDocument>
 *
 * @brief A class for manipulating an SVG file using DOM
 *
 * This class is a wrapper around QDomDocument for SVG files.
 * It:
 * @li implements elementById();
 * @li manipulates a node's style properties; and,
 * @li manipulates a node's transform properties.
 *
 * @note The DOM standard requires all changes to be "live", so we cannot cache any values
 *     from the file; instead, we always have to query the DOM for the current value.  This also
 *     means that style & matrix changes we make happen to the DOM immediately.
 *
 * A typical use is to read in an SVG file, edit the style or transform attributes
 * in DOM as desired, and then output a QByteArray suitable for being loaded with
 * QSvgRenderer::load().
 *
 * To read an SVG file into DOM:
 * @code
 * KGameSvgDocument svgDom;
 * svgDom.load("/path/to/svgFile.svg");
 * @endcode
 *
 * To find a node with a specific value in its id attribute, for example where id="playerOneGamepiece":
 * @code
 * QDomNode playerOneGamepiece = svgDom.elementById("playerOneGamepiece");
 *
 * // This works too
 * QDomNode playerOneGamepiece = svgDom.elementByUniqueAttributeValue("id", "playerOneGamepiece");
 * @endcode
 *
 * Most methods operate on the last node found by @c elementById() or @c elementByUniqueAttributeValue().
 * If the methods are working on the wrong node, then you are mistaken about which node was
 * the last node (or you found a bug).  Try calling @c setCurrentNode() with the node you are
 * wanting to modify to see if this is the issue.  Consider the following code for example:
 * @code
 * QDomNode playerOneGamepiece = svgDom.elementById("playerOneGamepiece");
 * QDomNode playerTwoGamepiece = svgDom.elementById("playerTwoGamepiece");
 *
 * // Set player one's game piece to have a white fill
 * svgDom.setStyleProperty("fill", "#ffffff");  // INCORRECT: playerTwoGamepiece is the last node, not playerOneGamepiece
 *
 * svgDom.setCurrentNode(playerOneGamepiece);   // CORRECT: Set current node to the node we want,
 * svgDom.setStyleProperty("fill", "#ffffff");  // then we modify the node
 * @endcode
 *
 * To skew the @c currentNode():
 * @code
 * // Skew the horizontal axis 7.5 degrees
 * svgDom.skew(-7.5, 0, KGameSvgDocument::ReplaceCurrentMatrix);
 * @endcode
 *
 * @warning Be careful when using the KGameSvgDocument::ApplyToCurrentMatrix flag. It multiplies the matrices,
 *     so if you repeatedly apply the same matrix to a node, you have a polynomial series @c x^2, and you will
 *     very quickly run into overflow issues.
 *
 * To output @c currentNode() to be rendered:
 * @code
 * QSvgRenderer svgRenderer;
 * QByteArray svg = svgDom.nodeToByteArray();
 * svgRenderer.load(svg);
 * @endcode
 *
 * To output the whole document to be rendered (See QDomDocument::toByteArray()):
 * @code
 * QSvgRenderer svgRenderer;
 * QByteArray svg = svgDom.toByteArray();
 * svgRenderer.load(svg);
 * @endcode
 *
 * @see QDomDocument, QSvgRenderer
 * @author Mark A. Taff \<kde@marktaff.com\>
 * @version 0.1
 *
 * @todo Add convenience functions for getting/setting individual style properties.
 *     I haven't completely convinced myself of the utility of this, so don't hold your breathe. ;-)
 */
class KDEGAMESPRIVATE_EXPORT KGameSvgDocument : public QDomDocument
{
public:
    /**
     * @brief Constructor
     */
    explicit KGameSvgDocument();

    /**
     * @brief Copy Constructor
     */
    KGameSvgDocument(const KGameSvgDocument &doc);

    /**
     * @brief Destructor
     */
    virtual ~KGameSvgDocument();

    /**
     * @brief Assignment Operator
     */
    KGameSvgDocument &operator=(const KGameSvgDocument &doc);

    /**
     * @brief Options for applying (multiplying) or replacing the current matrix
     */
    enum MatrixOption {
        /**
         * Apply to current matrix
         */
        ApplyToCurrentMatrix = 0x01,
        /**
         * Replace the current matrix
         */
        ReplaceCurrentMatrix = 0x02
    };
    /** @brief Q_DECLARE_FLAGS macro confuses doxygen, so create typedef's manually */
    typedef QFlags<MatrixOption> MatrixOptions;

    /**
     * Options for sorting style properties when building a style attribute
     */
    enum StylePropertySortOption {
        /**
         * When building a style attribute, do not sort
         */
        Unsorted = 0x01,
        /**
         * When building a style attribute, sort properties the same way Inkscape does
         */
        UseInkscapeOrder = 0x02
    };
    /** @brief Q_DECLARE_FLAGS macro confuses doxygen, so create typedef's manually */
    typedef QFlags<StylePropertySortOption> StylePropertySortOptions;

    /**
     * @brief Returns the node with the given value for the given attribute.
     *
     * Returns the element whose attribute given in @p attributeName is equal to the value
     * given in @p attributeValue.
     *
     * QDomDocument::elementById() always returns a null node because TT says they can't know
     * which attribute is the id attribute.  Here, we allow the id attribute to be specified.
     *
     * This function also sets @p m_currentNode to this node.
     *
     * @param attributeName The name of the identifying attribute, such as "id" to find.
     * @param attributeValue The value to look for in the attribute @p attributeName
     *     The values held in this attribute must be unique in the document, or the consequences
     *     may be unpredictably incorrect.  You've been warned. ;-)
     * @returns the matching node, or a null node if no matching node found
     */
    QDomNode elementByUniqueAttributeValue(const QString &attributeName, const QString &attributeValue);

    /**
     * @brief Returns a node with the given id.
     *
     * This is a convenience function.  We call elementByUniqueAttributeValue(), but we assume
     * that the name of the attribute is "id".  This assumption will be correct for valid SVG files.
     *
     * Returns the element whose ID is equal to elementId. If no element with the ID was found,
     * this function returns a null element.
     *
     * @param attributeValue The value of the id attribute to find
     * @returns the matching node, or a null node if no matching node found
     * @see elementByUniqueAttributeValue()
     */
    QDomNode elementById(const QString &attributeValue);

    /**
     * @brief Reads the SVG file svgFilename() into DOM.
     * @returns nothing
     */
    void load();

    /**
     * @overload
     * @brief This function permits specifying the svg file and loading it at once.
     *
     * @param svgFilename The filename of the SVG file to open.
     * @returns nothing
     */
    void load(const QString &svgFilename);

    /**
     * @brief Rotates the origin of the current node counterclockwise.
     *
     * @param degrees The amount in degrees to rotate by.
     * @param options Apply to current matrix or replace current matrix.
     * @returns nothing
     * @see QTransform#rotate()
     */
    void rotate(double degrees, const MatrixOptions &options = ApplyToCurrentMatrix);

    /**
     * @brief Moves the origin of the current node
     *
     * @param xPixels The number of pixels to move the x-axis by.
     * @param yPixels The number of pixels to move the y-axis by.
     * @param options Apply to current matrix or replace current matrix.
     * @returns nothing
     * @see QTransform::translate()
     */
    void translate(int xPixels, int yPixels, const MatrixOptions &options = ApplyToCurrentMatrix);

    /**
     * @brief Shears the origin of the current node.
     *
     * @param xRadians The amount in radians to shear (skew) the x-axis by.
     * @param yRadians The amount in radians to shear (skew) the y-axis by.
     * @param options Apply to current matrix or replace current matrix.
     * @returns nothing
     * @see QTransform::shear()
     */
    void shear(double xRadians, double yRadians, const MatrixOptions &options = ApplyToCurrentMatrix);

    /**
     * @brief Skews the origin of the current node.
     *
     * This is a convenience function.  It simply converts its arguments to
     * radians, then calls shear().
     *
     * @param xDegrees The amount in degrees to shear (skew) the x-axis by.
     * @param yDegrees The amount in degrees to shear (skew) the y-axis by.
     * @param options Apply to current matrix or replace current matrix.
     * @returns nothing
     * @see shear()
     */
    void skew(double xDegrees, double yDegrees, const MatrixOptions &options = ApplyToCurrentMatrix);

    /**
     * @brief Scales the origin of the current node.
     *
     * @note Neither @c xFactor nor @c yFactor may be zero, otherwise you scale
     *        the element into nonexistence.
     *
     * @param xFactor The factor to scale the x-axis by.
     * @param yFactor The factor to scale the y-axis by.
     * @param options Apply to current matrix or replace current matrix.
     * @returns nothing
     * @see QTransform::scale()
     */
    void scale(double xFactor, double yFactor, const MatrixOptions &options = ApplyToCurrentMatrix);

    /**
     * @brief Returns the last node found by elementById, or null if node not found
     *
     * @returns The current node
     * @see setCurrentNode()
     */
    QDomNode currentNode() const;

    /**
     * @brief Sets the current node.
     *
     * @param node The node to set currentNode to.
     * @returns nothing
     * @see currentNode()
     */
    void setCurrentNode(const QDomNode &node);

    /**
     * @brief Returns the name of the SVG file this DOM represents.
     *
     * @returns The current filename.
     * @see setSvgFilename()
     */
    QString svgFilename() const;

    /**
     * @brief Sets the current SVG filename.
     *
     * @param svgFilename The filename of the SVG file to open.
     * @returns nothing
     * @see svgFilename()
     */
    void setSvgFilename(const QString &svgFilename);

    /**
     * @brief Returns the value of the style property given for the current node.
     *
     * @note Internally, we create a hash with @c styleProperties, then return the value
     *     of the @c propertyName property.  As such, if you need the values of multiple
     *     properties, it will be more efficient to call @c styleProperties()
     *     and then use the hash directly.
     *
     * See KGameSvgDocumentPrivate::m_inkscapeOrder for a list of common SVG style properties
     *
     * @param propertyName the name of the property to return
     * @returns The value style property given, or null if no such property for this node.
     * @see setStyleProperty(), styleProperties(), setStyleProperties()
     */
    QString styleProperty(const QString &propertyName) const;

    /**
     * @brief Sets the value of the style property given for the current node.
     *
     * @note Internally, we create a hash with @c styleProperties, then update the
     *  @p propertyName to @p propertyValue, before finally applying the hash to
     *      DOM via @c setStyleProperties().  Because of this, if you need to set multiple
     *      properties per node, it will be more efficient to call @c styleProperties(),
     *      modify the hash it returns, and then apply the hash with @c setStyleProperties().
     *
     * @param propertyName The name of the property to set.
     * @param propertyValue The value of the property to set.
     * @returns nothing
     * @see styleProperty(), styleProperties(), setStyleProperties()
     */
    void setStyleProperty(const QString &propertyName, const QString &propertyValue);

    /**
     * @brief Returns the current node and its children as a new xml svg document.
     *
     * @returns The xml for the new svg document
     */
    QString nodeToSvg() const;

    /**
     * @brief Builds a new svg document and returns a QByteArray suitable for passing to QSvgRenderer::load().
     *
     * Internally, we call @c nodeToSvg() and then convert to a QByteArray, so this method
     * should be called @b instead of @c nodeToSvg().
     *
     * @returns the QByteArray
     */
    QByteArray nodeToByteArray() const;

    /**
     * @brief Returns the style attribute of the current node.
     *
     * Unless you are parsing your own style attribute for some reason, you probably
     * want to use styleProperty() or styleProperties().
     *
     * @returns The style attribute.
     * @see styleProperty() styleProperties()
     */
    QString style() const;

    /**
     * @brief Sets the style attribute of the current node.
     *
     * Unless you are parsing your own style attribute for some reason, you probably
     * want to use setStyleProperty() or setStyleProperties().
     *
     * @param styleAttribute The style attribute to apply.
     * @returns nothing
     *
     * @see setStyleProperty() setStyleProperties()
     */
    void setStyle(const QString &styleAttribute);

    /**
     * @brief Returns the patterns in the document
     *
     * @returns The patterns in the document
     */
    QDomNodeList patterns() const;

    /**
     * @brief Returns the linearGradients in the document
     *
     * @returns The linearGradients in the document
     */
    QDomNodeList linearGradients() const;

    /**
     * @brief Returns the radialGradients in the document
     *
     * @returns The radialGradients in the document
     */
    QDomNodeList radialGradients() const;

    /**
     * @brief Returns the defs in the document
     *
     * @returns The defs in the document
     */
    QDomNodeList defs() const;

    /**
     * @brief Returns the first def in the document
     *
     * @returns The first def in the document
     */
    QDomNode def() const;

    /**
     * @brief Returns the transform attribute of the current node.
     * @returns The transform attribute.
     * @see setTransform(), transformMatrix(), setTransformMatrix()
     */
    QString transform() const;

    /**
     * @brief Sets the transform attribute of the current node.
     *
     * As this function works on QStrings, it <b>replaces</b> the existing
     * transform attribute.  If you need to multiply, use setTransformMatrix() instead.
     *
     * @param transformAttribute The transform attribute to apply.
     * @returns nothing
     * @see transform(), transformMatrix(), setTransformMatrix()
     */
    void setTransform(const QString &transformAttribute);

    /**
     * @brief Returns a hash of the style properties of the current node.
     * @returns The style properties.
     * @see setStyleProperties()
     */
    QHash<QString, QString> styleProperties() const;

    /**
     * @brief Sets the style properties of the current node.
     *
     * The only(?) reason to set useInkscapeOrder to true is if you are saving the svg xml to a file
     * that may be human-edited later, for consistency. There is a performance hit, since hashes store
     * their data unsorted.
     *
     * @param _styleProperties The hash of style properties to apply.
     * @param options Apply the hash so the properties are in the same order as Inkscape writes them.
     * @returns nothing
     * @see styleProperties()
     */
    void setStyleProperties(const QHash<QString, QString> &_styleProperties, const StylePropertySortOptions &options = Unsorted);

    /**
     * @brief Returns the transform attribute of the current node as a matrix.
     *
     * @returns The matrix for the transform attribute.
     * @see setTransformMatrix()
     */
    QTransform transformMatrix() const;

    /**
     * @brief Sets the transform attribute of the current node.
     *
     * @param matrix The matrix to apply.
     * @param options Should we apply matrix to the current matrix?
     *     We modify matrix internally if @p options includes ApplyToCurrentMatrix, so it can't
     *     be passed as const.
     *     Normally we want to apply the existing matrix. If we apply the matrix,
     *     we potentially end up squaring with each call, e.g. x^2.
     * @returns nothing
     * @see transformMatrix()
     */
    void setTransformMatrix(QTransform &matrix, const MatrixOptions &options = ApplyToCurrentMatrix);

private:
    /**
     * @brief d-pointer
     */
    std::unique_ptr<KGameSvgDocumentPrivate> const d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KGameSvgDocument::MatrixOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(KGameSvgDocument::StylePropertySortOptions)

#endif // _KGAMESVGDOCUMENT_H_
