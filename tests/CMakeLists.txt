# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# This can be used for finding .rc files in the source dir, without installing them
add_definitions( -DKDESRCDIR=\\"${CMAKE_CURRENT_SOURCE_DIR}\\" )

include_directories( ${KDE4_KDEGAMES_INCLUDES} )

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

LIBKDEGAMES_EXECUTE_TESTS(
    ${libkdegames_unittests}
)
