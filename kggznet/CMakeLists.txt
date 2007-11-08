project(kggznet)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set(kggznet_LIB_SRCS
    kggzpacket.cpp
    kggzraw.cpp)

kde4_add_library(kggznet SHARED ${kggznet_LIB_SRCS})

target_link_libraries(kggznet ${KDE4_KDECORE_LIBS} ${QT_QTNETWORK_LIBRARY})

set_target_properties(kggznet PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kggznet 
        RUNTIME DESTINATION ${BIN_INSTALL_DIR}
        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
        ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)
