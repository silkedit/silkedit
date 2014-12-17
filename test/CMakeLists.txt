cmake_minimum_required(VERSION 2.8.11)

project(silkedit_test)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Automatically add the current source- and build directories to the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -Wextra -Wnon-virtual-dtor -Woverloaded-virtual")

file(COPY "testdata" DESTINATION .)

# Find the QtWidgets library
find_package(Qt5Gui REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Test REQUIRED)

# Find oniguruma using pkg-config
INCLUDE(FindPkgConfig)
pkg_check_modules(Oniguruma REQUIRED oniguruma)

include_directories(../src src ${Oniguruma_INCLUDE_DIRS})

link_directories(${Oniguruma_LIBRARY_DIRS})

set(TEST_LIBRARIES ${Oniguruma_LIBRARIES})

set(LANGUAGE_PARSER_TEST_SRCS ../src/TmLanguage.cpp ../src/PListParser.cpp ../src/Regexp.cpp src/LanguageParserTest.cpp)
add_executable(LanguageParserTest ${LANGUAGE_PARSER_TEST_SRCS} )
target_link_libraries(LanguageParserTest Qt5::Gui Qt5::Xml Qt5::Test ${TEST_LIBRARIES})
add_test( name LanguageParserTest command LanguageParserTest)

set(THEME_TEST_SRCS ../src/Theme.cpp ../src/PListParser.cpp src/ThemeTest.cpp)
add_executable(ThemeTest ${THEME_TEST_SRCS} )
target_link_libraries(ThemeTest Qt5::Gui Qt5::Xml Qt5::Test )
add_test( name ThemeTest command ThemeTest)

set(UTIL_TEST_SRCS ../src/Util.cpp src/UtilTest.cpp)
add_executable(UtilTest ${UTIL_TEST_SRCS} )
target_link_libraries(UtilTest Qt5::Gui Qt5::Xml Qt5::Test )
add_test( name UtilTest command UtilTest)

set(SYNTAX_HIGHLIGHTER_TEST_SRCS ../src/SyntaxHighlighter.cpp ../src/TmLanguage.cpp ../src/PListParser.cpp ../src/Regexp.cpp ../src/Util.cpp ../src/Theme.cpp src/SyntaxHighlighterTest.cpp)
add_executable(SyntaxHighlighterTest ${SYNTAX_HIGHLIGHTER_TEST_SRCS} )
target_link_libraries(SyntaxHighlighterTest Qt5::Gui Qt5::Xml Qt5::Test ${TEST_LIBRARIES})
add_test( name SyntaxHighlighterTest command SyntaxHighlighterTest)

set(REGEXP_TEST_SRCS ../src/Regexp.cpp src/RegexpTest.cpp)
add_executable(RegexpTest ${REGEXP_TEST_SRCS} )
target_link_libraries(RegexpTest Qt5::Gui Qt5::Xml Qt5::Test ${TEST_LIBRARIES})
add_test( name RegexpTest command RegexpTest)
