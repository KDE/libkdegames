project(kggzgames)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set(kggzgames_LIB_SRCS
    kggzseatsdialog.cpp)

kde4_automoc(${kggzgames_LIB_SRCS})

kde4_add_library(kggzgames SHARED ${kggzgames_LIB_SRCS})

target_link_libraries(kggzgames ${KDE4_KIO_LIBS} ${QT_QTNETWORK_LIBRARY} kggzmod)

set_target_properties(kggzgames PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kggzgames DESTINATION ${LIB_INSTALL_DIR})
