enable_testing()

project(libkdegames)

KDE4_NO_ENABLE_FINAL(libkdegames)
KDE4_NO_ENABLE_FINAL(libkdegamesprivate)

configure_file(highscore/config-highscore.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/highscore/config-highscore.h )

add_subdirectory( carddecks )
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
    ${CMAKE_CURRENT_SOURCE_DIR}/libkdegamesprivate/kgame/dialogs
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
    kgdifficulty.cpp
    kgtheme.cpp
    kgthemeprovider.cpp
    kgthemeselector.cpp
    kstandardgameaction.cpp
)
if(Q_WS_X11)
	set(kdegames_LIB_SRCS ${kdegames_LIB_SRCS} chooserastergraphicssystem.cpp)
endif(Q_WS_X11)

kde4_add_library(kdegames SHARED ${kdegames_LIB_SRCS})

target_link_libraries(kdegames ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KNEWSTUFF3_LIBS} ${QT_QTXML_LIBRARY} ${KGAUDIO_LINKLIBS})
target_link_libraries(kdegames LINK_INTERFACE_LIBRARIES ${KDE4_KDEUI_LIBS})

#NOTE: 6.0.0 = KDE 4.9 release
set(KDEGAMES_VERSION 6.0.0)
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
    libkdegamesprivate/kchat.cpp
    libkdegamesprivate/kchatdialog.cpp
    libkdegamesprivate/kgame/dialogs/kgameconnectdialog.cpp
    libkdegamesprivate/kgame/dialogs/kgamedebugdialog.cpp
    libkdegamesprivate/kgame/dialogs/kgamedialogconfig.cpp
    libkdegamesprivate/kgame/dialogs/kgamedialog.cpp
    libkdegamesprivate/kgame/dialogs/kgameerrordialog.cpp
    libkdegamesprivate/kgame/kgamechat.cpp
    libkdegamesprivate/kgame/kgame.cpp
    libkdegamesprivate/kgame/kgameerror.cpp
    libkdegamesprivate/kgame/kgameio.cpp
    libkdegamesprivate/kgame/kgamemessage.cpp
    libkdegamesprivate/kgame/kgamenetwork.cpp
    libkdegamesprivate/kgame/kgameprocess.cpp
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

kde4_add_ui_files(kdegamesprivate_LIB_SRCS
    libkdegamesprivate/kgamethemeselector.ui
)

kde4_add_library(kdegamesprivate SHARED ${kdegamesprivate_LIB_SRCS})

target_link_libraries(kdegamesprivate ${KDE4_KDEUI_LIBS} ${KDE4_KDNSSD_LIBS} ${KDE4_KNEWSTUFF3_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTNETWORK_LIBRARY} kdegames)
target_link_libraries(kdegamesprivate LINK_INTERFACE_LIBRARIES ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTNETWORK_LIBRARY} kdegames)

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

# create the Config.cmake and ConfigVersion.cmake files
file(RELATIVE_PATH CONF_REL_INCLUDE_DIR
    "${CMAKE_INSTALL_PREFIX}/${DATA_INSTALL_DIR}/cmake/modules"
    "${CMAKE_INSTALL_PREFIX}/${INCLUDE_INSTALL_DIR}")
configure_file(KDEGamesConfig.cmake.in
    "${PROJECT_BINARY_DIR}/KDEGamesConfig.cmake" @ONLY)
configure_file(KDEGamesConfigVersion.cmake.in
    "${PROJECT_BINARY_DIR}/KDEGamesConfigVersion.cmake" @ONLY)

install(FILES
    FindKDEGames.cmake
    "${PROJECT_BINARY_DIR}/KDEGamesConfig.cmake"
    "${PROJECT_BINARY_DIR}/KDEGamesConfigVersion.cmake"
DESTINATION "${DATA_INSTALL_DIR}/cmake/modules" COMPONENT Devel)

# install the export set for use with the install-tree
install(EXPORT KDEGamesLibraryDepends DESTINATION
    "${DATA_INSTALL_DIR}/cmake/modules" COMPONENT Devel)
