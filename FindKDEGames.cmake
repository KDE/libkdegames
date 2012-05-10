# - Try to find the kdegames library
# Once done this will define
#
#  KDEGAMES_FOUND - system has the kdegames library
#  KDEGAMES_INCLUDE_DIRS - a list of the relevant libkdegames include dirs. Allows the use of forwarding header, e.g. #include <KgDifficulty>
#  KDEGAMES_INCLUDE_DIR - the kdegames include directory
#  KDEGAMES_LIBRARY - Link this to use the kdegames library
#

get_filename_component(KDEGAMES_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include("${KDEGAMES_CMAKE_DIR}/KDEGamesConfig.cmake")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(KDEGAMES DEFAULT_MSG KDEGAMES_INCLUDE_DIR KDEGAMES_LIBRARY)

mark_as_advanced(KDEGAMES_INCLUDE_DIR KDEGAMES_INCLUDE_DIRS KDEGAMES_LIBRARY)
