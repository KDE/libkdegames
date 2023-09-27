set(kdegames_INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/KDEGames6)
set(KDEGAMES_CMAKECONFIG_NAME "KDEGames6")
set(KDEGAMES_OUTPUT_NAME KDEGames6)
set(KDEGAMES_EXPORT_NAME KDEGames6)

add_subdirectory(carddecks)
add_subdirectory(qml)
add_subdirectory(private)

configure_file(libkdegames_capabilities.h.in ${CMAKE_CURRENT_BINARY_DIR}/libkdegames_capabilities.h)
configure_file(highscore/config-highscore.h.in ${CMAKE_CURRENT_BINARY_DIR}/highscore/config-highscore.h )

add_library(KDEGames SHARED)

target_sources(KDEGames PRIVATE
    audio/kgaudioscene-${KGAUDIO_BACKEND}.cpp
    audio/kgsound-${KGAUDIO_BACKEND}.cpp
    audio/virtualfileqt-${KGAUDIO_BACKEND}.cpp
    colorproxy_p.cpp colorproxy_p.h
#   highscore/kconfigrawbackend.cpp
    highscore/khighscore.cpp highscore/khighscore.h
    highscore/kscoredialog.cpp highscore/kscoredialog.h
    kgameclock.cpp kgameclock.h
    kgamepopupitem.cpp kgamepopupitem.h
    kgamerendereditem.cpp kgamerendereditem.h
    kgamerenderedobjectitem.cpp kgamerenderedobjectitem.h
    kgamerendererclient.cpp kgamerendererclient.h
    kgamerenderer.cpp kgamerenderer.h
    kgdeclarativeview.cpp kgdeclarativeview.h
    kgimageprovider.cpp
    kgdifficulty.cpp kgdifficulty.h
    kgtheme.cpp kgtheme.h
    kgthemeprovider.cpp kgthemeprovider.h
    kgthemeselector.cpp kgthemeselector.h
    kstandardgameaction.cpp kstandardgameaction.h
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
        KgDifficulty,KgDifficultyLevel
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

ecm_generate_export_header(KDEGames
    EXPORT_FILE_NAME libkdegames_export.h
    BASE_NAME KDEGAMES
    VERSION ${KDEGAMES_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 4.1 7.4 7.5
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    USE_VERSION_HEADER
)

target_link_libraries(KDEGames
    PUBLIC
        KF6::ConfigCore
        KF6::I18n
        KF6::WidgetsAddons
        KF6::ConfigWidgets
        Qt6::Widgets
        Qt6::QuickWidgets
        Qt6::Qml
    PRIVATE
        ${KGAUDIO_LINKLIBS}
        KF6::NewStuffWidgets
        KF6::IconThemes
        KF6::XmlGui
        KF6::GuiAddons
        KF6::Completion
        Qt6::Xml
        Qt6::Svg
        Qt6::Quick
)

target_include_directories(KDEGames
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
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/${KDEGAMES_CMAKECONFIG_NAME}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
    SOVERSION ${KDEGAMES_SOVERSION}
)

set_target_properties(KDEGames PROPERTIES
    OUTPUT_NAME ${KDEGAMES_OUTPUT_NAME}
    EXPORT_NAME ${KDEGAMES_EXPORT_NAME}
    VERSION   ${KDEGAMES_VERSION}
    SOVERSION ${KDEGAMES_SOVERSION}
)

install(TARGETS KDEGames EXPORT KDEGamesTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

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
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/${KDEGAMES_CMAKECONFIG_NAME}")

# create the Config.cmake and ConfigVersion.cmake files
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KDEGamesConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${KDEGAMES_CMAKECONFIG_NAME}Config.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${KDEGAMES_CMAKECONFIG_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${KDEGAMES_CMAKECONFIG_NAME}ConfigVersion.cmake"
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT Devel
)

install(EXPORT KDEGamesTargets
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    FILE ${KDEGAMES_CMAKECONFIG_NAME}Targets.cmake
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
