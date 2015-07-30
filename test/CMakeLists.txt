cmake_minimum_required(VERSION 2.8.11)

project(silkedit_test)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Automatically add the current source- and build directories to the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (APPLE)
  set(CMAKE_PREFIX_PATH /usr/local/Qt-5.4.2)
elseif (MINGW)
  if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set(CMAKE_PREFIX_PATH "C:/Qt/Qt5.4.2/mingw64")
  else ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set(CMAKE_PREFIX_PATH "C:/Qt/Qt5.4.2/mingw32")
  endif ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
endif (APPLE)

file(COPY "testdata" DESTINATION .)

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Test REQUIRED)

if (APPLE)
  # find yaml-cpp and onig library
  find_library(YAML_CPP_LIBRARY NAMES libyaml-cpp.a PATHS /usr/local/lib)
  find_library(ONIG_LIBRARY NAMES libonig.a PATHS /usr/local/lib)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wall -pedantic -Wextra -Wnon-virtual-dtor -Woverloaded-virtual")
  include_directories(/usr/local/include ../msgpack-c/include ../src)
elseif (MINGW)
  set(YAML_CPP_LIBRARY yaml-cpp)
  set(ONIG_LIBRARY onig.a)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -static -static-libgcc -static-libstdc++ -Wall -pedantic -Wextra -Wnon-virtual-dtor -Woverloaded-virtual")
  if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( MSYS_INCLUDE c:/msys64/mingw64/include )
    set( MSYS_LIB c:/msys64/mingw64/lib )
  else ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( MSYS_INCLUDE c:/msys32/mingw32/include )
    set( MSYS_LIB c:/msys32/mingw32/lib )
  endif ( CMAKE_SIZEOF_VOID_P EQUAL 8 )

  # SYSTEM tells the compiler to skip warning for these header files.
  include_directories(SYSTEM ../msgpack-c/include ../src ${MSYS_INCLUDE} ../include)
  link_directories(${MSYS_LIB} ../lib)
endif (APPLE)

ENABLE_TESTING()

set(LANGUAGE_PARSER_TEST_SRCS ../src/LanguageParser.cpp ../src/PListParser.cpp ../src/Regexp.cpp ../src/Region.cpp src/LanguageParserTest.cpp)
add_executable(LanguageParserTest ${LANGUAGE_PARSER_TEST_SRCS} )
target_link_libraries(LanguageParserTest Qt5::Gui Qt5::Xml Qt5::Test ${YAML_CPP_LIBRARY} ${ONIG_LIBRARY})
add_test( NAME LanguageParserTest COMMAND LanguageParserTest)

set(THEME_TEST_SRCS ../src/Theme.cpp ../src/PListParser.cpp src/ThemeTest.cpp)
add_executable(ThemeTest ${THEME_TEST_SRCS} )
target_link_libraries(ThemeTest Qt5::Gui Qt5::Xml Qt5::Test )
add_test( NAME ThemeTest COMMAND ThemeTest)

set(UTIL_TEST_SRCS ../src/Util.cpp src/UtilTest.cpp)
add_executable(UtilTest ${UTIL_TEST_SRCS} )
target_link_libraries(UtilTest Qt5::Gui Qt5::Xml Qt5::Test )
add_test( NAME UtilTest COMMAND UtilTest)

set(SYNTAX_HIGHLIGHTER_TEST_SRCS ../src/SyntaxHighlighter.cpp ../src/LanguageParser.cpp ../src/PListParser.cpp ../src/Regexp.cpp ../src/Util.cpp ../src/Region.cpp ../src/Theme.cpp ../src/Session.cpp ../src/ThemeProvider.cpp ../src/ConfigManager.cpp ../src/Constants.cpp src/SyntaxHighlighterTest.cpp)
add_executable(SyntaxHighlighterTest ${SYNTAX_HIGHLIGHTER_TEST_SRCS} )
target_link_libraries(SyntaxHighlighterTest Qt5::Widgets Qt5::Xml Qt5::Test ${YAML_CPP_LIBRARY} ${ONIG_LIBRARY})
add_test( NAME SyntaxHighlighterTest COMMAND SyntaxHighlighterTest)

set(REGEXP_TEST_SRCS ../src/Regexp.cpp src/RegexpTest.cpp)
add_executable(RegexpTest ${REGEXP_TEST_SRCS} )
target_link_libraries(RegexpTest Qt5::Gui Qt5::Xml Qt5::Test ${YAML_CPP_LIBRARY} ${ONIG_LIBRARY})
add_test( NAME RegexpTest COMMAND RegexpTest)

set(REGION_TEST_SRCS ../src/Region.cpp src/RegionTest.cpp)
add_executable(RegionTest ${REGION_TEST_SRCS} )
target_link_libraries(RegionTest Qt5::Gui Qt5::Xml Qt5::Test ${YAML_CPP_LIBRARY} ${ONIG_LIBRARY})
add_test( NAME RegionTest COMMAND RegionTest)

set(TEXTEDITVIEWLOGIC_TEST_SRCS ../src/SyntaxHighlighter.cpp ../src/LanguageParser.cpp ../src/PListParser.cpp ../src/Regexp.cpp ../src/Util.cpp ../src/Region.cpp ../src/Theme.cpp ../src/Session.cpp ../src/ThemeProvider.cpp ../src/ConfigManager.cpp ../src/Constants.cpp ../src/TextEditViewLogic.cpp ../src/Regexp.cpp src/TextEditViewLogicTest.cpp)
add_executable(TextEditViewLogicTest ${TEXTEDITVIEWLOGIC_TEST_SRCS} )
target_link_libraries(TextEditViewLogicTest Qt5::Widgets Qt5::Xml Qt5::Test ${YAML_CPP_LIBRARY} ${ONIG_LIBRARY})
add_test( NAME TextEditViewLogicTest COMMAND TextEditViewLogicTest)
