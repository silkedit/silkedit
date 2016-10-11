add_custom_target(unit_test)
add_custom_target(benchmark)

find_package(Qt5Test REQUIRED)

function(add_unittest dir test_class)
  add_executable(${test_class} EXCLUDE_FROM_ALL src/${dir}/${test_class}.cpp ${PROJECT_SOURCE_DIR}/test/src/util/TestUtil.cpp)
  target_link_libraries(${test_class} Qt5::Test SilkEdit_widgets)
  add_test( NAME ${test_class} COMMAND ${test_class} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  set_property(TEST ${test_class} APPEND PROPERTY LABELS unit_test)
  add_dependencies(unit_test ${test_class})

if (APPLE)
  # run install_name_tool to update the path to libnode.dylib
  add_custom_command(TARGET ${test_class} POST_BUILD COMMAND install_name_tool -change /usr/local/lib/libnode.dylib @executable_path/libnode.dylib ${test_class})
  add_custom_command(TARGET ${test_class} POST_BUILD COMMAND install_name_tool -change libicuuc.57.dylib @executable_path/libicuuc.dylib ${test_class})
  add_custom_command(TARGET ${test_class} POST_BUILD COMMAND install_name_tool -change libicudata.57.dylib @executable_path/libicudata.dylib ${test_class})

endif ()
endfunction()

function(add_benchmark dir test_class)
  add_executable(${test_class} EXCLUDE_FROM_ALL src/${dir}/${test_class}.cpp ${PROJECT_SOURCE_DIR}/test/src/util/TestUtil.cpp)
  target_link_libraries(${test_class} Qt5::Test SilkEdit_widgets)
  add_test( NAME ${test_class} COMMAND ${test_class} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  set_property(TEST ${test_class} APPEND PROPERTY LABELS benchmark)
  add_dependencies(benchmark ${test_class})

if (APPLE)
  # run install_name_tool to update the path to libnode.dylib
  add_custom_command(TARGET ${test_class} POST_BUILD COMMAND install_name_tool -change /usr/local/lib/libnode.dylib @executable_path/libnode.dylib ${test_class})
endif ()
endfunction()

include_directories(SYSTEM
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/widgets/src
  ${CMAKE_SOURCE_DIR}/core
  ${CMAKE_SOURCE_DIR}/test/src/util
)

if (APPLE)
  # copy libnode.dylib
  file(COPY ${NODE_LIBRARY} DESTINATION .)

  # copy icu libraries
  file(COPY ${ICU_UC} DESTINATION .)
  file(COPY ${ICU_DATA} DESTINATION .)
  execute_process(COMMAND install_name_tool -change libicudata.57.dylib @executable_path/libicudata.dylib ${CMAKE_CURRENT_BINARY_DIR}/libicuuc.dylib)

elseif (MSVC)
  # Copy onig.dll
  file(COPY ${PROJECT_SOURCE_DIR}/lib/onig.dll DESTINATION .)

  # Copy node.dll
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    file(COPY ${PROJECT_SOURCE_DIR}/vendor/node/Debug/node.dll DESTINATION .)
  else ()
    file(COPY ${CONAN_BIN_DIRS_NODE}/node.dll DESTINATION .)
  endif ()

  # Copy pdb
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#    file(COPY "${PROJECT_SOURCE_DIR}/lib/yaml-cpp static md.pdb" DESTINATION .)
    file(COPY ${PROJECT_SOURCE_DIR}/lib/icu/icuMDd.pdb DESTINATION .)
  endif()
endif ()

file(COPY "testdata" DESTINATION .)
file(COPY ${PROJECT_SOURCE_DIR}/resources/icudt57l.dat DESTINATION .)

# core tests
add_unittest(core LanguageParserTest)
add_unittest(core ThemeTest)
add_unittest(core UtilTest)
add_unittest(core SyntaxHighlighterTest)
add_unittest(core RegexpTest)
add_unittest(core RegionTest)
add_unittest(core TextEditLogicTest)
add_unittest(core LineSeparatorTest)
add_unittest(core PackageTest)
add_unittest(core BOMTest)
add_unittest(core V8UtilTest)
add_unittest(core QObjectUtilTest)
add_unittest(core DocumentTest)
add_unittest(core TextCursorTest)

# widgets tests
add_unittest(widgets YamlUtilTest)

# benchmarks
add_benchmark(core SyntaxHighlighterBenchmark)
