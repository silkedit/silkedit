cmake_minimum_required(VERSION 2.8.11)

project(sk)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SK_SOURCES main.cpp mainWindow.cpp viEditView.cpp)
set(SK_HEADERS mainWindow.h viEditView.h)

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)

# Tell CMake to create the helloworld executable
add_executable(sk MACOSX_BUNDLE ${SK_SOURCES})

# Use the Widgets module from Qt 5.
target_link_libraries(sk Qt5::Widgets)
