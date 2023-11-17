# SPDX-FileCopyrightText: 2023 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

add_executable(renderelement renderelement.cpp)
target_link_libraries(renderelement Qt::Svg)

# setup generral renderig check target
add_custom_target(check_rendering)

function(list_with_rendering_check id old_file new_file)
    # symbolic target for cheking this file, always outdated
    set(check_rendering_output "check_rendering_${old_file}")
    set_property(SOURCE ${check_rendering_output} PROPERTY SYMBOLIC 1)

    add_custom_command(
        OUTPUT ${check_rendering_output}
        COMMAND ${CMAKE_COMMAND}
        ARGS
            "-DID=${id}"
            "-DWORKING_DIR=${CMAKE_CURRENT_BINARY_DIR}"
            "-DCOMPARE=${ImageMagick_compare_EXECUTABLE}"
            "-DRENDERELEMENT=$<TARGET_FILE:renderelement>"
            "-DOLD_FILE=${CMAKE_CURRENT_SOURCE_DIR}/${old_file}"
            "-DNEW_FILE=${new_file}"
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tools/CompareSVGs.cmake
        COMMENT "Checking SVG of ${id}"
    )

    # add to general target
    set(target_check_rendering_output "check_rendering_${id}")
    add_custom_target(${target_check_rendering_output} DEPENDS ${check_rendering_output})
    add_dependencies(check_rendering ${target_check_rendering_output})
endfunction()
