enable_testing()
project(libkdegamesprivate)

KDE4_NO_ENABLE_FINAL(libkdegamesprivate)

add_subdirectory(tests)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/kgame
    ${CMAKE_CURRENT_SOURCE_DIR}/kgame/dialogs
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

########### next target ###############

set(kdegamesprivate_LIB_SRCS
    kchatbase.cpp
    kchatbaseitemdelegate.cpp
    kchatbasemodel.cpp
    kchat.cpp
    kchatdialog.cpp
    kgame/dialogs/kgameconnectdialog.cpp
    kgame/dialogs/kgamedebugdialog.cpp
    kgame/dialogs/kgamedialogconfig.cpp
    kgame/dialogs/kgamedialog.cpp
    kgame/dialogs/kgameerrordialog.cpp
    kgame/kgamechat.cpp
    kgame/kgame.cpp
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
    kgamecanvas.cpp
    kgamedifficulty.cpp
    kgamesvgdocument.cpp
    kgametheme.cpp
    kgamethemeselector.cpp
)

kde4_add_ui_files(kdegamesprivate_LIB_SRCS
    kgamethemeselector.ui
)

kde4_add_library(kdegamesprivate SHARED ${kdegamesprivate_LIB_SRCS})

target_link_libraries(kdegamesprivate ${KDE4_KDEUI_LIBS} ${KDE4_KDNSSD_LIBS} ${KDE4_KNEWSTUFF3_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTNETWORK_LIBRARY} kdegames)
target_link_libraries(kdegamesprivate LINK_INTERFACE_LIBRARIES ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTNETWORK_LIBRARY} kdegames)

set_target_properties(kdegamesprivate PROPERTIES VERSION 1.0.0 SOVERSION 1)

########### install files ###############

install(TARGETS kdegamesprivate ${INSTALL_TARGETS_DEFAULT_ARGS})

# TODO: install headers files of libkdegamesprivate (for kdegames repo split)

install(FILES
    kchatbase.h
    kchatbaseitemdelegate.h
    kchatbasemodel.h
    kchatdialog.h
    kchat.h
    kgamecanvas.h
    kgamedifficulty.h
    kgamesvgdocument.h
    kgametheme.h
    kgamethemeselector.h
DESTINATION ${INCLUDE_INSTALL_DIR}/libkdegamesprivate COMPONENT Devel)

install(FILES
    kgame/kgamechat.h
    kgame/kgameerror.h
    kgame/kgame.h
    kgame/kgameio.h
    kgame/kgamemessage.h
    kgame/kgamenetwork.h
    kgame/kgameprocess.h
    kgame/kgamepropertyarray.h
    kgame/kgameproperty.h
    kgame/kgamepropertyhandler.h
    kgame/kgamesequence.h
    kgame/kgameversion.h
    kgame/kmessageclient.h
    kgame/kmessageio.h
    kgame/kmessageserver.h
    kgame/kplayer.h
DESTINATION ${INCLUDE_INSTALL_DIR}/libkdegamesprivate/kgame COMPONENT Devel)

install(FILES
    kgame/dialogs/kgameconnectdialog.h
    kgame/dialogs/kgamedebugdialog.h
    kgame/dialogs/kgamedialogconfig.h
    kgame/dialogs/kgamedialog.h
    kgame/dialogs/kgameerrordialog.h
DESTINATION ${INCLUDE_INSTALL_DIR}/libkdegamesprivate/kgame/dialogs COMPONENT Devel)
