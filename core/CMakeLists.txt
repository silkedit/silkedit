set(MODULE_NAME ${PROJECT_NAME}_core)

file(GLOB SILK_CORE_SOURCES *.cpp)
file(GLOB SILK_CORE_HEADERS *.h)

if (APPLE)
  find_library(ONIG_LIBRARY NAMES libonig.a PATHS /usr/local/lib)
  find_library(UCHARDET_LIBRARY NAMES libuchardet.a PATHS /usr/local/lib)
  find_library(YAML_CPP_LIBRARY NAMES libyaml-cpp.a PATHS /usr/local/lib)
elseif (MINGW)
  set(ONIG_LIBRARY onig.a)
  set(UCHARDET_LIBRARY uchardet.a)
  set(YAML_CPP_LIBRARY yaml-cpp)
endif (APPLE)

add_library(${MODULE_NAME} ${SILK_CORE_SOURCES} ${SILK_CORE_HEADERS})
target_link_libraries(${MODULE_NAME} Qt5::Widgets Qt5::Xml ${ONIG_LIBRARY} ${UCHARDET_LIBRARY} ${YAML_CPP_LIBRARY})
