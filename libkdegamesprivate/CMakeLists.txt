enable_testing()
project(libkdegamesprivate)

add_subdirectory(tests)

# NOTE: The libkdegamesprivate target is compiled in the parent directory,
# because CMake can't cope with exported libraries in two different
# directories.

########### install files ###############

install(FILES
    libkdegamesprivate_export.h
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
    kgame/kgameproperty.h
    kgame/kgamepropertyhandler.h
    kgame/kgamesequence.h
    kgame/kgameversion.h
    kgame/kmessageclient.h
    kgame/kmessageio.h
    kgame/kmessageserver.h
    kgame/kplayer.h
DESTINATION ${INCLUDE_INSTALL_DIR}/libkdegamesprivate/kgame COMPONENT Devel)
