
file( RELATIVE_PATH current_dir ${carddecks_base_dir} ${CMAKE_CURRENT_SOURCE_DIR} )
install( FILES index.desktop oxygen-air.svgz 11.png DESTINATION ${DATA_INSTALL_DIR}/carddecks/${current_dir} )
