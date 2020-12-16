cmake_minimum_required (VERSION 3.5 FATAL_ERROR)

set(KDEGAMES_VERSION 7.3.0)
set(KDEGAMES_SOVERSION 7)

project(libkdegames)

set (QT_MIN_VERSION "5.12.0")
set (KF5_MIN_VERSION "5.68.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets
    Qml
    Quick
    QuickWidgets
    Svg
    Test
)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
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

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateExportHeader)
include(ECMQtDeclareLoggingCategory)

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

add_definitions(-DMAKE_KDEGAMESPRIVATE_LIB)
add_definitions(-DTRANSLATION_DOMAIN="libkdegames5")

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050900
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DQT_NO_FOREACH
    -DQT_NO_KEYWORDS
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_STRICT_ITERATORS
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x052e00
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
)

add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
