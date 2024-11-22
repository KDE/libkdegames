cmake_minimum_required (VERSION 3.16 FATAL_ERROR)

# Release Service version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "24")
set(RELEASE_SERVICE_VERSION_MINOR "11")
set(RELEASE_SERVICE_VERSION_MICRO "80")

# generate patch level from release service version
set(_micro ${RELEASE_SERVICE_VERSION_MICRO})
if (_micro LESS "10")
    string(PREPEND _micro "0") # pad with 0
endif()
set(RELEASE_SERVICE_BASED_PATCHLEVEL "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${_micro}")

set(LIBKDEGAMES_VERSION_LIB "6.0")


project(libkdegames VERSION "${LIBKDEGAMES_VERSION_LIB}.${RELEASE_SERVICE_BASED_PATCHLEVEL}")
if (APPLE)
    # until now we didn't build on macos in 6.x
    # => change version for macos will not create problem
    # if we want to add new feature we will increase LIBKDEGAMES_VERSION_LIB

    # Code stoled to kdevelop
    # The math expression below automatically increments libkdegame minor version in each KDE Gear release.
    # The multiplier 3 is the number of releases that share ${RELEASE_SERVICE_VERSION_MAJOR}.
    # The fraction assigns consecutive minor versions to the 3 releases - 04, 08 and 12. The rounding up groups
    # pre-release versions with the corresponding final release versions: 03 with 04, 07 with 08 and 11 with 12.
    # The final subtrahend at the end of the expression ensures version continuity: libkdegames 6.0 in KDE Gear 24.08.
    math(EXPR LIBKDEGAME_OWN_MINOR_VERSION "${RELEASE_SERVICE_VERSION_MAJOR} * 3                  \
                                          + (${RELEASE_SERVICE_VERSION_MINOR} + 4 - 3) / 4      \
                                          - 74")
    ###################################################################################
    # Kludge for bug #448152: ld: malformed 64-bit a.b.c.d.e version number: 5.7.211201
    # Apple's linker uses the project version (not the SOVERSION) to calculate an
    # unsigned int in such a way that places restrictions on the magnitude of each tier
    # of the version string.
    # See here: https://www.sicpers.info/2013/03/how-to-version-a-mach-o-library/
    ###################################################################################
    project(libkdegames VERSION "${LIBKDEGAMES_VERSION_LIB}.${LIBKDEGAME_OWN_MINOR_VERSION}${_micro}")
endif()


set(QT_MIN_VERSION "6.5.0")
set(KF_MIN_VERSION "6.0.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateExportHeader)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMQmlModule)
include(FeatureSummary)
include(CMakePackageConfigHelpers)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Widgets
    Qml
    Quick
    Svg
    Test
)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    Archive
    ColorScheme
    Completion
    Config
    ConfigWidgets
    DNSSD
    GuiAddons
    IconThemes
    I18n
    NewStuff
    XmlGui
)

include(InternalMacros)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

set(HIGHSCORE_DIRECTORY "" CACHE STRING "Where to install system-wide highscores e.g. /var/games")

find_package(OpenAL)
set_package_properties(OpenAL PROPERTIES
    URL "https://www.openal.org/"
    TYPE REQUIRED
)

find_package(SndFile)
set_package_properties(SndFile PROPERTIES
    URL "http://www.mega-nerd.com/libsndfile/"
    TYPE REQUIRED
)

message(STATUS "Checking libsndfile capabilities")
try_compile(SNDFILE_WORKS
    ${CMAKE_CURRENT_BINARY_DIR}/src/audio/check-libsndfile-capabilities
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio/check-libsndfile-capabilities.cpp
    CMAKE_FLAGS -DINCLUDE_DIRECTORIES=${SNDFILE_INCLUDE_DIR})
if (NOT SNDFILE_WORKS)
    message(FATAL_ERROR "Your version of libsndfile (found in " ${SNDFILE_LIBRARIES} ") is too old. At least version 0.21 is needed.")
endif (NOT SNDFILE_WORKS)

message (STATUS "INCLUDES FOR SOUND:  " ${OPENAL_INCLUDE_DIR} " " ${SNDFILE_INCLUDE_DIR})
message (STATUS "LIBRARIES FOR SOUND: " ${OPENAL_LIBRARY} " " ${SNDFILE_LIBRARIES})
set(KGAUDIO_LINKLIBS ${OPENAL_LIBRARY} ${SNDFILE_LIBRARIES})
set(KGAUDIO_BACKEND openal)
set(KGAUDIO_BACKEND_OPENAL TRUE) # for configure_file() below

add_definitions(-DTRANSLATION_DOMAIN="libkdegames6")

ecm_set_disabled_deprecation_versions(
    QT 6.8.0
    KF 6.7
)

add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

ki18n_install(po)

file(GLOB_RECURSE CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
