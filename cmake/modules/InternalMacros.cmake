# SPDX-FileCopyrightText: 2021, 2023 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

find_package(7Zip)
set_package_properties(7Zip PROPERTIES
    PURPOSE "For installing SVG files as SVGZ"
)

if(WIN32)
    set_package_properties(7Zip PROPERTIES
        TYPE REQUIRED
    )
else()
    set_package_properties(7Zip PROPERTIES
        TYPE OPTIONAL
    )
    if(NOT TARGET 7Zip::7Zip)
        find_package(gzip)
        set_package_properties(gzip PROPERTIES
            TYPE REQUIRED
            PURPOSE "For installing SVG files as SVGZ (less efficient fallback for 7-Zip)"
        )
    endif()
endif()

find_package(svgcleaner)
set_package_properties(gzip PROPERTIES
    TYPE OPTIONAL
    PURPOSE "For shrinking the installed SVG files a bit."
)

function(generate_svgz svg_file svgz_file target_prefix)
    cmake_parse_arguments(ARGS "NO_CLEANING" "" "" ${ARGN})

    if (NOT IS_ABSOLUTE ${svg_file})
        set(svg_file "${CMAKE_CURRENT_SOURCE_DIR}/${svg_file}")
    endif()
    if (NOT EXISTS ${svg_file})
        message(FATAL_ERROR "No such file found: ${svg_file}")
    endif()
    get_filename_component(_fileName "${svg_file}" NAME)

    if(TARGET svgcleaner::svgcleaner AND NOT ARGS_NO_CLEANING)
        get_filename_component(cleaned_svg_dir ${svgz_file} DIRECTORY)
        get_filename_component(cleaned_svg_basename ${svgz_file} NAME_WLE)
        set(cleaned_svg_file "${cleaned_svg_dir}/${cleaned_svg_basename}-cleaned.svg")

        add_custom_command(
            OUTPUT ${cleaned_svg_file}
            COMMAND svgcleaner::svgcleaner
            ARGS
                "--quiet"
                "--no-defaults"
                # remove unused elements & attributes
                "--remove-metadata=yes"
                "--remove-comments=yes"
                "--remove-declarations=yes"
                "--remove-nonsvg-elements=yes"
                "--remove-unused-defs=yes"
                "--remove-nonsvg-attributes=yes"
                "--remove-text-attributes=yes"
                "--remove-unused-coordinates=yes"
                "--remove-default-attributes=yes"
                "--remove-needless-attributes=yes"
                "--simplify-transforms=yes"
                # minimize path expressions
                "--paths-to-relative=yes"
                "--remove-unused-segments=yes"
                "--convert-segments=yes"
                "--trim-paths=yes"
                "--remove-dupl-cmd-in-paths=yes"
                "--use-implicit-cmds=yes"
                # keep full values
                "--coordinates-precision=12"
                "--properties-precision=12"
                "--transforms-precision=12"
                "--paths-coordinates-precision=12"
                ${svg_file}
                ${cleaned_svg_file}
            DEPENDS ${svg_file}
            COMMENT "Optimizing ${_fileName}"
        )
        set(svg_file ${cleaned_svg_file})
    endif()

    if(TARGET 7Zip::7Zip)
        add_custom_command(
            OUTPUT ${svgz_file}
            COMMAND 7Zip::7Zip
            ARGS
                a
                -bd # silence logging
                -mx9 # compress best
                -tgzip
                ${svgz_file} ${svg_file}
            DEPENDS ${svg_file}
            COMMENT "Gzipping ${_fileName}"
        )
    else()
        add_custom_command(
            OUTPUT ${svgz_file}
            COMMAND gzip::gzip
            ARGS
                -9 # compress best
                -n # no original name and timestamp stored, for reproducibility
                -c # write to stdout
                ${svg_file} > ${svgz_file}
            DEPENDS ${svg_file}
            COMMENT "Gzipping ${_fileName}"
        )
    endif()

    add_custom_target("${target_prefix}${_fileName}z" ALL DEPENDS ${svgz_file})
endfunction()
