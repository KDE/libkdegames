project(corebindings)

set(corebindings_SRCS
    corebindingsplugin.cpp
    )

add_library(corebindingsplugin SHARED ${corebindings_SRCS})
target_link_libraries(corebindingsplugin KF5KDEGames Qt5::Qml)

install(TARGETS corebindingsplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/games/core)
install(DIRECTORY qml/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/games/core)
