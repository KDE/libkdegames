ecm_add_qml_module(corebindingsplugin URI org.kde.games.core)

target_sources(corebindingsplugin PRIVATE
    corebindingsplugin.cpp corebindingsplugin.h
)

ecm_target_qml_sources(corebindingsplugin VERSION 0.1 SOURCES
    qml/KGameItem.qml
)

target_link_libraries(corebindingsplugin
    PRIVATE
        KDEGamesGui
        Qt6::Qml
)

ecm_finalize_qml_module(corebindingsplugin)
