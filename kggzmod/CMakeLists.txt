project(kggzmod)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set(kggzmod_LIB_SRCS
    event.cpp
    module.cpp
    player.cpp
    request.cpp
    statistics.cpp)

kde4_automoc(${kggzmod_LIB_SRCS})

kde4_add_library(kggzmod STATIC ${kggzmod_LIB_SRCS})

target_link_libraries(kggzmod ${QT_QTNETWORK_LIBRARY})

