project(kggznet)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set(kggznet_LIB_SRCS
    kggzpacket.cpp
    kggzraw.cpp)

kde4_add_library(kggznet SHARED ${kggznet_LIB_SRCS})

target_link_libraries(kggznet ${KDE4_KDECORE_LIBS} ${QT_QTNETWORK_LIBRARY})

set_target_properties(kggznet PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kggznet ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    kggznet_export.h
    kggzraw.h
    kggzpacket.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kggznet)
