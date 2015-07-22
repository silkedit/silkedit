cmake_minimum_required(VERSION 3.0.2)

project(SilkEdit)

option (BUILD_EDGE "Build edge version" OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Automatically add the current source- and build directories to the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE SILK_SOURCES src/*.cpp)
file(GLOB_RECURSE SILK_HEADERS src/*.h)
file(GLOB_RECURSE SILK_UIS src/*.ui)

if (APPLE)
  file(GLOB_RECURSE SILK_RESOURCES resources/mac/*.qrc)
elseif (MINGW)
  file(GLOB_RECURSE SILK_RESOURCES resources/win/*.qrc)
endif (APPLE)

if (APPLE)
  set(CMAKE_PREFIX_PATH /usr/local/Qt-5.4.2)
elseif (MINGW)
  if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set(CMAKE_PREFIX_PATH "C:/Qt/Qt5.4.2/mingw64")
  else ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set(CMAKE_PREFIX_PATH "C:/Qt/Qt5.4.2/mingw32")
  endif ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
endif (APPLE)

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)

qt5_add_resources(UI_RESOURCES ${SILK_RESOURCES})

if (APPLE)
  # find yaml-cpp and onig library
  find_library(YAML_CPP_LIBRARY NAMES libyaml-cpp.a PATHS /usr/local/lib)
  find_library(ONIG_LIBRARY NAMES libonig.a PATHS /usr/local/lib)

  # icon files to copy in the bundle
  set( OSX_ICON_FILES resources/silkedit.icns )

  # set where in the bundle to put the icns files
  set_source_files_properties( ${OSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  # include the icns files in the target
  set( SILK_SOURCES ${SILK_SOURCES} ${OSX_ICON_FILES} )

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wall -pedantic -Wextra -Wnon-virtual-dtor -Woverloaded-virtual")
  include_directories(/usr/local/include msgpack-c/include src)
	add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SILK_SOURCES} ${SILK_HEADERS} ${SILK_UIS} ${UI_RESOURCES})
elseif (MINGW)
  set(YAML_CPP_LIBRARY yaml-cpp)
  set(ONIG_LIBRARY onig.a)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -Wextra -Wnon-virtual-dtor -Woverloaded-virtual")
  if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( MSYS_INCLUDE c:/msys64/mingw64/include )
    set( MSYS_LIB c:/msys64/mingw64/lib )
  else ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( MSYS_INCLUDE c:/msys32/mingw32/include )
    set( MSYS_LIB c:/msys32/mingw32/lib )
  endif ( CMAKE_SIZEOF_VOID_P EQUAL 8 )

  # SYSTEM tells the compiler to skip warning for these header files.
  include_directories(SYSTEM msgpack-c/include src ${MSYS_INCLUDE} include)
  link_directories(${MSYS_LIB} lib)

  if (BUILD_EDGE)
    set( SILK_ICON resources/silkedit_edge.rc)
  else (BUILD_EDGE)
    set( SILK_ICON resources/silkedit.rc)
  endif (BUILD_EDGE)

  add_executable(${PROJECT_NAME} WIN32 ${SILK_SOURCES} ${SILK_HEADERS} ${UI_RESOURCES} ${SILK_ICON})
endif (APPLE)

target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Network Qt5::Xml ${YAML_CPP_LIBRARY} ${ONIG_LIBRARY} )

if(APPLE)
  # bundle packages
  file(COPY ${PROJECT_SOURCE_DIR}/packages DESTINATION ${PROJECT_NAME}.app/Contents/MacOS)

  # bundle plugin runner (node.js)
  file(COPY ${PROJECT_SOURCE_DIR}/plugin_runner DESTINATION ${PROJECT_NAME}.app/Contents/MacOS)

  set_target_properties( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/src/SilkEdit-info.plist )

  # Use special icon for edge build
  if (BUILD_EDGE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp ${PROJECT_SOURCE_DIR}/resources/silkedit_edge.icns ${PROJECT_NAME}.app/Contents/Resources/silkedit.icns)
  endif (BUILD_EDGE)
elseif (MINGW)
  # Copy packages directory to the same directory of exe
  file(COPY ${PROJECT_SOURCE_DIR}/packages DESTINATION .)

  # Copy plugin_runner (node.js)
  file(COPY ${PROJECT_SOURCE_DIR}/plugin_runner DESTINATION .)
ENDIF(APPLE)
