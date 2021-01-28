project(corebindings)

add_library(corebindingsplugin SHARED)

target_sources(corebindingsplugin PRIVATE
    corebindingsplugin.cpp
)

target_link_libraries(corebindingsplugin
    PRIVATE
        KF5KDEGames
        Qt5::Qml
)

install(TARGETS corebindingsplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/games/core)
install(DIRECTORY qml/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/games/core)
