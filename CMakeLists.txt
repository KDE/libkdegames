cmake_minimum_required (VERSION 3.16 FATAL_ERROR)

# Release Service version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "24")
set(RELEASE_SERVICE_VERSION_MINOR "11")
set(RELEASE_SERVICE_VERSION_MICRO "70")

# generate patch level from release service version
set(_micro ${RELEASE_SERVICE_VERSION_MICRO})
if (_micro LESS "10")
    string(PREPEND _micro "0") # pad with 0
endif()
set(RELEASE_SERVICE_BASED_PATCHLEVEL "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${_micro}")

project(libkdegames VERSION "6.0.${RELEASE_SERVICE_BASED_PATCHLEVEL}")

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
    QT 6.7
    KF 6.4
)

add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

ki18n_install(po)

file(GLOB_RECURSE CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
