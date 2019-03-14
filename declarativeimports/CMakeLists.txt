project(corebindings)

set(corebindings_SRCS
    corebindingsplugin.cpp
    )

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
)

add_library(corebindingsplugin SHARED ${corebindings_SRCS})
target_link_libraries(corebindingsplugin KF5KDEGames Qt5::Qml)

install(TARGETS corebindingsplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/games/core)
install(DIRECTORY qml/ DESTINATION ${QML_INSTALL_DIR}/org/kde/games/core)
