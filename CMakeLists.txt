enable_testing()

project(libkdegames)

configure_file(highscore/config-highscore.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/highscore/config-highscore.h )

add_subdirectory( carddecks )
add_subdirectory( digits )
add_subdirectory( highscore )
add_subdirectory( includes )
add_subdirectory( kgame )
add_subdirectory( pics )
add_subdirectory( tests )
add_subdirectory( kggznet )
add_subdirectory( kggzgames )
add_subdirectory( kggzmod )

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/highscore
   ${CMAKE_CURRENT_SOURCE_DIR}/kgame
   ${CMAKE_CURRENT_SOURCE_DIR}/kgame/dialogs
   ${CMAKE_CURRENT_BINARY_DIR}/highscore
)

########### next target ###############

set(kdegames_LIB_SRCS
    digits/kgamesvgdigits.cpp
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
    carddeckinfo.cpp
    kcarddialog.cpp
    kchat.cpp
    kchatbase.cpp
    kchatbaseitemdelegate.cpp
    kchatbasemodel.cpp
    kchatdialog.cpp
    kgamecanvas.cpp
    kgameclock.cpp
    kgamelcd.cpp
    kgamemisc.cpp
    kgameprogress.cpp
    kgamepopupitem.cpp
    kgamesvgdocument.cpp
    kgametheme.cpp
    kgamethemeselector.cpp
    kstandardgameaction.cpp
    kgamedifficulty.cpp
    cardcache.cpp
)

kde4_add_ui_files(kdegames_LIB_SRCS
    kgamethemeselector.ui
    kgamecardselector.ui
)

kde4_add_library(kdegames SHARED ${kdegames_LIB_SRCS})

target_link_libraries(kdegames ${KDE4_KDNSSD_LIBS} ${KDE4_KNEWSTUFF2_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTNETWORK_LIBRARY} )

set_target_properties(kdegames PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kdegames ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
    libkdegames_export.h
    cardcache.h
    kgamemisc.h
    kcarddialog.h
    kstandardgameaction.h
    kchatbase.h
    kchatbasemodel.h
    kchatbaseitemdelegate.h
    kchat.h
    kchatdialog.h
    kgameprogress.h
    kgamelcd.h
    kgamesvgdocument.h
    kgamepopupitem.h
    kgrid2d.h
    kgamecanvas.h
    kgameclock.h
    kgametheme.h
    kgamethemeselector.h
    kgamedifficulty.h
    DESTINATION ${INCLUDE_INSTALL_DIR})

