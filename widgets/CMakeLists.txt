set(MODULE_NAME ${PROJECT_NAME}_widgets)

if (MSVC)
  # qtlockedfile_win.cpp needs UNICODE defined
  ADD_DEFINITIONS(-DUNICODE)
endif ()

file(GLOB_RECURSE SILK_WIDGETS_SOURCES src/*.cpp)

# Exclude qtlockedfile files. They are supposed to be #included in qtlocalpeer.h
list(REMOVE_ITEM SILK_WIDGETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/qtsingleapplication/qtlockedfile.cpp)
list(REMOVE_ITEM SILK_WIDGETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/qtsingleapplication/qtlockedfile_win.cpp)
list(REMOVE_ITEM SILK_WIDGETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/qtsingleapplication/qtlockedfile_unix.cpp)

file(GLOB_RECURSE SILK_WIDGETS_HEADERS src/*.h)

add_library(${MODULE_NAME} STATIC ${SILK_WIDGETS_SOURCES} ${SILK_WIDGETS_HEADERS})

target_include_directories(${MODULE_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR}
)

if (APPLE)
  target_link_libraries(${MODULE_NAME}
                        ${PROJECT_NAME}_core
                        Qt5::Widgets
                        Qt5::Network
                        Qt5::Xml
                        Qt5::Svg
                        Qt5::WebEngineWidgets
  )
elseif (MSVC)
  target_link_libraries(${MODULE_NAME}
                        ${PROJECT_NAME}_core
                        Qt5::Widgets
                        Qt5::Network
                        Qt5::Xml
                        Qt5::Svg
                        Qt5::WebEngineWidgets
  )
endif ()
