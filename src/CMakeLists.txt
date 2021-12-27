set(kdegames_INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KF5KDEGames)

add_subdirectory(carddecks)
add_subdirectory(qml)
add_subdirectory(private)

configure_file(libkdegames_capabilities.h.in ${CMAKE_CURRENT_BINARY_DIR}/libkdegames_capabilities.h)
configure_file(highscore/config-highscore.h.in ${CMAKE_CURRENT_BINARY_DIR}/highscore/config-highscore.h )

add_library(KF5KDEGames SHARED)

target_sources(KF5KDEGames PRIVATE
    audio/kgaudioscene-${KGAUDIO_BACKEND}.cpp
    audio/kgsound-${KGAUDIO_BACKEND}.cpp
    audio/virtualfileqt-${KGAUDIO_BACKEND}.cpp
    colorproxy_p.cpp
#   highscore/kconfigrawbackend.cpp
    highscore/khighscore.cpp
    highscore/kscoredialog.cpp
    kgameclock.cpp
    kgamepopupitem.cpp
    kgamerendereditem.cpp
    kgamerenderedobjectitem.cpp
    kgamerendererclient.cpp
    kgamerenderer.cpp
    kgdeclarativeview.cpp
    kgimageprovider.cpp
    kgdifficulty.cpp
    kgtheme.cpp
    kgthemeprovider.cpp
    kgthemeselector.cpp
    kstandardgameaction.cpp
)

ecm_generate_headers(kdegames_CamelCase_HEADERS
    HEADER_NAMES
        KGameClock
        KGamePopupItem
        KGameRenderedItem
        KGameRenderedObjectItem
        KGameRenderer
        KGameRendererClient
        KgDeclarativeView
        KgDifficulty
        KgTheme
        KgThemeProvider
        KgThemeSelector
        KStandardGameAction
    REQUIRED_HEADERS kdegames_HEADERS
)

ecm_generate_headers(kdegames_CamelCase_HEADERS
    HEADER_NAMES
        KHighscore
        KScoreDialog
    RELATIVE highscore
    PREFIX highscore
    REQUIRED_HEADERS kdegames_highscore_HEADERS
)

ecm_generate_headers(kdegames_CamelCase_HEADERS
    HEADER_NAMES
        KgAudioScene
        KgSound
    RELATIVE audio
    REQUIRED_HEADERS kdegames_HEADERS
)

ecm_qt_export_logging_category(
    IDENTIFIER GAMES_LIB
    CATEGORY_NAME org.kde.games.lib
    DESCRIPTION "libkdegames"
    EXPORT KDEGAMES
)

ecm_qt_export_logging_category(
    IDENTIFIER GAMES_UI
    CATEGORY_NAME org.kde.games.ui
    DESCRIPTION "libkdegames UI"
    EXPORT KDEGAMES
)

ecm_qt_export_logging_category(
    IDENTIFIER GAMES_HIGHSCORE
    CATEGORY_NAME org.kde.games.highscore
    DESCRIPTION "libkdegames Highscore"
    EXPORT KDEGAMES
)

ecm_generate_export_header(KF5KDEGames
    EXPORT_FILE_NAME libkdegames_export.h
    BASE_NAME KDEGAMES
    VERSION ${KDEGAMES_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 4.1
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_link_libraries(KF5KDEGames
    PUBLIC
        Qt${QT_MAJOR_VERSION}::Widgets
        Qt${QT_MAJOR_VERSION}::QuickWidgets
        Qt${QT_MAJOR_VERSION}::Qml
        KF5::ConfigCore
        KF5::I18n
        KF5::WidgetsAddons
        KF5::ConfigWidgets
    PRIVATE
        ${KGAUDIO_LINKLIBS}
        Qt${QT_MAJOR_VERSION}::Xml
        Qt${QT_MAJOR_VERSION}::Svg
        Qt${QT_MAJOR_VERSION}::Quick
        KF5::Declarative
        KF5::NewStuff
        KF5::IconThemes
        KF5::XmlGui
        KF5::GuiAddons
        KF5::Completion
)

target_include_directories(KF5KDEGames
    INTERFACE
        "$<INSTALL_INTERFACE:${kdegames_INCLUDE_INSTALL_DIR}>"
    PRIVATE
        ${OPENAL_INCLUDE_DIR}
        ${SNDFILE_INCLUDE_DIR}
        highscore
        ${CMAKE_CURRENT_BINARY_DIR}/highscore
)

ecm_setup_version(${KDEGAMES_VERSION}
    VARIABLE_PREFIX KDEGAMES
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdegames_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KDEGamesConfigVersion.cmake"
    SOVERSION ${KDEGAMES_SOVERSION}
)

set_target_properties(KF5KDEGames PROPERTIES
    EXPORT_NAME KF5KDEGames
    VERSION   ${KDEGAMES_VERSION}
    SOVERSION ${KDEGAMES_SOVERSION}
)

install(TARGETS KF5KDEGames EXPORT KF5KDEGamesTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install(FILES kgthemeprovider-migration.upd
    DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR}
)

install(FILES ${kdegames_CamelCase_HEADERS}
    DESTINATION ${kdegames_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(FILES
    ${kdegames_highscore_HEADERS}
    DESTINATION ${kdegames_INCLUDE_INSTALL_DIR}/highscore
    COMPONENT Devel
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libkdegames_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/libkdegames_capabilities.h
    ${kdegames_HEADERS}
    DESTINATION ${kdegames_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

########### generate exports ###############

# define the installation directory for the CMake files
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5KDEGames")

# create the Config.cmake and ConfigVersion.cmake files
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KDEGamesConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5KDEGamesConfig.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KF5KDEGamesConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5KDEGamesConfigVersion.cmake"
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT Devel
)

install(EXPORT KF5KDEGamesTargets
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    FILE KF5KDEGamesTargets.cmake
    COMPONENT Devel
)
	
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdegames_version.h
    DESTINATION ${kdegames_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT KDEGAMES
    FILE libkdegames.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
