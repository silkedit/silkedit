project(node)

add_executable(${PROJECT_NAME} node_main.cc )

include_directories(SYSTEM include src ${Boost_INCLUDE_DIRS} include/node)
if (APPLE)
  target_link_libraries(${PROJECT_NAME} ${NODE_LIBRARY} )
elseif (MSVC)
  target_link_libraries(${PROJECT_NAME} NODE_LIBRARY )
endif ()
