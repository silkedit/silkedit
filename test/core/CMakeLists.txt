include_directories(SYSTEM
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/core
)

file(COPY "testdata" DESTINATION .)

if (APPLE)
  # copy libnode.dylib
  file(COPY ${NODE_LIBRARY} DESTINATION .)
elseif (MSVC)
  # Copy onig.dll
  file(COPY ${PROJECT_SOURCE_DIR}/lib/onig.dll DESTINATION .)

  # Copy node.dll
  if (USE_DEBUG_NODE)
    file(COPY ${PROJECT_SOURCE_DIR}/vendor/node/Debug/node.dll DESTINATION .)
  else ()
    file(COPY ${PROJECT_SOURCE_DIR}/vendor/node/Release/node.dll DESTINATION .)
  endif ()
endif ()

function(add_unittest test_class)
  add_executable(${test_class} EXCLUDE_FROM_ALL ${test_class}.cpp TestUtil.cpp)
  target_link_libraries(${test_class} Qt5::Test SilkEdit_core)
  add_test( NAME ${test_class} COMMAND ${test_class} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  add_dependencies(unit_test ${test_class})

if (APPLE)
  # run install_name_tool to update the path to libnode.dylib
  add_custom_command(TARGET ${test_class} POST_BUILD COMMAND install_name_tool -change /usr/local/lib/libnode.dylib @executable_path/libnode.dylib ${test_class})
endif ()
endfunction()

add_unittest(LanguageParserTest)
add_unittest(ThemeTest)
add_unittest(UtilTest)
add_unittest(SyntaxHighlighterTest)
add_unittest(RegexpTest)
add_unittest(RegionTest)
add_unittest(TextEditViewLogicTest)
add_unittest(LineSeparatorTest)
add_unittest(PackageTest)
add_unittest(BOMTest)
add_unittest(V8UtilTest)
add_unittest(QObjectUtilTest)
add_unittest(DocumentTest)
