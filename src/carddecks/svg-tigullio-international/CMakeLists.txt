
file( RELATIVE_PATH current_dir ${carddecks_base_dir} ${CMAKE_CURRENT_SOURCE_DIR} )

FILE(GLOB _pngfile "*.png" )
FILE(GLOB _desktopfile "*.desktop" )
install( FILES ${_pngfile} tigullio-international.svgz DESTINATION  ${KDE_INSTALL_DATADIR}/carddecks/${current_dir} )
install( FILES ${_desktopfile}  DESTINATION  ${KDE_INSTALL_DATADIR}/carddecks/${current_dir} )
