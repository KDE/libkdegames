set(kdegames_INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/KDEGames6)
set(KDEGAMES_CMAKECONFIG_NAME "KDEGames6")
set(KDEGAMES_OUTPUT_NAME KDEGames6)
set(KDEGAMES_EXPORT_NAME KDEGames6)

set(kdegamesgui_INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/KDEGames6Gui)
set(KDEGAMESGUI_CMAKECONFIG_NAME "KDEGames6Gui")
set(KDEGAMESGUI_OUTPUT_NAME KDEGames6Gui)
set(KDEGAMESGUI_EXPORT_NAME KDEGames6Gui)

set(kdegameswidgets_INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/KDEGames6Widgets)
set(KDEGAMESWIDGETS_CMAKECONFIG_NAME "KDEGames6Widgets")
set(KDEGAMESWIDGETS_OUTPUT_NAME KDEGames6Widgets)
set(KDEGAMESWIDGETS_EXPORT_NAME KDEGames6Widgets)

ecm_setup_version(PROJECT
    VARIABLE_PREFIX KDEGAMES
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdegames_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/${KDEGAMES_CMAKECONFIG_NAME}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
    SOVERSION 7
)

add_subdirectory(carddecks)
add_subdirectory(qml)
add_subdirectory(private)

configure_file(libkdegames_capabilities.h.in ${CMAKE_CURRENT_BINARY_DIR}/libkdegames_capabilities.h)
configure_file(highscore/config-highscore.h.in ${CMAKE_CURRENT_BINARY_DIR}/highscore/config-highscore.h )

add_library(KDEGamesGui)

set_target_properties(KDEGamesGui PROPERTIES
    OUTPUT_NAME ${KDEGAMESGUI_OUTPUT_NAME}
    EXPORT_NAME ${KDEGAMESGUI_EXPORT_NAME}
    VERSION     ${KDEGAMES_VERSION}
    SOVERSION   ${KDEGAMES_SOVERSION}
)

target_sources(KDEGamesGui PRIVATE
    audio/kgameaudioscene-${KGAUDIO_BACKEND}.cpp
    audio/kgamesound-${KGAUDIO_BACKEND}.cpp
    audio/virtualfileqt-${KGAUDIO_BACKEND}.cpp
    colorproxy_p.cpp colorproxy_p.h
#   highscore/kconfigrawbackend.cpp
    highscore/khighscore.cpp highscore/khighscore.h
    kgameclock.cpp kgameclock.h
    kgamerendererclient.cpp kgamerendererclient.h
    kgamerenderer.cpp kgamerenderer.h
    kgameimageprovider.cpp kgameimageprovider_p.h
    kgamedifficulty.cpp kgamedifficulty.h
    kgametheme.cpp kgametheme.h
    kgamethemeprovider.cpp kgamethemeprovider.h
    kstandardgameaction.cpp kstandardgameaction.h
)

ecm_generate_headers(kdegamesgui_CamelCase_HEADERS
    HEADER_NAMES
        KGameClock
        KGameRenderer
        KGameRendererClient
        KGameDifficulty,KGameDifficultyLevel
        KGameTheme
        KGameThemeProvider
        KStandardGameAction
    REQUIRED_HEADERS kdegamesgui_HEADERS
)

ecm_generate_headers(kdegamesgui_CamelCase_HEADERS
    HEADER_NAMES
        KHighscore
    RELATIVE highscore
    PREFIX highscore
    REQUIRED_HEADERS kdegamesgui_highscore_HEADERS
)

ecm_generate_headers(kdegamesgui_CamelCase_HEADERS
    HEADER_NAMES
        KGameAudioScene
        KGameSound
    RELATIVE audio
    REQUIRED_HEADERS kdegamesgui_HEADERS
)

ecm_qt_declare_logging_category(KDEGamesGui
    HEADER kdegames_logging.h
    IDENTIFIER KDEGAMES_LOG
    CATEGORY_NAME kdegames
    DESCRIPTION "libkdegames"
    EXPORT KDEGAMES
)

ecm_qt_declare_logging_category(KDEGamesGui
    HEADER kdegames_audio_logging.h
    IDENTIFIER KDEGAMES_AUDIO_LOG
    CATEGORY_NAME kdegames.audio
    DESCRIPTION "libkdegames Audio"
    EXPORT KDEGAMES
)

ecm_qt_declare_logging_category(KDEGamesGui
    HEADER kdegames_highscore_logging.h
    IDENTIFIER KDEGAMES_HIGHSCORE_LOG
    CATEGORY_NAME kdegames.highscore
    DESCRIPTION "libkdegames Highscore"
    EXPORT KDEGAMES
)

ecm_generate_export_header(KDEGamesGui
    BASE_NAME KDEGames
    VERSION ${KDEGAMES_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    USE_VERSION_HEADER
)

target_link_libraries(KDEGamesGui
    PUBLIC
        KF6::ConfigCore # KHighscore API, uses KConfig
        KF6::WidgetsAddons # KStandardGameAction API, uses KSelectAction:QWidgetAction 
        KF6::ConfigWidgets # KStandardGameAction API, uses KRecentFilesAction
        Qt6::Qml # KGameThemeProvider API
    PRIVATE
        ${KGAUDIO_LINKLIBS}
        KF6::I18n
        KF6::IconThemes
        KF6::XmlGui
        KF6::GuiAddons
        KF6::Completion
        KF6::CoreAddons
        Qt6::Xml
        Qt6::Svg
        Qt6::Quick # KGameImageProvider:QQuickImageProvider, used by KGameThemeProvider::setDeclarativeEngine
)

target_include_directories(KDEGamesGui
    INTERFACE
        "$<INSTALL_INTERFACE:${kdegamesgui_INCLUDE_INSTALL_DIR}>"
        "$<INSTALL_INTERFACE:${kdegames_INCLUDE_INSTALL_DIR}>" # version header
    PRIVATE
        ${OPENAL_INCLUDE_DIR}
        ${SNDFILE_INCLUDE_DIR}
        highscore
        ${CMAKE_CURRENT_BINARY_DIR}/highscore
)

install(TARGETS KDEGamesGui EXPORT KDEGamesTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES ${kdegamesgui_CamelCase_HEADERS}
    DESTINATION ${kdegamesgui_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(FILES
    ${kdegamesgui_highscore_HEADERS}
    DESTINATION ${kdegamesgui_INCLUDE_INSTALL_DIR}/highscore
    COMPONENT Devel
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kdegames_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/libkdegames_capabilities.h
    ${kdegamesgui_HEADERS}
    DESTINATION ${kdegamesgui_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

########### install files ###############

add_library(KDEGamesWidgets)

set_target_properties(KDEGamesWidgets PROPERTIES
    OUTPUT_NAME ${KDEGAMESWIDGETS_OUTPUT_NAME}
    EXPORT_NAME ${KDEGAMESWIDGETS_EXPORT_NAME}
    VERSION     ${KDEGAMES_VERSION}
    SOVERSION   ${KDEGAMES_SOVERSION}
)

target_sources(KDEGamesWidgets PRIVATE
    kgamepopupitem.cpp kgamepopupitem.h
    kgamerendereditem.cpp kgamerendereditem.h
    kgamerenderedgraphicsobject.cpp kgamerenderedgraphicsobject.h
    kgamegraphicsviewrenderer.cpp kgamegraphicsviewrenderer.h
    kgamethemeselector.cpp kgamethemeselector.h
    highscore/kscoredialog.cpp highscore/kscoredialog.h
)

ecm_generate_export_header(KDEGamesWidgets
    VERSION ${KDEGAMES_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    USE_VERSION_HEADER
    VERSION_BASE_NAME KDEGames
)

ecm_generate_headers(kdegameswidgets_CamelCase_HEADERS
    HEADER_NAMES
        KGamePopupItem
        KGameRenderedItem
        KGameRenderedGraphicsObject
        KGameGraphicsViewRenderer
        KGameThemeSelector
    REQUIRED_HEADERS kdegameswidgets_HEADERS
)

ecm_generate_headers(kdegameswidgets_CamelCase_HEADERS
    HEADER_NAMES
        KScoreDialog
    RELATIVE highscore
    PREFIX highscore
    REQUIRED_HEADERS kdegameswidgets_highscore_HEADERS
)

ecm_qt_declare_logging_category(KDEGamesWidgets
    HEADER kdegameswidgets_highscore_logging.h
    IDENTIFIER KDEGAMESWIDGETS_HIGHSCORE_LOG
    CATEGORY_NAME kdegames.widgets.highscore
    DESCRIPTION "libkdegameswidgets Highscore"
    EXPORT KDEGAMES
)

target_link_libraries(KDEGamesWidgets
    PUBLIC
        KDEGamesGui
    PRIVATE
        KF6::NewStuffWidgets
        KF6::I18n
        KF6::CoreAddons
        KF6::Completion
)

target_include_directories(KDEGamesWidgets
    INTERFACE
        "$<INSTALL_INTERFACE:${kdegameswidgets_INCLUDE_INSTALL_DIR}>"
        "$<INSTALL_INTERFACE:${kdegames_INCLUDE_INSTALL_DIR}>" # version header
    PRIVATE
        highscore
        ${CMAKE_CURRENT_BINARY_DIR}/highscore
)

install(TARGETS KDEGamesWidgets EXPORT KDEGamesTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES ${kdegameswidgets_CamelCase_HEADERS}
    DESTINATION ${kdegameswidgets_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(FILES
    ${kdegameswidgets_highscore_HEADERS}
    DESTINATION ${kdegameswidgets_INCLUDE_INSTALL_DIR}/highscore
    COMPONENT Devel
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kdegameswidgets_export.h
    ${kdegameswidgets_HEADERS}
    DESTINATION ${kdegameswidgets_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

########### install files ###############

add_library(KDEGames)

set_target_properties(KDEGames PROPERTIES
    OUTPUT_NAME ${KDEGAMES_OUTPUT_NAME}
    EXPORT_NAME ${KDEGAMES_EXPORT_NAME}
    VERSION     ${KDEGAMES_VERSION}
    SOVERSION   ${KDEGAMES_SOVERSION}
)

target_sources(KDEGames PRIVATE
)

target_link_libraries(KDEGames
    PUBLIC
        KDEGamesWidgets
)

install(TARGETS KDEGames EXPORT KDEGamesTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES ${kdegames_CamelCase_HEADERS}
    DESTINATION ${kdegames_INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kdegames_export.h
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
