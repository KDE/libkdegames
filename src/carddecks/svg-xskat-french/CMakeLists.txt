
file( RELATIVE_PATH current_dir ${carddecks_base_dir} ${CMAKE_CURRENT_SOURCE_DIR} )

install( FILES index.desktop french.svgz 11.png COPYRIGHT DESTINATION ${DATA_INSTALL_DIR}/carddecks/${current_dir} )
