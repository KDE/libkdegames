set(kdegamesprivate_INCLUDE_INSTALL_DIR ${kdegames_INCLUDE_INSTALL_DIR}/libkdegamesprivate)
set(KDEGAMESPRIVATE_OUTPUT_NAME KDEGames6Private)
set(KDEGAMESPRIVATE_EXPORT_NAME KDEGames6Private)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

add_library(KDEGamesPrivate SHARED)

ecm_generate_export_header(KDEGamesPrivate
    VERSION ${KDEGAMES_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    USE_VERSION_HEADER
    VERSION_BASE_NAME KDEGames
)

ecm_qt_declare_logging_category(KDEGamesPrivate
    HEADER kdegamesprivate_logging.h
    IDENTIFIER KDEGAMESPRIVATE_LOG
    CATEGORY_NAME kdegames.private
    DESCRIPTION "libkdegamesprivate"
    EXPORT KDEGAMES
)

ecm_qt_declare_logging_category(KDEGamesPrivate
    HEADER kdegamesprivate_kgame_logging.h
    IDENTIFIER KDEGAMESPRIVATE_KGAME_LOG
    CATEGORY_NAME kdegames.private.kgame
    DESCRIPTION "libkdegamesprivate KGame"
    EXPORT KDEGAMES
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
    kgamesvgdocument.cpp
)

target_link_libraries(KDEGamesPrivate
    PUBLIC
        KDEGames
        KF6::Completion
        Qt6::Xml
        Qt6::Network
    PRIVATE
        KF6::DNSSD
        KF6::NewStuffWidgets
        KF6::Archive
        KF6::XmlGui
        Qt6::Core5Compat
)

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
    ${CMAKE_CURRENT_BINARY_DIR}/kdegamesprivate_export.h
    kchatbase.h
    kchatbaseitemdelegate.h
    kchatbasemodel.h
    kgamecanvas.h
    kgamesvgdocument.h
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
