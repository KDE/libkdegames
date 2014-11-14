enable_testing()

project(libkdegames)

cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)
set (QT_MIN_VERSION "5.3.0")

find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})


find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets Qml Quick QuickWidgets Svg Test)
find_package(KF5 REQUIRED COMPONENTS CoreAddons Config WidgetsAddons Codecs Archive
    DBusAddons DNSSD Declarative
    I18n GuiAddons Service ConfigWidgets ItemViews IconThemes Completion JobWidgets TextWidgets GlobalAccel XmlGui Crash
    Bookmarks KIO NewStuff KDELibs4Support)

include(FeatureSummary)
include(GenerateExportHeader)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMMarkNonGuiExecutable)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DMAKE_KDEGAMESPRIVATE_LIB)
add_definitions(-DTRANSLATION_DOMAIN="libkdegames")

include_directories(${CMAKE_SOURCE_DIR}/includes)

find_package(OpenAL REQUIRED)
set_package_properties(OPENAL PROPERTIES
                       URL "http://connect.creativelabs.com/openal"
                      )

find_package(SndFile REQUIRED)
set_package_properties(SndFile PROPERTIES
                       URL "http://www.mega-nerd.com/libsndfile/"
                      )

set(HIGHSCORE_DIRECTORY "" CACHE STRING "Where to install system-wide highscores e.g. /var/games")

configure_file(highscore/config-highscore.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/highscore/config-highscore.h )

add_subdirectory( carddecks )
add_subdirectory( declarativeimports )
add_subdirectory( highscore )
add_subdirectory( includes )
add_subdirectory( libkdegamesprivate )
add_subdirectory( tests )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/highscore
    ${CMAKE_CURRENT_BINARY_DIR}/highscore
    # the following only for libkdegamesprivate, but there aren't
    # target-specific include directories
    ${CMAKE_CURRENT_SOURCE_DIR}/libkdegamesprivate/kgame
    ${CMAKE_CURRENT_SOURCE_DIR}/libkdegamesprivate/..
)

option (USE_OPENAL_SNDFILE "use OpenAL and libsndfile in libkdegames" ON)
if (SNDFILE_FOUND AND USE_OPENAL_SNDFILE)
    message(STATUS "Checking libsndfile capabilities")
    try_compile(SNDFILE_WORKS
        ${CMAKE_CURRENT_BINARY_DIR}/audio/check-libsndfile-capabilities
        ${CMAKE_CURRENT_SOURCE_DIR}/audio/check-libsndfile-capabilities.cpp
        CMAKE_FLAGS -DINCLUDE_DIRECTORIES=${SNDFILE_INCLUDE_DIR})
    if (NOT SNDFILE_WORKS)
        message(FATAL_ERROR "Your version of libsndfile (found in " ${SNDFILE_LIBRARIES} ") is too old. At least version 0.21 is needed. To skip the optional OpenAL/libsndfile dependency in libkdegames (not recommended), re-run cmake with -DUSE_OPENAL_SNDFILE=OFF.")
    endif (NOT SNDFILE_WORKS)
endif (SNDFILE_FOUND AND USE_OPENAL_SNDFILE)

message (STATUS "INCLUDES FOR SOUND:  " ${OPENAL_INCLUDE_DIR} " " ${SNDFILE_INCLUDE_DIR})
message (STATUS "LIBRARIES FOR SOUND: " ${OPENAL_LIBRARY} " " ${SNDFILE_LIBRARIES})
include_directories(${OPENAL_INCLUDE_DIR} ${SNDFILE_INCLUDE_DIR})
set(KGAUDIO_LINKLIBS ${OPENAL_LIBRARY} ${SNDFILE_LIBRARIES})
set(KGAUDIO_BACKEND openal)
set(KGAUDIO_BACKEND_OPENAL TRUE) # for configure_file() below

configure_file(libkdegames_capabilities.h.in ${CMAKE_CURRENT_BINARY_DIR}/libkdegames_capabilities.h)

########### next target ###############

set(kdegames_LIB_SRCS
    audio/kgaudioscene-${KGAUDIO_BACKEND}.cpp
    audio/kgsound-${KGAUDIO_BACKEND}.cpp
    colorproxy_p.cpp
#   highscore/kconfigrawbackend.cpp
    highscore/kexthighscore.cpp
    highscore/kexthighscore_gui.cpp
    highscore/kexthighscore_internal.cpp
    highscore/kexthighscore_item.cpp
    highscore/kexthighscore_tab.cpp
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
if(Q_WS_X11)
	set(kdegames_LIB_SRCS ${kdegames_LIB_SRCS} chooserastergraphicssystem.cpp)
endif(Q_WS_X11)

add_library(KF5KDEGames SHARED ${kdegames_LIB_SRCS})

generate_export_header(KF5KDEGames BASE_NAME libkdegames EXPORT_MACRO_NAME KDEGAMES_EXPORT DEPRECATED_MACRO_NAME KDE_DEPRECATED)

target_link_libraries(KF5KDEGames
                PRIVATE
                      ${KGAUDIO_LINKLIBS}
                      Qt5::Xml
                      Qt5::Svg
                      Qt5::Quick
                      KF5::Declarative
                      KF5::KIOWidgets
                      KF5::NewStuff
                      KF5::KDELibs4Support
                PUBLIC
                      Qt5::Widgets
                      Qt5::QuickWidgets
                      Qt5::Qml
                      KF5::ConfigCore
                      KF5::I18n
                      KF5::WidgetsAddons
                      KF5::ConfigWidgets
)

target_include_directories(KF5KDEGames INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KF5KDEGames;${KF5_INCLUDE_INSTALL_DIR}/KF5KDEGames/KDE;${KF5_INCLUDE_INSTALL_DIR}>")

#NOTE: 6.0.0 = KDE 4.9 release
set(KDEGAMES_VERSION 6.1.0)
set(KDEGAMES_SOVERSION 6)

set_target_properties(KF5KDEGames PROPERTIES VERSION   ${KDEGAMES_VERSION}
                                             SOVERSION ${KDEGAMES_SOVERSION}
                                             EXPORT_NAME KF5KDEGames
)

ecm_setup_version(${KDEGAMES_VERSION} VARIABLE_PREFIX KDEGAMES
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdegames_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KDEGamesConfigVersion.cmake"
                        SOVERSION 6)

set_target_properties(KF5KDEGames PROPERTIES
    VERSION   ${KDEGAMES_VERSION}
    SOVERSION ${KDEGAMES_SOVERSION}
)

install(TARGETS KF5KDEGames EXPORT KF5KDEGamesLibraryDepends
    ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

# NOTE: The libkdegamesprivate target is compiled in this directory, because
# CMake can't cope with exported libraries in two different directories.

set(kdegamesprivate_LIB_SRCS
    libkdegamesprivate/kchatbase.cpp
    libkdegamesprivate/kchatbaseitemdelegate.cpp
    libkdegamesprivate/kchatbasemodel.cpp
    libkdegamesprivate/kgame/kgamechat.cpp
    libkdegamesprivate/kgame/kgame.cpp
    libkdegamesprivate/kgame/kgameerror.cpp
    libkdegamesprivate/kgame/kgameio.cpp
    libkdegamesprivate/kgame/kgamemessage.cpp
    libkdegamesprivate/kgame/kgamenetwork.cpp
    libkdegamesprivate/kgame/kgameproperty.cpp
    libkdegamesprivate/kgame/kgamepropertyhandler.cpp
    libkdegamesprivate/kgame/kgamesequence.cpp
    libkdegamesprivate/kgame/kmessageclient.cpp
    libkdegamesprivate/kgame/kmessageio.cpp
    libkdegamesprivate/kgame/kmessageserver.cpp
    libkdegamesprivate/kgame/kplayer.cpp
    libkdegamesprivate/kgamecanvas.cpp
    libkdegamesprivate/kgamedifficulty.cpp
    libkdegamesprivate/kgamesvgdocument.cpp
    libkdegamesprivate/kgametheme.cpp
    libkdegamesprivate/kgamethemeselector.cpp
)

ki18n_wrap_ui(kdegamesprivate_LIB_SRCS
    libkdegamesprivate/kgamethemeselector.ui
)

add_library(KF5KDEGamesPrivate SHARED ${kdegamesprivate_LIB_SRCS})

target_link_libraries(KF5KDEGamesPrivate
                PRIVATE
                      KF5::DNSSD
                      KF5::NewStuff
                      KF5::Archive
                PUBLIC
                      Qt5::Xml
                      Qt5::Network
                      KF5::Completion
                      KF5::KDELibs4Support
                      KF5KDEGames
)

target_include_directories(KF5KDEGamesPrivate INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KF5KDEGames/libkdegamesprivate>" )

set_target_properties(KF5KDEGamesPrivate PROPERTIES
    VERSION   1.0.0
    SOVERSION 1
)

install(TARGETS KF5KDEGamesPrivate EXPORT KF5KDEGamesLibraryDepends
    ${INSTALL_TARGETS_DEFAULT_ARGS})


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
ecm_configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KDEGamesConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF5KDEGamesConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5KDEGamesConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF5KDEGamesConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

# install the export set for use with the install-tree
install(EXPORT KF5KDEGamesLibraryDepends DESTINATION
    ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel)
	
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdegames_version.h  DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KF5KDEGames  COMPONENT Devel)

feature_summary(WHAT ALL
                INCLUDE_QUIET_PACKAGES
                FATAL_ON_MISSING_REQUIRED_PACKAGES)

