cmake_minimum_required(VERSION 2.8.11)

project(SilkEdit_Benchmark)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Automatically add the current source- and build directories to the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SILK_SOURCES ../src/SyntaxHighlighter.cpp ../src/LanguageParser.cpp ../src/PListParser.cpp ../src/Regexp.cpp ../src/Util.cpp ../src/Region.cpp ../src/Theme.cpp ../src/Session.cpp ../src/ThemeProvider.cpp ../src/ConfigManager.cpp ../src/Constants.cpp src/main.cpp)

if (APPLE)
  set(CMAKE_PREFIX_PATH /usr/local/Qt-5.4.0)
endif (APPLE)

file(COPY "testdata" DESTINATION .)

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)

qt5_add_resources(UI_RESOURCES ${SILK_RESOURCES})

if (APPLE)
  # include the icns files in the target
  set( SILK_SOURCES ${SILK_SOURCES} ${OSX_ICON_FILES} )

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wall -pedantic -Wextra -Wnon-virtual-dtor -Woverloaded-virtual")
  include_directories(../msgpack-c/include ../src)
  add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SILK_SOURCES})
endif (APPLE)

target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Network Qt5::Xml yaml-cpp onig.a )
