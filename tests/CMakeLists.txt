include(ECMMarkAsTest)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
)

macro(libkdegames_executable_tests)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        # if test has an ui file, use it
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_testname}.ui)
            ki18n_wrap_ui(${_testname} ${_testname}.ui)
        endif()

        target_link_libraries(${_testname} KDEGames KF6::I18n KF6::CoreAddons Qt6::Test)
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro()

libkdegames_executable_tests(
#    kxerrorhandlertest # File missing from svn
    kgamepopupitemtest
)
