
file( RELATIVE_PATH current_dir ${carddecks_base_dir} ${CMAKE_CURRENT_SOURCE_DIR} )
install( FILES 11.png index.desktop paris.svgz DESTINATION ${DATA_INSTALL_DIR}/carddecks/${current_dir} )

