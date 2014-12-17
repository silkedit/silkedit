cmake_minimum_required(VERSION 2.8.11)

project(silkedit)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Automatically add the current source- and build directories to the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -Wextra -Wnon-virtual-dtor -Woverloaded-virtual")

file(GLOB_RECURSE SILK_SOURCES src/*.cpp)
file(GLOB_RECURSE SILK_HEADERS src/*.h)

configure_file("src/keymap.yml" "keymap.yml" COPYONLY)
configure_file("src/config.yml" "config.yml" COPYONLY)
file(COPY "packages" DESTINATION .)

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Test REQUIRED)

# Find oniguruma using pkg-config
INCLUDE(FindPkgConfig)
pkg_check_modules(Oniguruma REQUIRED oniguruma)

IF (APPLE)
  find_library(CoreFoundationLibrary CoreFoundation)
ENDIF (APPLE)

include_directories(include src stextedit/mysrc ${Oniguruma_INCLUDE_DIRS})

link_directories(lib stextedit/build ${Oniguruma_LIBRARY_DIRS})

add_executable(SilkEdit MACOSX_BUNDLE ${SILK_SOURCES} ${SILK_HEADERS})

target_link_libraries(SilkEdit Qt5::Widgets Qt5::Xml ruby-2.1.3 yaml-cpp-0.5.1 stextedit ${Oniguruma_LIBRARIES} ${CoreFoundationLibrary})

ENABLE_TESTING()
add_subdirectory(test)
