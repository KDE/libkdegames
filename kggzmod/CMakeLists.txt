project(kggzmod)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set(kggzmod_LIB_SRCS
    event.cpp
    module.cpp
    player.cpp
    request.cpp
    statistics.cpp)

kde4_add_library(kggzmod SHARED ${kggzmod_LIB_SRCS})

target_link_libraries(kggzmod ${KDE4_KDECORE_LIBS} ${QT_QTNETWORK_LIBRARY} kggznet)

set_target_properties(kggzmod PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kggzmod DESTINATION ${LIB_INSTALL_DIR})
