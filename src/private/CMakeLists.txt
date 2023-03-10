set(kdegamesprivate_INCLUDE_INSTALL_DIR ${kdegames_INCLUDE_INSTALL_DIR}/libkdegamesprivate)
if (QT_MAJOR_VERSION STREQUAL "5")
    set(KDEGAMESPRIVATE_OUTPUT_NAME KF5KDEGamesPrivate)
    set(KDEGAMESPRIVATE_EXPORT_NAME KF5KDEGamesPrivate)
else()
    set(KDEGAMESPRIVATE_OUTPUT_NAME KDEGames6Private)
    set(KDEGAMESPRIVATE_EXPORT_NAME KDEGames6Private)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

ecm_qt_export_logging_category(
    IDENTIFIER GAMES_PRIVATE
    CATEGORY_NAME org.kde.games.private
    DESCRIPTION "libkdegamesprivate"
    EXPORT KDEGAMES
)

ecm_qt_export_logging_category(
    IDENTIFIER GAMES_PRIVATE_KGAME
    CATEGORY_NAME org.kde.games.private.kgame
    DESCRIPTION "libkdegamesprivate KGame"
    EXPORT KDEGAMES
)

add_library(KDEGamesPrivate SHARED)

ecm_generate_export_header(KDEGamesPrivate
    EXPORT_FILE_NAME libkdegamesprivate_export.h
    BASE_NAME KDEGAMESPRIVATE
    VERSION ${KDEGAMES_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 3.2 4.9 7.3 7.4
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_sources(KDEGamesPrivate PRIVATE
    kchatbase.cpp
    kchatbaseitemdelegate.cpp
    kchatbasemodel.cpp
    kgame/kgamechat.cpp
    kgame/kgame.cpp
    kgame/kgameerror.cpp
    kgame/kgameio.cpp
    kgame/kgamemessage.cpp
    kgame/kgamenetwork.cpp
    kgame/kgameproperty.cpp
    kgame/kgamepropertyhandler.cpp
    kgame/kgamesequence.cpp
    kgame/kmessageclient.cpp
    kgame/kmessageio.cpp
    kgame/kmessageserver.cpp
    kgame/kplayer.cpp
    kgamecanvas.cpp
    kgamedifficulty.cpp
    kgamesvgdocument.cpp
    kgametheme.cpp
    kgamethemeselector.cpp
)

ki18n_wrap_ui(KDEGamesPrivate
    kgamethemeselector.ui
)

target_link_libraries(KDEGamesPrivate
    PUBLIC
        KDEGames
        KF${KF_MAJOR_VERSION}::Completion
        Qt${QT_MAJOR_VERSION}::Xml
        Qt${QT_MAJOR_VERSION}::Network
    PRIVATE
        KF${KF_MAJOR_VERSION}::DNSSD
        KF${KF_MAJOR_VERSION}::NewStuffWidgets
        KF${KF_MAJOR_VERSION}::Archive
        KF${KF_MAJOR_VERSION}::XmlGui
)
if (TARGET Qt6::Core5Compat)
    target_link_libraries(KDEGamesPrivate PRIVATE Qt6::Core5Compat)
endif()


target_include_directories(KDEGamesPrivate
    INTERFACE
        "$<INSTALL_INTERFACE:${kdegamesprivate_INCLUDE_INSTALL_DIR}>"
    PRIVATE
        kgame
)

set_target_properties(KDEGamesPrivate PROPERTIES
    OUTPUT_NAME ${KDEGAMESPRIVATE_OUTPUT_NAME}
    EXPORT_NAME ${KDEGAMESPRIVATE_EXPORT_NAME}
    VERSION   ${KDEGAMES_VERSION}
    SOVERSION ${KDEGAMES_SOVERSION}
)

install(TARGETS KDEGamesPrivate
    EXPORT KDEGamesTargets
    ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libkdegamesprivate_export.h
    kchatbase.h
    kchatbaseitemdelegate.h
    kchatbasemodel.h
    kgamecanvas.h
    kgamedifficulty.h
    kgamesvgdocument.h
    kgametheme.h
    kgamethemeselector.h
    DESTINATION ${kdegamesprivate_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(FILES
    kgame/kgamechat.h
    kgame/kgameerror.h
    kgame/kgame.h
    kgame/kgameio.h
    kgame/kgamemessage.h
    kgame/kgamenetwork.h
    kgame/kgameproperty.h
    kgame/kgamepropertyhandler.h
    kgame/kgamesequence.h
    kgame/kgameversion.h
    kgame/kmessageclient.h
    kgame/kmessageio.h
    kgame/kmessageserver.h
    kgame/kplayer.h
    DESTINATION ${kdegamesprivate_INCLUDE_INSTALL_DIR}/kgame
    COMPONENT Devel
)
