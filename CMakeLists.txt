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

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Test REQUIRED)

# Find oniguruma using pkg-config
INCLUDE(FindPkgConfig)
pkg_check_modules(Oniguruma REQUIRED oniguruma)

IF (APPLE)
  # icon files to copy in the bundle
  set( OSX_ICON_FILES resources/silkEdit.icns )
  # set where in the bundle to put the icns files
  set_source_files_properties( ${OSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  # include the icns files in the target
  set( SILK_SOURCES ${SILK_SOURCES} ${OSX_ICON_FILES} )

  # bundle default packages
  file( GLOB_RECURSE PACKAGES packages/* )
  set_source_files_properties( ${PACKAGES} PROPERTIES MACOSX_PACKAGE_LOCATION MacOS/Packages )
  set( SILK_SOURCES ${SILK_SOURCES} ${PACKAGES} )

  # bundle plugin server (node.js)
  file( COPY lib/node-0.10.36.bin DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/plugin_server FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
  file( RENAME plugin_server/node-0.10.36.bin plugin_server/node )
  file( GLOB_RECURSE PLUGIN_SERVER plugin_server/* )
  set_source_files_properties( ${PLUGIN_SERVER} PROPERTIES MACOSX_PACKAGE_LOCATION MacOS/plugin_server )
  set( SILK_SOURCES ${SILK_SOURCES} ${PLUGIN_SERVER} )

ENDIF (APPLE)

qt5_add_resources(UI_RESOURCES ${SILK_RESOURCES})

include_directories(include src ${Oniguruma_INCLUDE_DIRS})

link_directories(lib ${Oniguruma_LIBRARY_DIRS})

add_executable(SilkEdit MACOSX_BUNDLE ${SILK_SOURCES} ${SILK_HEADERS} ${UI_RESOURCES})

target_link_libraries(SilkEdit Qt5::Widgets Qt5::Network Qt5::Xml yaml-cpp-0.5.1 ${Oniguruma_LIBRARIES} )

if(APPLE)
  set_target_properties( SilkEdit PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/src/SilkEdit-info.plist )
ENDIF(APPLE)
