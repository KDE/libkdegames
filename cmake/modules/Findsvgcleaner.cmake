# SPDX-FileCopyrightText: 2023 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

#[=======================================================================[.rst:
Findsvgcleaner
--------------

Try to find svgcleaner

If the svgcleaner executable is not in your PATH, you can provide
an alternative name or full path location with the ``svgcleaner_EXECUTABLE``
variable.

This will define the following variables:

``svgcleaner_FOUND``
    TRUE if svgcleaner is available

``svgcleaner_EXECUTABLE``
    Path to svgcleaner executable

If ``svgcleaner_FOUND`` is TRUE, it will also define the following imported
target:

``svgcleaner::svgcleaner``
    Path to svgcleaner executable
#]=======================================================================]

find_program(svgcleaner_EXECUTABLE NAMES svgcleaner)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(svgcleaner
    FOUND_VAR
        svgcleaner_FOUND
    REQUIRED_VARS
        svgcleaner_EXECUTABLE
)
mark_as_advanced(svgcleaner_EXECUTABLE)

if(NOT TARGET svgcleaner::svgcleaner AND svgcleaner_FOUND)
    add_executable(svgcleaner::svgcleaner IMPORTED)
    set_target_properties(svgcleaner::svgcleaner PROPERTIES
        IMPORTED_LOCATION "${svgcleaner_EXECUTABLE}"
    )
endif()

include(FeatureSummary)
set_package_properties(svgcleaner PROPERTIES
    URL "https://github.com/RazrFalcon/svgcleaner"
    DESCRIPTION "Tool to clean up SVG files from unnecessary data"
)
