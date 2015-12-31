project(node)

if (APPLE)
  add_executable(${PROJECT_NAME} node_main.cc )
else (APPLE)
  add_executable(${PROJECT_NAME} WIN32 node_main.cc )
endif (APPLE)

target_link_libraries(${PROJECT_NAME} ${NODE_LIBRARY} )
