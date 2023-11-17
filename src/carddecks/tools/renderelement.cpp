/*
    SPDX-FileCopyrightText: 2023 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: BSD-3-Clause
*/

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QString>
#include <QSvgRenderer>

#include <iostream>

using namespace Qt::Literals;

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument(u"svg_file"_s, u"Input SVG file"_s);
    parser.addPositionalArgument(u"element_id"_s, u"SVG Element Id"_s);
    parser.addPositionalArgument(u"width"_s, u"Width"_s);
    parser.addPositionalArgument(u"height"_s, u"Height"_s);
    parser.addPositionalArgument(u"png_file"_s, u"Output PNG file"_s);

    parser.process(app);

    const QStringList args = parser.positionalArguments();

    if (args.size() < 5) {
        std::cout << qPrintable(parser.helpText());
        return -1;
    }

    const int width = args[2].toInt();
    const int height = args[3].toInt();
    const QString inputPath = args[0];
    const QString elementId = args[1];
    const QString outputPath = args[4];

    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QSvgRenderer renderer(inputPath);
    if (!renderer.isValid()) {
        return -1;
    }

    QPainter p(&image);
    renderer.render(&p, elementId);

    image.save(outputPath, "PNG");

    return 0;
}
