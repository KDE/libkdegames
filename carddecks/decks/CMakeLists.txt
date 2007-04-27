
file( RELATIVE_PATH current_dir ${carddecks_base_dir} ${CMAKE_CURRENT_SOURCE_DIR} )
FILE(GLOB _pngfile "*.png" )
FILE(GLOB _desktopfile "*.desktop" )
FILE(GLOB _svgfile "*.svg" )
FILE(GLOB _svgzfile "*.svgz" )

install( FILES ${_pngfile}  DESTINATION  ${DATA_INSTALL_DIR}/carddecks/${current_dir} )
install( FILES ${_desktopfile}  DESTINATION  ${DATA_INSTALL_DIR}/carddecks/${current_dir} )
install( FILES ${_svgfile}  DESTINATION  ${DATA_INSTALL_DIR}/carddecks/${current_dir} )
install( FILES ${_svgzfile}  DESTINATION  ${DATA_INSTALL_DIR}/carddecks/${current_dir} )

