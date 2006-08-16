



########### install files ###############

install( FILES star.png  DESTINATION  ${DATA_INSTALL_DIR}/kdegames/pics )

#kde4_install_icons( ${ICON_INSTALL_DIR}   )
kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )




#original Makefile.am contents follow:

#picsdir = $(kde_datadir)/kdegames/pics
#pics_DATA = star.png
#
#KDE_ICON = action-roll action-highscore action-endturn
