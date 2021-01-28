project(libkdegamesprivate)

set(kdegamesprivate_INCLUDE_INSTALL_DIR ${kdegames_INCLUDE_INSTALL_DIR}/libkdegamesprivate)

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

add_library(KF5KDEGamesPrivate SHARED)

ecm_generate_export_header(KF5KDEGamesPrivate
    EXPORT_FILE_NAME libkdegamesprivate_export.h
    BASE_NAME KDEGAMESPRIVATE
    VERSION ${KDEGAMES_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 3.2 4.9 7.3
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_sources(KF5KDEGamesPrivate PRIVATE
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

ki18n_wrap_ui(KF5KDEGamesPrivate
    kgamethemeselector.ui
)

target_link_libraries(KF5KDEGamesPrivate
    PUBLIC
        KF5KDEGames
        KF5::Completion
        Qt5::Xml
        Qt5::Network
    PRIVATE
        KF5::DNSSD
        KF5::NewStuff
        KF5::Archive
)

target_include_directories(KF5KDEGamesPrivate
    INTERFACE
        "$<INSTALL_INTERFACE:${kdegamesprivate_INCLUDE_INSTALL_DIR}>"
    PRIVATE
        kgame
)

set_target_properties(KF5KDEGamesPrivate PROPERTIES
    VERSION   ${KDEGAMES_VERSION}
    SOVERSION ${KDEGAMES_SOVERSION}
)

install(TARGETS KF5KDEGamesPrivate
    EXPORT KF5KDEGamesTargets
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
