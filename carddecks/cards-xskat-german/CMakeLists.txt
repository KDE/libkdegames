
file( RELATIVE_PATH current_dir ${carddecks_base_dir} ${CMAKE_CURRENT_SOURCE_DIR} )
FILE(GLOB _pngfile "*.png" )
FILE(GLOB _desktopfile "*.desktop" )

install( FILES ${_pngfile}  DESTINATION  ${DATA_INSTALL_DIR}/carddecks/${current_dir} )
install( FILES ${_desktopfile}  DESTINATION  ${DATA_INSTALL_DIR}/carddecks/${current_dir} )
install( FILES COPYRIGHT  DESTINATION  ${DATA_INSTALL_DIR}/carddecks/${current_dir} )

