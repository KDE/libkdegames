project(corebindings)

include_directories( ${CMAKE_SOURCE_DIR}/libkdegames )

set(corebindings_SRCS
    corebindingsplugin.cpp
    )

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        ${KDE4_INCLUDES}
)

add_library(corebindingsplugin SHARED ${corebindings_SRCS})
target_link_libraries(corebindingsplugin kdegames Qt5::Qml)

install(TARGETS corebindingsplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/games/core)
install(DIRECTORY qml/ DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/games/core)
