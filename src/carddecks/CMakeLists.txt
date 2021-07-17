# SPDX-FileCopyrightText: 2021 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

function(install_carddeck id)
    set(options AUTHORS COPYING COPYRIGHT)
    set(oneValueArgs PREVIEW SVG)
    set(multiValueArgs)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(output_dir "${CMAKE_CURRENT_BINARY_DIR}/${id}")
    file(MAKE_DIRECTORY "${output_dir}")
    set(svgz "${output_dir}/${ARG_SVG}z")
    generate_svgz("${id}/${ARG_SVG}" ${svgz} "carddeck-${id}-")
    set(copyright_files)
    if (ARG_COPYRIGHT)
        list(APPEND copyright_files "${id}/COPYRIGHT")
    endif()
    if (ARG_COPYING)
        list(APPEND copyright_files "${id}/COPYING")
    endif()
    if (ARG_AUTHORS)
        list(APPEND copyright_files "${id}/AUTHORS")
    endif()
    install(
        FILES
            ${copyright_files}
            ${id}/index.desktop
            ${id}/${ARG_PREVIEW}
            ${svgz}
        DESTINATION ${KDE_INSTALL_DATADIR}/carddecks/${id}
    )
endfunction()

install_carddeck(svg-ancient-egyptians      PREVIEW 11.png              SVG Ancient_Egyptians.svg)
install_carddeck(svg-dondorf                PREVIEW 11.png              SVG dondorf.svg)
install_carddeck(svg-future                 PREVIEW 11.png              SVG future.svg)
install_carddeck(svg-gm-paris               PREVIEW 11.png              SVG paris.svg)
install_carddeck(svg-jolly-royal            PREVIEW 11.png              SVG jolly-royal.svg)
install_carddeck(svg-konqi-modern           PREVIEW 11.png              SVG konqi.svg)
install_carddeck(svg-nicu-ornamental        PREVIEW 11.png              SVG ornamental.svg  AUTHORS COPYING)
install_carddeck(svg-nicu-white             PREVIEW 11.png              SVG white.svg  AUTHORS COPYING)
install_carddeck(svg-oxygen                 PREVIEW 11.png              SVG oxygen.svg)
install_carddeck(svg-oxygen-air             PREVIEW 11.png              SVG oxygen-air.svg)
install_carddeck(svg-oxygen-white           PREVIEW 11.png              SVG oxygen-white.svg)
install_carddeck(svg-penguins               PREVIEW 11.png              SVG penguins.svg  COPYRIGHT)
install_carddeck(svg-standard               PREVIEW 11.png              SVG standard.svg)
install_carddeck(svg-tigullio-international PREVIEW queen-of-hearts.png SVG tigullio-international.svg)
install_carddeck(svg-xskat-french           PREVIEW 11.png              SVG french.svg  COPYRIGHT)
install_carddeck(svg-xskat-german           PREVIEW 11.png              SVG german.svg  COPYRIGHT)
