#
# CrashView cmake file
#

cmake_minimum_required(VERSION 3.0.2)

project(CrashView)

# Base project folder(relative path)
#set(BASE_PROJECT_DIR "../..")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Automatically add the current source- and build directories to the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE CRASHVIEW_SOURCES src/*.cpp src/*.c)
file(GLOB_RECURSE CRASHVIEW_HEADERS src/*.h)
file(GLOB_RECURSE CRASHVIEW_UIS src/*.ui)
file(GLOB CRASHVIEW_TRANSLATIONS trans/*.ts)

if (APPLE)
  file(GLOB_RECURSE PLATFORM_RESOURCES res/mac/*.qrc)
else (APPLE)
  file(GLOB_RECURSE PLATFORM_RESOURCES res/win/*.qrc)
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

qt5_add_resources(UI_RESOURCES ${CRASHVIEW_RESOURCES})
qt5_add_translation(QM_FILES ${CRASHVIEW_TRANSLATIONS})

#build
if (APPLE)

  # icon files to copy in the bundle
#  set( OSX_ICON_FILES res/crashview.icns )

  # set where in the bundle to put the icns files
#  set_source_files_properties( ${OSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION res)
  set_source_files_properties( ${QM_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION res/translations)

  # include the icns files in the target
#  set( CRASHVIEW_SOURCES ${CRASHVIEW_SOURCES} ${OSX_ICON_FILES} )
  set( CRASHVIEW_SOURCES ${CRASHVIEW_SOURCES})

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wall -pedantic -Wextra -Wnon-virtual-dtor -Woverloaded-virtual")
  include_directories(SYSTEM /usr/local/include src)
  add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${CRASHVIEW_SOURCES} ${CRASHVIEW_HEADERS} ${CRASHVIEW_UIS} ${UI_RESOURCES} ${QM_FILES})

else (APPLE)

  include_directories(SYSTEM include src ${Boost_INCLUDE_DIRS})
  add_executable(${PROJECT_NAME} WIN32 ${CRASHVIEW_SOURCES} ${CRASHVIEW_HEADERS} ${CRASHVIEW_UIS} ${UI_RESOURCES} ${QM_FILES})

endif (APPLE)

#INCLUDE(Dart)

#target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Network Qt5::Xml)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)


#publish
if(APPLE)

  file(COPY /usr/local/bin/minidump_stackwalk DESTINATION ${PROJECT_NAME}.app/Contents/MacOS/)
  file(COPY /usr/lib/libc++.1.dylib DESTINATION ${PROJECT_NAME}.app/Contents/MacOS/)
  file(COPY /usr/lib/libSystem.B.dylib DESTINATION ${PROJECT_NAME}.app/Contents/MacOS/)

else (APPLE)

  #file(COPY /usr/local/bin/minidump_stackwalk.exe
  #          /usr/bin/msys-2.0.dll
  #          /usr/bin/msys-gcc_s-seh-1.dll
  #          /usr/bin/msys-stdc++-6.dll DESTINATION .)

ENDIF(APPLE)
