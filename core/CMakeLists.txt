set(MODULE_NAME ${PROJECT_NAME}_core)

if (MSVC)
  # ICU requires this when linking with a static one
  ADD_DEFINITIONS(-DU_STATIC_IMPLEMENTATION)
endif ()

include_directories(SYSTEM
  ${PROJECT_SOURCE_DIR} ${Squirrel_INCLUDE_DIRS}
)

file(GLOB_RECURSE SILK_CORE_SOURCES *.cpp *.mm)
if (APPLE)
  list(REMOVE_ITEM SILK_CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/atom/node_bindings_win.cpp ${CMAKE_CURRENT_SOURCE_DIR}/SquirrelAutoUpdater_win.cpp)
elseif (MSVC)
  list(REMOVE_ITEM SILK_CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/atom/node_bindings_mac.cpp ${CMAKE_CURRENT_SOURCE_DIR}/SquirrelAutoUpdater_mac.mm)
endif ()

file(GLOB_RECURSE SILK_CORE_HEADERS *.h)
add_library(${MODULE_NAME} STATIC ${SILK_CORE_SOURCES} ${SILK_CORE_HEADERS})

if (APPLE)
  find_library(ONIG_LIBRARY NAMES libonig.a PATHS /usr/local/lib)

  # ReactiveCocoa uses ARC so we need to set -fobjc-weak
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/SquirrelAutoUpdater_mac.mm PROPERTIES COMPILE_FLAGS "-fobjc-weak")

  target_link_libraries(${MODULE_NAME}
                        Qt5::Widgets
                        Qt5::Xml
                        ${ONIG_LIBRARY}
                        ${UCHARDET}
                        ${YAML_CPP}
                        ${NODE_LIBRARY}
                        ${ICU_UC}
                        ${ICU_DATA}
                        ${Squirrel}
                        ${Mantle}
                        ${ReactiveCocoa}
                        "-framework Foundation"

  )
elseif (MSVC)
  add_library(ONIG_LIBRARY SHARED IMPORTED)
  set_target_properties(ONIG_LIBRARY PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/vendor/Onigmo)
  set_property(TARGET ONIG_LIBRARY PROPERTY IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/lib/onig.dll)
  set_property(TARGET ONIG_LIBRARY PROPERTY IMPORTED_IMPLIB ${PROJECT_SOURCE_DIR}/lib/onig.lib)

  target_link_libraries(${MODULE_NAME}
                        Qt5::Widgets
                        Qt5::Xml
                        ONIG_LIBRARY
                        ${UCHARDET}
                        ${YAML_CPP}
                        ${NODE_LIBRARY}
                        ${ICU_UC}
                        ${ICU_DATA}
  )
endif ()

