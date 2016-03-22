cmake_minimum_required(VERSION 3.0.2)

project(SilkEdit)

option (BUILD_EDGE "Build edge version" OFF)
option (USE_DEBUG_NODE "Use debug build of libnode (it's slow but useful for debugging because it has many debug only assertions)" OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Automatically add the current source- and build directories to the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Node.js defines HAVE_OPENSSL by default
ADD_DEFINITIONS(-DHAVE_OPENSSL)

# '+' will automatically be performed as the QStringBuilder '%' everywhere.
ADD_DEFINITIONS(-DQT_USE_QSTRINGBUILDER)

# These are necessary to import v8 and libuv symbols in node.dll
ADD_DEFINITIONS(-DUSING_V8_SHARED)
ADD_DEFINITIONS(-DUSING_UV_SHARED)

file(GLOB_RECURSE SILK_SOURCES src/*.cpp)
if (APPLE)
  ADD_DEFINITIONS(-D__POSIX__)
  list(REMOVE_ITEM SILK_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/atom/node_bindings_win.cpp)
elseif (MSVC)
  list(REMOVE_ITEM SILK_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/atom/node_bindings_mac.cpp)
endif ()

file(GLOB_RECURSE SILK_HEADERS src/*.h)

file(GLOB_RECURSE SILK_UIS src/*.ui)
set(SILK_RESOURCES resources/style.qrc resources/images_common.qrc resources/files.qrc)
if (BUILD_EDGE)
  set(SILK_RESOURCES ${SILK_RESOURCES} resources/images_edge.qrc)
else ()
  set(SILK_RESOURCES ${SILK_RESOURCES} resources/images.qrc)
endif ()
file(GLOB SILK_TRANSLATIONS translations/*.ts)

set( BREAKPAD_LIBS "breakpad/src/client/mac/build/${CMAKE_BUILD_TYPE}")
if (APPLE)
  set(CMAKE_PREFIX_PATH "/usr/local/Qt-5.6.0")
else (MSVC)
  set(CMAKE_PREFIX_PATH "C:/Qt/Qt5.6.0")
endif ()

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5WebEngineWidgets REQUIRED)

find_library(Breakpad Breakpad PATHS ${BREAKPAD_LIBS})
if (Breakpad)
    message(STATUS "Breakpad: ${Breakpad}")
endif()
if (NOT Breakpad)
    message(FATAL_ERROR "Breakpad not found")
endif()

if (APPLE)
  if (USE_DEBUG_NODE)
    find_library(NODE_LIBRARY NAMES libnode_debug.dylib PATHS ${PROJECT_SOURCE_DIR}/vendor/node/out/Debug NO_DEFAULT_PATH)
  else ()
    find_library(NODE_LIBRARY NAMES libnode.dylib PATHS ${PROJECT_SOURCE_DIR}/vendor/node/out/Release NO_DEFAULT_PATH)
  endif ()
elseif (MSVC)
  add_library(NODE_LIBRARY SHARED IMPORTED)
  if (USE_DEBUG_NODE)
    set_property(TARGET NODE_LIBRARY PROPERTY IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/vendor/node/Debug/node.dll)
    set_property(TARGET NODE_LIBRARY PROPERTY IMPORTED_IMPLIB ${PROJECT_SOURCE_DIR}/vendor/node/Debug/node.lib)
  else ()
    set_property(TARGET NODE_LIBRARY PROPERTY IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/vendor/node/Release/node.dll)
    set_property(TARGET NODE_LIBRARY PROPERTY IMPORTED_IMPLIB ${PROJECT_SOURCE_DIR}/vendor/node/Release/node.lib)
  endif ()
endif ()
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

  # -Wno-gnu-zero-variadic-macro-arguments: to suppress warnings of qCDebug(category, ...)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -stdlib=libc++ -Wall -pedantic -Wextra -Wnon-virtual-dtor -Woverloaded-virtual -Wno-gnu-zero-variadic-macro-arguments -g")
  include_directories(SYSTEM
    /usr/local/include
    src
    breakpad/src
    vendor/node/deps/v8
    vendor/node/deps/v8/include
    vendor/node/deps/uv/include
    vendor/node/deps/cares/include
    vendor/node/deps/openssl/openssl/include)

  add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SILK_SOURCES} ${SILK_HEADERS} ${SILK_UIS} ${UI_RESOURCES} ${QM_FILES})
elseif (MSVC)
  string(REGEX REPLACE "/Z[iI7]" ""
         CMAKE_CXX_FLAGS_RELEASE
         "${CMAKE_CXX_FLAGS_RELEASE}")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
  string(REGEX REPLACE "/INCREMENTAL:NO" ""
         CMAKE_EXE_LINKER_FLAGS_RELEASE
         "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /debug /OPT:REF")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_SCL_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_WARNINGS")

  #breakpad libs under 'lib' folder
  set( BREAKPAD_LIBS "breakpad/${CMAKE_BUILD_TYPE}")

  # SYSTEM tells the compiler to skip warning for these header files.
  include_directories(SYSTEM
    include
    src
    ${Boost_INCLUDE_DIRS}
    breakpad/src
    vendor/node/deps/v8
    vendor/node/deps/v8/include
    vendor/node/deps/uv/include
    vendor/node/deps/cares/include
    vendor/node/deps/openssl/openssl/include
  )
  link_directories(${PROJECT_SOURCE_DIR}/lib)

  if (BUILD_EDGE)
    set( SILK_ICON resources/silkedit_edge.rc)
  else ()
    set( SILK_ICON resources/silkedit.rc)
  endif ()

  add_executable(${PROJECT_NAME} WIN32 ${SILK_SOURCES} ${SILK_HEADERS} ${SILK_UIS} ${UI_RESOURCES} ${QM_FILES} ${SILK_ICON})
endif ()

INCLUDE(Dart)
# enable_testing should be in the root
enable_testing()
add_subdirectory(core)
add_subdirectory(crashreporter)
add_subdirectory(test)

if(APPLE)
  target_link_libraries(${PROJECT_NAME}
                        ${PROJECT_NAME}_core
                        Qt5::Widgets
                        Qt5::Network
                        Qt5::Xml
                        Qt5::Svg
                        Qt5::WebEngineWidgets
                        ${YAML_CPP_LIBRARY}
                        ${NODE_LIBRARY}
                        ${Breakpad}
)
elseif (MSVC)
  target_link_libraries(${PROJECT_NAME}
                        ${PROJECT_NAME}_core
                        Qt5::Widgets
                        Qt5::Network
                        Qt5::Xml
                        Qt5::Svg
                        Qt5::WebEngineWidgets
                        ${YAML_CPP_LIBRARY}
                        NODE_LIBRARY
  )
  target_link_libraries(${PROJECT_NAME} ${BREAKPAD_LIBS}/common ${BREAKPAD_LIBS}/exception_handler ${BREAKPAD_LIBS}/crash_generation_client)
endif()

if(APPLE)
  # bundle themes
  file(COPY ${PROJECT_SOURCE_DIR}/themes DESTINATION ${PROJECT_NAME}.app/Contents/Resources)

  # bundle builtin packages
  file(COPY ${PROJECT_SOURCE_DIR}/packages DESTINATION ${PROJECT_NAME}.app/Contents/Resources)

  # copy npm
  file(COPY
     ${PROJECT_SOURCE_DIR}/npm/bin/npm-cli.js
     DESTINATION
     ${PROJECT_NAME}.app/Contents/Resources/npm/bin)

  file(COPY
     ${PROJECT_SOURCE_DIR}/npm/lib
     ${PROJECT_SOURCE_DIR}/npm/node_modules
     ${PROJECT_SOURCE_DIR}/npm/package.json
     DESTINATION
     ${PROJECT_NAME}.app/Contents/Resources/npm)

  # bundle jslib
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources/jslib)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/minified/jslib ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources/jslib)

  set_target_properties( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/src/SilkEdit-info.plist)
  set_target_properties( ${PROJECT_NAME} PROPERTIES FRAMEWORK true )

  # run install_name_tool to update the path to libnode.dylib
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND install_name_tool -change /usr/local/lib/libnode.dylib @executable_path/../Frameworks/libnode.dylib ${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME})

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
  file(COPY ${Breakpad} DESTINATION ${PROJECT_NAME}.app/Contents/Frameworks)

  # copy libnode.dylib
  file(COPY ${NODE_LIBRARY} DESTINATION ${PROJECT_NAME}.app/Contents/Frameworks)

  # copy CrashReporter
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp -R ${PROJECT_BINARY_DIR}/crashreporter/CrashReporter.app/Contents/MacOS/CrashReporter ${PROJECT_NAME}.app/Contents/MacOS)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp ${PROJECT_BINARY_DIR}/crashreporter/crashreporter_*.qm ${PROJECT_NAME}.app/Contents/Resources/translations)

elseif (MSVC)
  # Copy themes
  file(COPY ${PROJECT_SOURCE_DIR}/themes DESTINATION .)

  # Copy packages directory to the same directory of exe
  file(COPY ${PROJECT_SOURCE_DIR}/packages DESTINATION .)

  # bundle jslib
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove_directory ./jslib)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/minified/jslib ./jslib)

  # copy npm
  file(COPY
     ${PROJECT_SOURCE_DIR}/npm/bin/npm-cli.js
     DESTINATION npm/bin)

  file(COPY
     ${PROJECT_SOURCE_DIR}/npm/lib
     ${PROJECT_SOURCE_DIR}/npm/node_modules
     ${PROJECT_SOURCE_DIR}/npm/package.json
     DESTINATION npm)

  # Copy onig.dll
  file(COPY ${PROJECT_SOURCE_DIR}/lib/onig.dll DESTINATION .)

  # Copy node.dll
  if (USE_DEBUG_NODE)
    file(COPY ${PROJECT_SOURCE_DIR}/vendor/node/Debug/node.dll DESTINATION .)
  else ()
    file(COPY ${PROJECT_SOURCE_DIR}/vendor/node/Release/node.dll DESTINATION .)
  endif ()

  # copy CrashReporter
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/crashreporter/crashreporter.exe ${PROJECT_BINARY_DIR})
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/crashreporter/crashreporter_ja.qm ${PROJECT_BINARY_DIR})
endif()
