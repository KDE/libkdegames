# SPDX-FileCopyrightText: 2021, 2023 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

option(BUILD_SVG_CHECKS "Build SVG rendering checks." OFF)
add_feature_info(BUILD_SVG_CHECKS BUILD_SVG_CHECKS "Build SVG rendering checks.")

if(BUILD_SVG_CHECKS)
    find_package(ImageMagick COMPONENTS compare)
    set_package_properties(ImageMagick PROPERTIES
        TYPE REQUIRED
        PURPOSE "For checking cleaned SVG file renderings"
    )

    add_subdirectory(tools)
endif()

function(install_carddeck id)
    set(options AUTHORS COPYING COPYRIGHT NO_CLEANING)
    set(oneValueArgs PREVIEW SVG)
    set(multiValueArgs)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(output_dir "${CMAKE_CURRENT_BINARY_DIR}/${id}")
    file(MAKE_DIRECTORY "${output_dir}")

    set(generate_args)
    if (ARG_NO_CLEANING)
        list(APPEND generate_args NO_CLEANING)
    endif()

    set(svgz "${output_dir}/${ARG_SVG}z")
    generate_svgz("${id}/${ARG_SVG}" ${svgz} "carddeck-${id}-" ${generate_args})
    if(BUILD_SVG_CHECKS)
        list_with_rendering_check(${id} "${id}/${ARG_SVG}" ${svgz})
    endif()

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

# Disabling svgcleaner due to issues with: card back
# Yet to be analysed.
install_carddeck(svg-ancient-egyptians      PREVIEW 11.png              SVG Ancient_Egyptians.svg   NO_CLEANING)
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
# Disabling svgcleaner due to issues with: cards king_*, queen_*, jack_*
# svgcleaner when removing default attributes fails over <use> e.g. with card decos setting some style,
# where the copied (sub)elements explicitly want to (re)set some default value, e.g. to stroke:none
# -> --remove-default-attributes=no helps
# Some other stroke issue though yet to be analysed
install_carddeck(svg-penguins               PREVIEW 11.png              SVG penguins.svg  COPYRIGHT   NO_CLEANING)
install_carddeck(svg-standard               PREVIEW 11.png              SVG standard.svg)
install_carddeck(svg-tigullio-international PREVIEW queen-of-hearts.png SVG tigullio-international.svg)
install_carddeck(svg-xskat-french           PREVIEW 11.png              SVG french.svg  COPYRIGHT)
install_carddeck(svg-xskat-german           PREVIEW 11.png              SVG german.svg  COPYRIGHT)
