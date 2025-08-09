add_executable(hello_vn host.cpp)
target_link_libraries(hello_vn PRIVATE goethe)
target_include_directories(hello_vn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../sdk)
set_target_properties(hello_vn PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/samples/hello_vn)

