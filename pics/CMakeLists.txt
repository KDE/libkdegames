kde4_header()




########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kdegames/pics FILES star.png )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

#picsdir = $(kde_datadir)/kdegames/pics
#pics_DATA = star.png
#
#KDE_ICON = action-roll action-highscore action-endturn
