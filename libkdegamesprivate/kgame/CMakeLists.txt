
add_subdirectory( dialogs ) 



########### next target ###############


########### install files ###############

install(FILES
   kgame.h
   kplayer.h
   kgamenetwork.h
   kgameproperty.h
   kgamemessage.h
   kgameio.h
   kgameprocess.h
   kgamepropertyarray.h
   kgamechat.h
   kgamepropertyhandler.h
   kgameerror.h
   kgamesequence.h
   kgameversion.h
   kmessageio.h
   kmessageserver.h
   kmessageclient.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kgame COMPONENT Devel)




