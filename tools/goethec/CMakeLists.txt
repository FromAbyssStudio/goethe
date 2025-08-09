add_executable(goethec main.cpp story_cmds.cpp)
target_include_directories(goethec PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../sdk)
target_link_libraries(goethec PRIVATE goethe)
set_target_properties(goethec PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools)

