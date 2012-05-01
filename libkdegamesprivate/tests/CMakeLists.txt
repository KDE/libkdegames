set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

configure_file(config-tests.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-tests.h)

MACRO(LIBKDEGAMESPRIVATE_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname}  ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kdegamesprivate)
       ENDFOREACH(_testname)
ENDMACRO(LIBKDEGAMESPRIVATE_UNIT_TESTS)

LIBKDEGAMESPRIVATE_UNIT_TESTS(
    kgamesvgdocumenttest
    kgamepropertytest
)
