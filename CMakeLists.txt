project(libkdegames)

add_subdirectory( carddecks ) 
add_subdirectory( highscore ) 
add_subdirectory( kgame ) 
add_subdirectory( pics ) 

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/highscore/ ${CMAKE_CURRENT_SOURCE_DIR}/kgame/ ${CMAKE_CURRENT_SOURCE_DIR}/kgame/dialogs/ )


########### next target ###############

set(libkhighscore_SRCS  
	highscore/kconfigrawbackend.cpp 
	highscore/kfilelock.cpp 
	highscore/khighscore.cpp 
	highscore/kscoredialog.cpp
	highscore/kexthighscore_item.cpp 
	highscore/kexthighscore_internal.cpp 
	highscore/kexthighscore_tab.cpp 
	highscore/kexthighscore_gui.cpp
	highscore/kexthighscore.cpp
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

set(kdegames_LIB_SRCS ${libkhighscore_SRCS} ${libkgame_SRCS} ${libkgamedialog_SRCS}
   kgamecanvas.cpp 
   kcarddialog.cpp 
   kstdgameaction.cpp 
   kzoommainwindow.cpp
   kgamemisc.cpp 
   kchatbase.cpp 
   kchat.cpp 
   kchatdialog.cpp 
   kgameprogress.cpp 
   kgamelcd.cpp )

kde4_automoc(kdegames ${kdegames_LIB_SRCS})

kde4_add_library(kdegames SHARED ${kdegames_LIB_SRCS})

target_link_libraries(kdegames  ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KDNSSD_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTNETWORK_LIBRARY})

set_target_properties(kdegames PROPERTIES VERSION 1.2.0 SOVERSION 1 )
install(TARGETS kdegames  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install(FILES
   libkdegames_export.h
   kgamemisc.h
   kcarddialog.h
   kstdgameaction.h
   kzoommainwindow.h
   kchatbase.h
   kchat.h
   kchatdialog.h
   kgameprogress.h
   kgamelcd.h
   kgrid2d.h
   kgamecanvas.h
DESTINATION ${INCLUDE_INSTALL_DIR})

