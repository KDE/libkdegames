# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(  ${CMAKE_SOURCE_DIR}/libkdegames )

MACRO(LIBKDEGAMES_BUILD_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_automoc(${_testname}.cpp)
               kde4_add_executable(${_testname} NOGUI RUN_UNINSTALLED ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} ${KDE4_KDEGAMES_LIBS} ${QT_QTTEST_LIBRARY} kdegames)
       ENDFOREACH(_testname)
ENDMACRO(LIBKDEGAMES_BUILD_TESTS)

MACRO(LIBKDEGAMES_EXECUTE_TESTS)
       FOREACH(_testname ${ARGN})
               add_test(libkdegames-${_testname} ${EXECUTABLE_OUTPUT_PATH}/${_testname})
       ENDFOREACH(_testname)
ENDMACRO(LIBKDEGAMES_EXECUTE_TESTS)

set(libkdegames_unittests
    kgamesvgdocumenttest 
)

LIBKDEGAMES_BUILD_TESTS(
    ${libkdegames_unittests}
)

if (Q_WS_X11)
LIBKDEGAMES_BUILD_TESTS(
#    kxerrorhandlertest # File missing from svn
)
endif (Q_WS_X11)

# Install any data files needed by the unit tests
install(FILES
	kgamesvgdocumenttest.svg
    DESTINATION ${EXECUTABLE_OUTPUT_PATH})

LIBKDEGAMES_EXECUTE_TESTS(
    ${libkdegames_unittests}
)
