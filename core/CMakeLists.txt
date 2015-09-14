cmake_minimum_required(VERSION 3.0.2)

project(SilkEdit_core)

set(CMAKE_AUTOMOC ON)

# Automatically add the current source- and build directories to the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE SILK_SOURCES *.cpp)
file(GLOB_RECURSE SILK_HEADERS *.h)

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5LinguistTools REQUIRED)

if (APPLE)
  find_library(YAML_CPP_LIBRARY NAMES libyaml-cpp.a PATHS /usr/local/lib)
  find_library(ONIG_LIBRARY NAMES libonig.a PATHS /usr/local/lib)
  find_library(UCHARDET_LIBRARY NAMES libuchardet.a PATHS /usr/local/lib)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wall -pedantic -Wextra -Wnon-virtual-dtor -Woverloaded-virtual")
  include_directories(SYSTEM /usr/local/include msgpack-c/include src)
  add_library(${PROJECT_NAME} ${SILK_SOURCES} ${SILK_HEADERS})
elseif (MINGW)
  set(YAML_CPP_LIBRARY yaml-cpp)
  set(ONIG_LIBRARY onig.a)
  set(UCHARDET_LIBRARY uchardet.a)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -Wextra -Wnon-virtual-dtor -Woverloaded-virtual")
  if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( MSYS_INCLUDE c:/msys64/mingw64/include c:/msys64/usr/local/include)
    set( MSYS_LIB c:/msys64/mingw64/lib c:/msys64/usr/local/lib)
  else ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( MSYS_INCLUDE c:/msys32/mingw32/include c:/msys32/usr/local/include)
    set( MSYS_LIB c:/msys32/mingw32/lib c:/msys32/usr/local/lib)
  endif ( CMAKE_SIZEOF_VOID_P EQUAL 8 )

  # SYSTEM tells the compiler to skip warning for these header files.
  include_directories(SYSTEM msgpack-c/include src ${MSYS_INCLUDE} include)
  link_directories(${MSYS_LIB} lib)

  add_library(${PROJECT_NAME} ${SILK_SOURCES} ${SILK_HEADERS})
endif (APPLE)

target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Network Qt5::Xml ${YAML_CPP_LIBRARY} ${ONIG_LIBRARY} ${UCHARDET_LIBRARY})
