project(kggznet)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set(kggznet_LIB_SRCS
    kggzpacket.cpp
    kggzraw.cpp)

kde4_automoc(${kggznet_LIB_SRCS})

kde4_add_library(kggznet STATIC ${kggznet_LIB_SRCS})

target_link_libraries(kggznet ${QT_QTNETWORK_LIBRARY})

