
file( RELATIVE_PATH current_dir ${carddecks_base_dir} ${CMAKE_CURRENT_SOURCE_DIR} )
install( FILES index.desktop standard.svgz 11.png DESTINATION ${KDE_INSTALL_DATADIR}/carddecks/${current_dir} )

