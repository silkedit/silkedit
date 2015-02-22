cmake_minimum_required(VERSION 2.8.11)

project(silkedit)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Automatically add the current source- and build directories to the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wall -pedantic -Wextra -Wnon-virtual-dtor -Woverloaded-virtual")
  set(CMAKE_PREFIX_PATH "/usr/local/Qt-5.4.0")
elseif (MINGW)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -static -static-libgcc -static-libstdc++ -Wall -pedantic -Wextra -Wnon-virtual-dtor -Woverloaded-virtual")
  set(CMAKE_PREFIX_PATH "C:/Qt5.4/5.4/mingw491_32")
endif (APPLE)

file(GLOB_RECURSE SILK_SOURCES src/*.cpp)
file(GLOB_RECURSE SILK_HEADERS src/*.h)
file(GLOB_RECURSE SILK_RESOURCES resources/*.qrc)

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Test REQUIRED)

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
ENDIF (APPLE)

qt5_add_resources(UI_RESOURCES ${SILK_RESOURCES})

if (APPLE)
  include_directories(msgpack-c/include src)
elseif (MINGW)
  include_directories(msgpack-c/include src c:/msys32/mingw32/include c:/include)
  link_directories(c:/msys32/mingw32/lib c:/lib)
endif (APPLE)

if (APPLE)
  add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SILK_SOURCES} ${SILK_HEADERS} ${UI_RESOURCES})
elseif (MINGW)
  add_executable(${PROJECT_NAME} ${SILK_SOURCES} ${SILK_HEADERS} ${UI_RESOURCES})
endif (APPLE)

target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Network Qt5::Xml yaml-cpp onig )

if(APPLE)
  # bundle plugin runner (node.js)
  # we need to use custom_command to copy directory.
  ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp -r ${PROJECT_SOURCE_DIR}/plugin_runner ${PROJECT_NAME}.app/Contents/MacOS)
  set_target_properties( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/src/SilkEdit-info.plist )
ENDIF(APPLE)
