# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -Wextra")

set(SK_SOURCES main.cpp mainWindow.cpp viEditView.cpp)

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)

add_executable(sk MACOSX_BUNDLE ${SK_SOURCES})

# Use the Widgets module from Qt 5.
target_link_libraries(sk Qt5::Widgets)
