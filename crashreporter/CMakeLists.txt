#
# CrashReporter cmake file
#

cmake_minimum_required(VERSION 3.0.2)

project(CrashReporter)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Automatically add the current source- and build directories to the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE CRASHREPORTER_SOURCES src/*.cpp src/*.c)
file(GLOB_RECURSE CRASHREPORTER_HEADERS src/*.h)
file(GLOB_RECURSE CRASHREPORTER_UIS src/*.ui)
file(GLOB CRASHREPORTER_TRANSLATIONS trans/*.ts)

if (APPLE)
  file(GLOB_RECURSE CRASHREPORTER_RESOURCES res/mac/*.qrc)
else (APPLE)
  file(GLOB_RECURSE CRASHREPORTER_RESOURCES res/win/*.qrc)
endif (APPLE)


if (APPLE)
  set(CMAKE_PREFIX_PATH /usr/local/Qt-5.5.1)
else (APPLE)
  set(CMAKE_PREFIX_PATH "C:/Qt/Qt5.5.1")
endif (APPLE)


find_package(Qt5Widgets REQUIRED)
#find_package(Qt5Xml REQUIRED)
#find_package(Qt5Network REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Boost REQUIRED)

qt5_add_resources(CRASHREPORTER_UI_RESOURCES ${CRASHREPORTER_RESOURCES})
qt5_add_translation(CRASHREPORTER_QM_FILES ${CRASHREPORTER_TRANSLATIONS})

#build
if (APPLE)

  # icon files to copy in the bundle
#  set( OSX_ICON_FILES res/crashview.icns )

  # set where in the bundle to put the icns files
#  set_source_files_properties( ${OSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION res)
  set_source_files_properties( ${QM_FILES1} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/translations)

  # include the icns files in the target
#  set( CRASHREPORTER_SOURCES ${CRASHREPORTER_SOURCES} ${OSX_ICON_FILES} )
  set( CRASHREPORTER_SOURCES ${CRASHREPORTER_SOURCES})

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wall -pedantic -Wextra -Wnon-virtual-dtor -Woverloaded-virtual")
  include_directories(SYSTEM /usr/local/include src)
  add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${CRASHREPORTER_SOURCES} ${CRASHREPORTER_HEADERS} ${CRASHREPORTER_UIS} ${CRASHREPORTER_UI_RESOURCES} ${CRASHREPORTER_QM_FILES})

else (APPLE)

  include_directories(SYSTEM include src ${Boost_INCLUDE_DIRS})
  add_executable(${PROJECT_NAME} WIN32 ${CRASHREPORTER_SOURCES} ${CRASHREPORTER_HEADERS} ${CRASHREPORTER_UIS} ${CRASHREPORTER_UI_RESOURCES} ${CRASHREPORTER_QM_FILES})

endif (APPLE)

#target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Network Qt5::Xml)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
