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
if(NOT WIN32)
  add_subdirectory( kggzmod )
endif(NOT WIN32)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/highscore
   ${CMAKE_CURRENT_SOURCE_DIR}/kgame
   ${CMAKE_CURRENT_SOURCE_DIR}/kgame/dialogs
   ${CMAKE_CURRENT_BINARY_DIR}/highscore
)

########### next target ###############

set(libkhighscore_SRCS  
#   highscore/kconfigrawbackend.cpp 
    highscore/khighscore.cpp 
    highscore/kscoredialog.cpp
    highscore/kexthighscore_item.cpp 
    highscore/kexthighscore_internal.cpp 
    highscore/kexthighscore_tab.cpp 
    highscore/kexthighscore_gui.cpp
    highscore/kexthighscore.cpp
)

set(libkgamedigits_SRCS 
    digits/kgamesvgdigits.cpp 
    )

set(libkgame_SRCS 
    kgame/kgame.cpp 
    kgame/kplayer.cpp 
    kgame/kgamenetwork.cpp 
    kgame/kgameproperty.cpp
    kgame/kgamemessage.cpp 
    kgame/kgameio.cpp 
    kgame/kgameprocess.cpp 
    kgame/kgamechat.cpp
    kgame/kgamepropertyhandler.cpp 
    kgame/kgameerror.cpp 
    kgame/kgamesequence.cpp
    kgame/kmessageio.cpp 
    kgame/kmessageserver.cpp 
    kgame/kmessageclient.cpp
    )

set(libkgamedialog_SRCS 
    kgame/dialogs/kgamedialog.cpp 
    kgame/dialogs/kgameconnectdialog.cpp 
    kgame/dialogs/kgameerrordialog.cpp 
    kgame/dialogs/kgamedebugdialog.cpp 
    kgame/dialogs/kgamedialogconfig.cpp
    )

set(kdegames_LIB_SRCS ${libkhighscore_SRCS} ${libkgame_SRCS} ${libkgamedialog_SRCS} ${libkgamedigits_SRCS}
    kgamecanvas.cpp 
    kcarddialog.cpp 
    kstandardgameaction.cpp 
    kzoommainwindow.cpp
    kgamemisc.cpp 
    kchatbase.cpp 
    kchatbasemodel.cpp 
    kchatbaseitemdelegate.cpp 
    kchat.cpp 
    kchatdialog.cpp 
    kgameprogress.cpp 
    kgamelcd.cpp
    kgamesvgdocument.cpp 
    kgameclock.cpp
   )

kde4_automoc(${kdegames_LIB_SRCS})

kde4_add_library(kdegames SHARED ${kdegames_LIB_SRCS})

target_link_libraries(kdegames  ${KDE4_KIO_LIBS} ${KDE4_KDNSSD_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTNETWORK_LIBRARY})

set_target_properties(kdegames PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kdegames  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install(FILES
    libkdegames_export.h
    kgamemisc.h
    kcarddialog.h
    kstandardgameaction.h
    kzoommainwindow.h
    kchatbase.h
    kchatbasemodel.h
    kchatbaseitemdelegate.h
    kchat.h
    kchatdialog.h
    kgameprogress.h
    kgamelcd.h
    kgamesvgdocument.h
    kgrid2d.h
    kgamecanvas.h
    kgameclock.h
    DESTINATION ${INCLUDE_INSTALL_DIR})

