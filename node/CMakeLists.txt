project(node)

add_executable(${PROJECT_NAME} node_main.cc )

include_directories(SYSTEM include src ${Boost_INCLUDE_DIRS} include/node)
target_link_libraries(${PROJECT_NAME} NODE_LIBRARY )
