cmake_minimum_required(VERSION 2.8.11)

project(silkedit_test)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Automatically add the current source- and build directories to the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -Wextra -Wnon-virtual-dtor -Woverloaded-virtual")

file(GLOB_RECURSE SILK_SOURCES ../src/*.cpp)
file(GLOB_RECURSE SILK_HEADERS ../src/*.h)
file(GLOB_RECURSE TEST_SOURCES src/*.cpp)
file(GLOB_RECURSE TEST_HEADERS src/*.h)

# exclude main.cpp from SILK_SOURCES
list(REMOVE_ITEM SILK_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../src/main.cpp)

file(COPY "packages" DESTINATION .)
file(COPY "testdata" DESTINATION .)

# Find the QtWidgets library
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Test REQUIRED)

IF (APPLE)
  find_library(CoreFoundationLibrary CoreFoundation)
ENDIF (APPLE)

include_directories(../include ../src ../stextedit/mysrc src)

link_directories(../lib ../stextedit/build)

add_executable(SilkEditTest ${SILK_SOURCES} ${SILK_HEADERS} ${TEST_HEADERS} ${TEST_SOURCES})

target_link_libraries(SilkEditTest Qt5::Widgets Qt5::Xml Qt5::Test ruby-2.1.3 yaml-cpp-0.5.1 stextedit ${CoreFoundationLibrary})
