set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

include(ECMMarkAsTest)

configure_file(config-tests.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-tests.h)

MACRO(LIBKDEGAMESPRIVATE_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} Qt5::Test KF5KDEGamesPrivate)
	       add_test(NAME libkdegamesprivate-${_testname} COMMAND ${_testname})
               ecm_mark_as_test(${_testname})
       ENDFOREACH(_testname)
ENDMACRO(LIBKDEGAMESPRIVATE_UNIT_TESTS)

LIBKDEGAMESPRIVATE_UNIT_TESTS(
    kgamesvgdocumenttest
    kgamepropertytest
)
