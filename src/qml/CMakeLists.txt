set(corebindingsplugin_QML_INSTALL_DIR ${KDE_INSTALL_QMLDIR}/org/kde/games/core)

add_library(corebindingsplugin SHARED)

target_sources(corebindingsplugin PRIVATE
    corebindingsplugin.cpp corebindingsplugin.h
)

target_link_libraries(corebindingsplugin
    PRIVATE
        KF5KDEGames
        Qt${QT_MAJOR_VERSION}::Qml
)

install(TARGETS corebindingsplugin
    DESTINATION ${corebindingsplugin_QML_INSTALL_DIR}
)
install(DIRECTORY qml/
    DESTINATION ${corebindingsplugin_QML_INSTALL_DIR}
)
