project(node)

add_executable(${PROJECT_NAME} node_main.cc )

include_directories(SYSTEM
  ${PROJECT_SOURCE_DIR}/../vendor/node/src
)

if (APPLE)
  target_link_libraries(${PROJECT_NAME} ${NODE_LIBRARY} )

  # run install_name_tool to update the path to libnode.dylib
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND install_name_tool -change /usr/local/lib/libnode.dylib @executable_path/../Frameworks/libnode.dylib ${PROJECT_NAME})

elseif (MSVC)
  target_link_libraries(${PROJECT_NAME} NODE_LIBRARY )
endif ()

