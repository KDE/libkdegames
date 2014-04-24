enable_testing()

project(libkdegames)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})


find_package(Qt5 5.2 REQUIRED NO_MODULE COMPONENTS Widgets Qml Quick Svg Test)
find_package(KF5 REQUIRED COMPONENTS CoreAddons Config ItemModels WidgetsAddons WindowSystem Codecs Archive
    Config DBusAddons DNSSD Declarative
    I18n GuiAddons Service ConfigWidgets ItemViews IconThemes Completion JobWidgets TextWidgets GlobalAccel XmlGui Crash
    Bookmarks KIO NotifyConfig NewStuff KDE4Support)

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DMAKE_KDEGAMESPRIVATE_LIB)
include_directories(${KDE4_INCLUDES} ${CMAKE_SOURCE_DIR}/includes)

find_package(OpenAL)
set_package_properties(OPENAL PROPERTIES
                       URL "http://connect.creativelabs.com/openal"
                       TYPE RECOMMENDED 
                       PURPOSE "Phonon is used as a fallback, but low-latency sound output is only available with OpenAL/SndFile"
                      )

find_package(SndFile)
set_package_properties(SndFile PROPERTIES
                       URL "http://www.mega-nerd.com/libsndfile/"
                       TYPE RECOMMENDED
                       PURPOSE "Phonon is used as a fallback, but low-latency sound output is only available with OpenAL/SndFile"
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

if (OPENAL_FOUND AND SNDFILE_FOUND AND SNDFILE_WORKS)
    message (STATUS "INCLUDES FOR SOUND:  " ${OPENAL_INCLUDE_DIR} " " ${SNDFILE_INCLUDE_DIR})
    message (STATUS "LIBRARIES FOR SOUND: " ${OPENAL_LIBRARY} " " ${SNDFILE_LIBRARIES})
    include_directories(${OPENAL_INCLUDE_DIR} ${SNDFILE_INCLUDE_DIR})
    set(KGAUDIO_LINKLIBS ${OPENAL_LIBRARY} ${SNDFILE_LIBRARIES})
    set(KGAUDIO_BACKEND openal)
    set(KGAUDIO_BACKEND_OPENAL TRUE) # for configure_file() below
else (OPENAL_FOUND AND SNDFILE_FOUND AND SNDFILE_WORKS)
    message (STATUS "FALLBACK LIBRARY FOR SOUND: " ${KDE4_PHONON_LIBS})
    set(KGAUDIO_LINKLIBS ${KDE4_PHONON_LIBS})
    set(KGAUDIO_BACKEND phonon)
    set(KGAUDIO_BACKEND_PHONON TRUE) # for configure_file() below
endif (OPENAL_FOUND AND SNDFILE_FOUND AND SNDFILE_WORKS)

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

add_library(kdegames SHARED ${kdegames_LIB_SRCS})

target_link_libraries(kdegames KF5::KIOWidgets KF5::NewStuff KF5::KDE4Support Qt5::Xml Qt5::Svg Qt5::Qml Qt5::Quick KF5::Declarative ${KGAUDIO_LINKLIBS})
target_link_libraries(kdegames LINK_INTERFACE_LIBRARIES KF5::KDE4Support)

#NOTE: 6.0.0 = KDE 4.9 release
set(KDEGAMES_VERSION 6.1.0)
set(KDEGAMES_SOVERSION 6)

set_target_properties(kdegames PROPERTIES
    VERSION   ${KDEGAMES_VERSION}
    SOVERSION ${KDEGAMES_SOVERSION}
)

install(TARGETS kdegames EXPORT KDEGamesLibraryDepends
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

add_library(kdegamesprivate SHARED ${kdegamesprivate_LIB_SRCS})

target_link_libraries(kdegamesprivate KF5::DNSSD KF5::NewStuff KF5::Archive Qt5::Xml Qt5::Network kdegames)
target_link_libraries(kdegamesprivate LINK_INTERFACE_LIBRARIES Qt5::Xml Qt5::Network kdegames)

set_target_properties(kdegamesprivate PROPERTIES
    VERSION   1.0.0
    SOVERSION 1
)

install(TARGETS kdegamesprivate EXPORT KDEGamesLibraryDepends
    ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install(FILES kgthemeprovider-migration.upd
    DESTINATION ${DATA_INSTALL_DIR}/kconf_update)

install(FILES
    libkdegames_export.h
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
DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

########### generate exports ###############

# add libraries to the build-tree export set
export(TARGETS kdegames kdegamesprivate
    FILE "${PROJECT_BINARY_DIR}/KDEGamesLibraryDepends.cmake")

# define the installation directory for the CMake files
set(CMAKECONFIG_INSTALL_DIR ${LIB_INSTALL_DIR}/cmake/KDEGames)

# create the Config.cmake and ConfigVersion.cmake files
file(RELATIVE_PATH CONF_REL_INCLUDE_DIR
    "${CMAKE_INSTALL_PREFIX}/${CMAKECONFIG_INSTALL_DIR}"
    "${CMAKE_INSTALL_PREFIX}/${INCLUDE_INSTALL_DIR}")
configure_file(KDEGamesConfig.cmake.in
    "${PROJECT_BINARY_DIR}/KDEGamesConfig.cmake" @ONLY)
configure_file(KDEGamesConfigVersion.cmake.in
    "${PROJECT_BINARY_DIR}/KDEGamesConfigVersion.cmake" @ONLY)

install(FILES
    "${PROJECT_BINARY_DIR}/KDEGamesConfig.cmake"
    "${PROJECT_BINARY_DIR}/KDEGamesConfigVersion.cmake"
DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel)

# install the export set for use with the install-tree
install(EXPORT KDEGamesLibraryDepends DESTINATION
    ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel)

feature_summary(WHAT ALL
                INCLUDE_QUIET_PACKAGES
                FATAL_ON_MISSING_REQUIRED_PACKAGES)

