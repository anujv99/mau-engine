
project( engine )

# find vulkan headers and lib
find_package( Vulkan REQUIRED ) # Vulkan_INCLUDE_DIR, Vulkan_LIBRARIES

file( GLOB_RECURSE project_source_files src/*.* )
file( GLOB_RECURSE project_header_files include/*.* )

add_library( engine STATIC ${project_source_files} ${project_header_files} )

target_include_directories( engine PRIVATE "include" ${GLFW_INCLUDE_DIR} ${GLM_INCLUDE_DIR} ${Vulkan_INCLUDE_DIR} )

target_link_libraries( engine PRIVATE glfw ${Vulkan_LIBRARIES} )

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
  add_compile_definitions( MAU_DEBUG DEBUG )
elseif( CMAKE_BUILD_TYPE STREQUAL "Release" )
  add_compile_definitions( MAU_RELEASE NDEBUG )
endif()

add_compile_definitions( "MAU_MODULE_NAME=\"engine\"" )
