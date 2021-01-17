
file( RELATIVE_PATH current_dir ${carddecks_base_dir} ${CMAKE_CURRENT_SOURCE_DIR} )
install( FILES 11.png index.desktop future.svgz DESTINATION ${KDE_INSTALL_DATADIR}/carddecks/${current_dir} )

