add_subdirectory(includes)
add_subdirectory(carddecks)
add_subdirectory(highscore)
add_subdirectory(qml)
add_subdirectory(private)

include_directories(
    ${OPENAL_INCLUDE_DIR}
    ${SNDFILE_INCLUDE_DIR}
    highscore
    ${CMAKE_CURRENT_BINARY_DIR}/highscore
    # the following only for libkdegamesprivate, but there aren't
    # target-specific include directories
    private/kgame
)

configure_file(libkdegames_capabilities.h.in ${CMAKE_CURRENT_BINARY_DIR}/libkdegames_capabilities.h)
configure_file(highscore/config-highscore.h.in ${CMAKE_CURRENT_BINARY_DIR}/highscore/config-highscore.h )

set(kdegames_LIB_SRCS
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

add_library(KF5KDEGames SHARED ${kdegames_LIB_SRCS})

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
        Qt5::Widgets
        Qt5::QuickWidgets
        Qt5::Qml
        KF5::ConfigCore
        KF5::I18n
        KF5::WidgetsAddons
        KF5::ConfigWidgets
    PRIVATE
        ${KGAUDIO_LINKLIBS}
        Qt5::Xml
        Qt5::Svg
        Qt5::Quick
        KF5::Declarative
        KF5::NewStuff
        KF5::IconThemes
        KF5::GuiAddons
        KF5::Completion
)

target_include_directories(KF5KDEGames
    INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KF5KDEGames;${KF5_INCLUDE_INSTALL_DIR}/KF5KDEGames/KDE;${KF5_INCLUDE_INSTALL_DIR}>"
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/includes>"
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

install(TARGETS KF5KDEGames EXPORT KF5KDEGamesLibraryDepends ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


# NOTE: The libkdegamesprivate target is compiled in this directory, because
# CMake can't cope with exported libraries in two different directories.

set(kdegamesprivate_LIB_SRCS
    private/kchatbase.cpp
    private/kchatbaseitemdelegate.cpp
    private/kchatbasemodel.cpp
    private/kgame/kgamechat.cpp
    private/kgame/kgame.cpp
    private/kgame/kgameerror.cpp
    private/kgame/kgameio.cpp
    private/kgame/kgamemessage.cpp
    private/kgame/kgamenetwork.cpp
    private/kgame/kgameproperty.cpp
    private/kgame/kgamepropertyhandler.cpp
    private/kgame/kgamesequence.cpp
    private/kgame/kmessageclient.cpp
    private/kgame/kmessageio.cpp
    private/kgame/kmessageserver.cpp
    private/kgame/kplayer.cpp
    private/kgamecanvas.cpp
    private/kgamedifficulty.cpp
    private/kgamesvgdocument.cpp
    private/kgametheme.cpp
    private/kgamethemeselector.cpp
)

ki18n_wrap_ui(kdegamesprivate_LIB_SRCS
    private/kgamethemeselector.ui
)

add_library(KF5KDEGamesPrivate SHARED ${kdegamesprivate_LIB_SRCS})
ecm_generate_export_header(KF5KDEGamesPrivate
    EXPORT_FILE_NAME libkdegamesprivate_export.h
    BASE_NAME KDEGAMESPRIVATE
    VERSION ${KDEGAMES_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 3.2 4.9 7.3
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
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

target_include_directories(KF5KDEGamesPrivate INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KF5KDEGames/libkdegamesprivate>" )

set_target_properties(KF5KDEGamesPrivate PROPERTIES
    VERSION   ${KDEGAMES_VERSION}
    SOVERSION ${KDEGAMES_SOVERSION}
)

install(TARGETS KF5KDEGamesPrivate EXPORT KF5KDEGamesLibraryDepends ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install(FILES kgthemeprovider-migration.upd
    DESTINATION ${DATA_INSTALL_DIR}/kconf_update)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libkdegames_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/libkdegames_capabilities.h
    audio/kgaudioscene.h
    audio/kgsound.h
    kgameclock.h
    kgamepopupitem.h
    kgamerendereditem.h
    kgamerenderedobjectitem.h
    kgamerendererclient.h
    kgamerenderer.h
    kgdeclarativeview.h
    kgdifficulty.h
    kgtheme.h
    kgthemeprovider.h
    kgthemeselector.h
    kstandardgameaction.h
DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KF5KDEGames COMPONENT Devel)

########### generate exports ###############

# add libraries to the build-tree export set
export(TARGETS KF5KDEGames KF5KDEGamesPrivate
    FILE "${PROJECT_BINARY_DIR}/KF5KDEGamesLibraryDepends.cmake")

# define the installation directory for the CMake files
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5KDEGames")

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

# install the export set for use with the install-tree
install(EXPORT KF5KDEGamesLibraryDepends
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT Devel
)
	
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdegames_version.h
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KF5KDEGames
    COMPONENT Devel
)

install(FILES libkdegames.categories  DESTINATION  ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
