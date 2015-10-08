include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/core)

file(COPY "testdata" DESTINATION .)

function(add_unittest test_class)
  add_executable(${test_class} EXCLUDE_FROM_ALL ${test_class}.cpp)
  target_link_libraries(${test_class} Qt5::Test SilkEdit_core)
  add_test( NAME ${test_class} COMMAND ${test_class} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  add_dependencies(unit_test ${test_class})
endfunction()

add_unittest(LanguageParserTest)
add_unittest(ThemeTest)
add_unittest(UtilTest)
add_unittest(SyntaxHighlighterTest)
add_unittest(RegexpTest)
add_unittest(RegionTest)
add_unittest(TextEditViewLogicTest)
add_unittest(LineSeparatorTest)
