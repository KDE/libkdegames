kde4_header()

file( RELATIVE_PATH current_dir ${carddecks_base_dir} ${CMAKE_CURRENT_SOURCE_DIR} )

install_files( ${DATA_INSTALL_DIR}/carddecks/${current_dir} .*\\.png )
install_files( ${DATA_INSTALL_DIR}/carddecks/${current_dir} .*\\.desktop )
install_files( ${DATA_INSTALL_DIR}/carddecks/${current_dir} COPYRIGHT )

kde4_footer()
