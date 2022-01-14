cmake_minimum_required (VERSION 3.16 FATAL_ERROR)

set(KDEGAMES_VERSION 7.3.1)
set(KDEGAMES_SOVERSION 7)

project(libkdegames)

set (QT_MIN_VERSION "5.15.0")
set (KF_MIN_VERSION "5.89.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateExportHeader)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)
include(CMakePackageConfigHelpers)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets
    Qml
    Quick
    QuickWidgets
    Svg
    Test
)
if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(Qt6Core5Compat)
endif()

find_package(KF5 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    Archive
    Completion
    Config
    ConfigWidgets
    DNSSD
    Declarative
    GuiAddons
    IconThemes
    I18n
    NewStuff
    Service
    XmlGui
    WidgetsAddons
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

add_definitions(-DTRANSLATION_DOMAIN="libkdegames5")

add_definitions(
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050f02
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055900
    -DKCOREADDONS_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054900 # KRandomSequence, cannot be removed
    -DKNEWSTUFF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054F00 # KNS3::DownloadDialog -> needs porting
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
)

add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
