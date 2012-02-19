enable_testing()

project(libkdegames)

KDE4_NO_ENABLE_FINAL(libkdegames)
configure_file(highscore/config-highscore.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/highscore/config-highscore.h )

add_subdirectory( carddecks )
add_subdirectory( highscore )
add_subdirectory( includes )
add_subdirectory( kgame )
add_subdirectory( pics )
add_subdirectory( tests )

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/highscore
   ${CMAKE_CURRENT_SOURCE_DIR}/kgame
   ${CMAKE_CURRENT_SOURCE_DIR}/kgame/dialogs
   ${CMAKE_CURRENT_BINARY_DIR}/highscore
)

option (USE_OPENAL_SNDFILE "use OpenAL and libsndfile in libkdegames" ON)
if (SNDFILE_FOUND AND USE_OPENAL_SNDFILE)
    message(STATUS "Checking libsndfile capabilities")
    try_compile(SNDFILE_WORKS
        ${CMAKE_CURRENT_BINARY_DIR}/audio/check-libsndfile-capabilities
        ${CMAKE_CURRENT_SOURCE_DIR}/audio/check-libsndfile-capabilities.cpp)
    if (NOT SNDFILE_WORKS)
        message(FATAL_ERROR "Your version of libsndfile is too old. At least version 0.21 is needed. To skip the optional OpenAL/libsndfile dependency in libkdegames (not recommended), re-run cmake with -DUSE_OPENAL_SNDFILE=OFF.")
    endif (NOT SNDFILE_WORKS)
endif (SNDFILE_FOUND AND USE_OPENAL_SNDFILE)

if (OPENAL_FOUND AND SNDFILE_FOUND AND SNDFILE_WORKS)
    include_directories(${OPENAL_INCLUDE_DIR} ${SNDFILE_INCLUDE_DIRS})
    set(KGAUDIO_LINKLIBS ${OPENAL_LIBRARY} ${SNDFILE_LIBRARIES})
    set(KGAUDIO_BACKEND openal)
else (OPENAL_FOUND AND SNDFILE_FOUND AND SNDFILE_WORKS)
    set(KGAUDIO_LINKLIBS ${KDE4_PHONON_LIBS})
    set(KGAUDIO_BACKEND phonon)
endif (OPENAL_FOUND AND SNDFILE_FOUND AND SNDFILE_WORKS)

########### next target ###############

set(kdegames_LIB_SRCS
    audio/kgaudioscene-${KGAUDIO_BACKEND}.cpp
    audio/kgsound-${KGAUDIO_BACKEND}.cpp
    colorproxy_p.cpp
#   highscore/kconfigrawbackend.cpp
    highscore/kexthighscore_item.cpp
    highscore/kexthighscore_internal.cpp
    highscore/kexthighscore_tab.cpp
    highscore/kexthighscore_gui.cpp
    highscore/kexthighscore.cpp
    highscore/khighscore.cpp
    highscore/kscoredialog.cpp
    kgame/kgame.cpp
    kgame/kgamechat.cpp
    kgame/kgameerror.cpp
    kgame/kgameio.cpp
    kgame/kgamemessage.cpp
    kgame/kgamenetwork.cpp
    kgame/kgameprocess.cpp
    kgame/kgameproperty.cpp
    kgame/kgamepropertyhandler.cpp
    kgame/kgamesequence.cpp
    kgame/kmessageclient.cpp
    kgame/kmessageio.cpp
    kgame/kmessageserver.cpp
    kgame/kplayer.cpp
    kgame/dialogs/kgameconnectdialog.cpp
    kgame/dialogs/kgamedebugdialog.cpp
    kgame/dialogs/kgamedialog.cpp
    kgame/dialogs/kgamedialogconfig.cpp
    kgame/dialogs/kgameerrordialog.cpp
    kchat.cpp
    kchatbase.cpp
    kchatbaseitemdelegate.cpp
    kchatbasemodel.cpp
    kchatdialog.cpp
    kgamecanvas.cpp
    kgameclock.cpp
    kgamepopupitem.cpp
    kgamerendereditem.cpp
    kgamerenderedobjectitem.cpp
    kgamerenderer.cpp
    kgamerendererclient.cpp
    kgamesvgdocument.cpp
    kgametheme.cpp
    kgamethemeselector.cpp
    kstandardgameaction.cpp
    kgamedifficulty.cpp
)
if(Q_WS_X11)
	set(kdegames_LIB_SRCS ${kdegames_LIB_SRCS} chooserastergraphicssystem.cpp)
endif(Q_WS_X11)

kde4_add_ui_files(kdegames_LIB_SRCS
    kgamethemeselector.ui
)

kde4_add_library(kdegames SHARED ${kdegames_LIB_SRCS})

target_link_libraries(kdegames ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KDNSSD_LIBS} ${KDE4_KNEWSTUFF3_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${KGAUDIO_LINKLIBS})
target_link_libraries(kdegames LINK_INTERFACE_LIBRARIES ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTNETWORK_LIBRARY} )

#NOTE: 6.0.0 = KDE 4.9 release, please increase minor version in next minor KDE SC version if API changes
set_target_properties(kdegames PROPERTIES VERSION 6.0.0
                                          SOVERSION 6 )

install(TARGETS kdegames ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
    libkdegames_export.h
    kstandardgameaction.h
    kchatbase.h
    kchatbasemodel.h
    kchatbaseitemdelegate.h
    kchat.h
    kchatdialog.h
    kgamesvgdocument.h
    kgamepopupitem.h
    kgamecanvas.h
    kgameclock.h
    kgamerendereditem.h
    kgamerenderedobjectitem.h
    kgamerenderer.h
    kgamerendererclient.h
    kgametheme.h
    kgamethemeselector.h
    kgamedifficulty.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
