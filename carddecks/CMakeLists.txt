
# will be used in subdirs' CMakeLists.txt
set( carddecks_base_dir ${CMAKE_CURRENT_SOURCE_DIR} )

# when adding new deck subdirectory it should be sufficient to just
# copy CMakeLists.txt from any existing deckdir to the new one.
add_subdirectory( cards-penguins )
add_subdirectory( cards-xskat-french )
add_subdirectory( cards-default)
add_subdirectory( decks )
add_subdirectory( cards-konqi-modern )
add_subdirectory( cards-xskat-german )
add_subdirectory( svg-oxygen )
add_subdirectory( svg-oxygen-white )
add_subdirectory( svg-jolly-royal )
add_subdirectory( svg-nicu-ornamental )
add_subdirectory( svg-dondorf )
add_subdirectory( svg-nicu-white )
add_subdirectory( svg-gm-paris )
add_subdirectory( svg-tigullio-international )

########### install files ###############

