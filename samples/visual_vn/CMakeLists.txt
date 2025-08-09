if(NOT GOETHE_BACKEND_SDL3)
  message(STATUS "samples/visual_vn: Skipped (GOETHE_BACKEND_SDL3=OFF)")
  return()
endif()

if(GOETHE_SDL3_HEADLESS)
  message(STATUS "samples/visual_vn: Skipped for headless SDL build (GOETHE_SDL3_HEADLESS=ON)")
  return()
endif()

add_executable(visual_vn main.cpp)
target_link_libraries(visual_vn PRIVATE goethe SDL3::SDL3)
target_include_directories(visual_vn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../sdk)
set_target_properties(visual_vn PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/samples/visual_vn)

add_custom_command(TARGET visual_vn POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_CURRENT_SOURCE_DIR}/assets
          ${CMAKE_BINARY_DIR}/samples/visual_vn/assets)


