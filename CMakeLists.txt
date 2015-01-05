cmake_minimum_required(VERSION 2.8.11)

project(silkedit)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Automatically add the current source- and build directories to the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wall -pedantic -Wextra -Wnon-virtual-dtor -Woverloaded-virtual")

file(GLOB_RECURSE SILK_SOURCES src/*.cpp)
file(GLOB_RECURSE SILK_HEADERS src/*.h)
file(GLOB_RECURSE SILK_RESOURCES resources/*.qrc)

configure_file("src/keymap.yml" "keymap.yml" COPYONLY)
configure_file("src/config.yml" "config.yml" COPYONLY)

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Test REQUIRED)

# Find oniguruma using pkg-config
INCLUDE(FindPkgConfig)
pkg_check_modules(Oniguruma REQUIRED oniguruma)

IF (APPLE)
  find_library(CoreFoundationLibrary CoreFoundation)

  # icon files to copy in the bundle
  set( OSX_ICON_FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/silkEdit.icns )
  # set where in the bundle to put the icns files
  set_source_files_properties( ${OSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  # include the icns files in the target
  set( SILK_SOURCES ${SILK_SOURCES} ${OSX_ICON_FILES} )

  # bundle default packages
  file( GLOB_RECURSE PACKAGES ${CMAKE_CURRENT_SOURCE_DIR}/packages/* )
  set_source_files_properties( ${PACKAGES} PROPERTIES MACOSX_PACKAGE_LOCATION MacOS/packages )
  set( SILK_SOURCES ${SILK_SOURCES} ${PACKAGES} )
ENDIF (APPLE)

qt5_add_resources(UI_RESOURCES ${SILK_RESOURCES})

include_directories(include src stextedit/mysrc ${Oniguruma_INCLUDE_DIRS})

link_directories(lib stextedit/build ${Oniguruma_LIBRARY_DIRS})

add_executable(SilkEdit MACOSX_BUNDLE ${SILK_SOURCES} ${SILK_HEADERS} ${UI_RESOURCES})

target_link_libraries(SilkEdit Qt5::Widgets Qt5::Xml yaml-cpp-0.5.1 stextedit ${Oniguruma_LIBRARIES} ${CoreFoundationLibrary})

if(APPLE)
  set_target_properties( SilkEdit PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/src/SilkEdit-info.plist )
ENDIF(APPLE)
