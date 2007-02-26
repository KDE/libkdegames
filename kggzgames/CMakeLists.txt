project(kggzgames)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set(kggzgames_LIB_SRCS
    kggzseatsdialog.cpp)

kde4_automoc(${kggzgames_LIB_SRCS})

kde4_add_library(kggzgames STATIC ${kggzgames_LIB_SRCS})

target_link_libraries(kggzgames ${QT_QTNETWORK_LIBRARY})

