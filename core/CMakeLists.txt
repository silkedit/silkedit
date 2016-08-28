set(MODULE_NAME ${PROJECT_NAME}_core)

if (MSVC)
  # ICU requires this when linking with a static one
  ADD_DEFINITIONS(-DU_STATIC_IMPLEMENTATION)
endif ()

include(${PROJECT_SOURCE_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

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
  find_library(UCHARDET_LIBRARY NAMES libuchardet.a PATHS /usr/local/lib)
  find_library(YAML_CPP_LIBRARY NAMES libyaml-cpp.a PATHS ${CONAN_LIB_DIRS_YAML-CPP})
  find_library(ICUUC_LIBRARY NAMES libicuuc.a PATHS /usr/local/lib)
  find_library(ICUDATA_LIBRARY NAMES libicudata.a PATHS /usr/local/lib)

  # ReactiveCocoa uses ARC so we need to set -fobjc-weak
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/SquirrelAutoUpdater_mac.mm PROPERTIES COMPILE_FLAGS "-fobjc-weak")

  target_link_libraries(${MODULE_NAME}
                        Qt5::Widgets
                        Qt5::Xml
                        ${ONIG_LIBRARY}
                        ${UCHARDET_LIBRARY}
                        ${YAML_CPP_LIBRARY}
                        ${NODE_LIBRARY}
                        ${ICUUC_LIBRARY}
                        ${ICUDATA_LIBRARY}
                        ${Squirrel}
                        ${Mantle}
                        ${ReactiveCocoa}
                        "-framework Foundation"

  )
elseif (MSVC)
  add_library(YAML_CPP_LIBRARY STATIC IMPORTED)
  set_property(TARGET YAML_CPP_LIBRARY PROPERTY IMPORTED_LOCATION_RELEASE ${PROJECT_SOURCE_DIR}/lib/libyaml-cppmd.lib)
  set_property(TARGET YAML_CPP_LIBRARY PROPERTY IMPORTED_LOCATION_DEBUG ${PROJECT_SOURCE_DIR}/lib/libyaml-cppmdd.lib)

  add_library(UCHARDET_LIBRARY STATIC IMPORTED)
  set_property(TARGET UCHARDET_LIBRARY PROPERTY IMPORTED_LOCATION_RELEASE ${PROJECT_SOURCE_DIR}/lib/uchardet/Release/uchardet.lib)
  set_property(TARGET UCHARDET_LIBRARY PROPERTY IMPORTED_LOCATION_DEBUG ${PROJECT_SOURCE_DIR}/lib/uchardet/Debug/uchardet.lib)

  add_library(ICUUC_LIBRARY STATIC IMPORTED)
  set_property(TARGET ICUUC_LIBRARY PROPERTY IMPORTED_LOCATION_RELEASE ${PROJECT_SOURCE_DIR}/lib/icu/sicuuc.lib)
  set_property(TARGET ICUUC_LIBRARY PROPERTY IMPORTED_LOCATION_DEBUG ${PROJECT_SOURCE_DIR}/lib/icu/sicuucd.lib)

  add_library(ICUDATA_LIBRARY STATIC IMPORTED)
  set_property(TARGET ICUDATA_LIBRARY PROPERTY IMPORTED_LOCATION_RELEASE ${PROJECT_SOURCE_DIR}/lib/icu/sicudt.lib)
  set_property(TARGET ICUDATA_LIBRARY PROPERTY IMPORTED_LOCATION_DEBUG ${PROJECT_SOURCE_DIR}/lib/icu/sicudtd.lib)

  add_library(ONIG_LIBRARY SHARED IMPORTED)
  set_target_properties(ONIG_LIBRARY PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/vendor/Onigmo)
  set_property(TARGET ONIG_LIBRARY PROPERTY IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/lib/onig.dll)
  set_property(TARGET ONIG_LIBRARY PROPERTY IMPORTED_IMPLIB ${PROJECT_SOURCE_DIR}/lib/onig.lib)

  target_link_libraries(${MODULE_NAME}
                        Qt5::Widgets
                        Qt5::Xml
                        ONIG_LIBRARY
                        UCHARDET_LIBRARY
                        YAML_CPP_LIBRARY
                        NODE_LIBRARY
                        ICUUC_LIBRARY
                        ICUDATA_LIBRARY
  )
endif ()

