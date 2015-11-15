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
set(SILK_RESOURCES resources/style.qrc resources/images_common.qrc)
if (BUILD_EDGE)
  set(SILK_RESOURCES ${SILK_RESOURCES} resources/images_edge.qrc)
else ()
  set(SILK_RESOURCES ${SILK_RESOURCES} resources/images.qrc)
endif ()
file(GLOB SILK_TRANSLATIONS translations/*.ts)

if (APPLE)
  set(CMAKE_PREFIX_PATH /usr/local/Qt-5.5.1)
else (APPLE)
  set(CMAKE_PREFIX_PATH "C:/Qt/Qt5.5.1")
endif (APPLE)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5WebEngineWidgets REQUIRED)
find_package(Boost REQUIRED)

qt5_add_resources(UI_RESOURCES ${SILK_RESOURCES})
qt5_add_translation(QM_FILES ${SILK_TRANSLATIONS})

if (APPLE)
  # icon files to copy in the bundle
  set( OSX_ICON_FILES resources/silkedit.icns )

  # set where in the bundle to put the icns files
  set_source_files_properties( ${OSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  set_source_files_properties( ${QM_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/translations)

  # include the icns files in the target
  set( SILK_SOURCES ${SILK_SOURCES} ${OSX_ICON_FILES} )

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wall -pedantic -Wextra -Wnon-virtual-dtor -Woverloaded-virtual -g")
  include_directories(SYSTEM /usr/local/include msgpack-c/include src breakpad/src)

  # breakpad framework
  set( BREAKPAD_LIBS "breakpad/src/client/mac/build/${CMAKE_BUILD_TYPE}")
  link_directories(${BREAKPAD_LIBS})

  add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SILK_SOURCES} ${SILK_HEADERS} ${SILK_UIS} ${UI_RESOURCES} ${QM_FILES})
else (APPLE)
#
#if(MSVC)
  string(REGEX REPLACE "/Z[iI7]" ""
         CMAKE_CXX_FLAGS_RELEASE
         "${CMAKE_CXX_FLAGS_RELEASE}")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
  string(REGEX REPLACE "/INCREMENTAL:NO" ""
         CMAKE_EXE_LINKER_FLAGS_RELEASE
         "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /debug /OPT:REF")
#endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_SCL_SECURE_NO_WARNINGS")

  #breakpad libs under 'lib' folder
  set( BREAKPAD_LIBS "breakpad/${CMAKE_BUILD_TYPE}")

  # SYSTEM tells the compiler to skip warning for these header files.
  include_directories(SYSTEM msgpack-c/include include src ${Boost_INCLUDE_DIRS} breakpad/src)
  link_directories(lib)

  if (BUILD_EDGE)
    set( SILK_ICON resources/silkedit_edge.rc)
  else (BUILD_EDGE)
    set( SILK_ICON resources/silkedit.rc)
  endif (BUILD_EDGE)

  add_executable(${PROJECT_NAME} WIN32 ${SILK_SOURCES} ${SILK_HEADERS} ${SILK_UIS} ${UI_RESOURCES} ${QM_FILES} ${SILK_ICON})
endif (APPLE)

INCLUDE(Dart)
# enable_testing should be in the root
enable_testing()
add_subdirectory(core)
add_subdirectory(crashreporter)
add_subdirectory(test)

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_core Qt5::Widgets Qt5::Network Qt5::Xml Qt5::WebEngineWidgets ${YAML_CPP_LIBRARY})
# brakpad lib/framework
if(APPLE)
  target_link_libraries(${PROJECT_NAME}
    "-F${PROJECT_SOURCE_DIR}/${BREAKPAD_LIBS}"
    "-framework Breakpad")
else (APPLE)
  target_link_libraries(${PROJECT_NAME} ${BREAKPAD_LIBS}/common ${BREAKPAD_LIBS}/exception_handler ${BREAKPAD_LIBS}/crash_generation_client)
ENDIF(APPLE)

if(APPLE)
  # bundle packages
  file(COPY ${PROJECT_SOURCE_DIR}/packages DESTINATION ${PROJECT_NAME}.app/Contents/MacOS)

  # bundle silkedit_helper (node.js)
  file(COPY ${PROJECT_SOURCE_DIR}/helper DESTINATION ${PROJECT_NAME}.app/Contents/MacOS)

  set_target_properties( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/src/SilkEdit-info.plist)
  set_target_properties( ${PROJECT_NAME} PROPERTIES FRAMEWORK true )

  # Use special icon for edge build
  if (BUILD_EDGE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp ${PROJECT_SOURCE_DIR}/resources/silkedit_edge.icns ${PROJECT_NAME}.app/Contents/Resources/silkedit.icns)
  endif (BUILD_EDGE)

  # bundle fonts
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp -r ${PROJECT_SOURCE_DIR}/resources/fonts ${PROJECT_NAME}.app/Contents/Resources)

  # Copy qt_<locale>.qm files because macdeployqt doesn't copy them (windeployqt does)
  file(COPY ${CMAKE_PREFIX_PATH}/translations/qt_ja.qm
            ${CMAKE_PREFIX_PATH}/translations/qtmultimedia_ja.qm
            ${CMAKE_PREFIX_PATH}/translations/qtbase_ja.qm
            ${CMAKE_PREFIX_PATH}/translations/qtquick1_ja.qm
            ${CMAKE_PREFIX_PATH}/translations/qtscript_ja.qm
            ${CMAKE_PREFIX_PATH}/translations/qtxmlpatterns_ja.qm DESTINATION ${PROJECT_NAME}.app/Contents/Resources/translations)

  # breakpad framework
  file(COPY ${BREAKPAD_LIBS}/breakpad.framework DESTINATION ${PROJECT_NAME}.app/Contents/Frameworks)

else (APPLE)
  # Copy packages directory to the same directory of exe
  file(COPY ${PROJECT_SOURCE_DIR}/packages DESTINATION .)

  # Copy silkedit_helper (node.js)
  file(COPY ${PROJECT_SOURCE_DIR}/helper DESTINATION .)

  # Copy onig.dll
  file(COPY ${PROJECT_SOURCE_DIR}/lib/onig.dll DESTINATION .)
ENDIF(APPLE)
