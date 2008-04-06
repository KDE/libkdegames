project(kggzgames)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set(kggzgames_LIB_SRCS
    kggzseatsdialog.cpp
    kggzrankingsdialog.cpp)

kde4_add_library(kggzgames SHARED ${kggzgames_LIB_SRCS})

target_link_libraries(kggzgames ${KDE4_KIO_LIBS} ${QT_QTNETWORK_LIBRARY} kggzmod kdegames)
# FIXME: we link against kdegames but we only need the scores dialogue

set_target_properties(kggzgames PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kggzgames ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
    kggzgames_export.h
    kggzseatsdialog.h
    kggzrankingsdialog.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kggzgames)
