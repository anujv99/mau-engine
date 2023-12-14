
project( engine )

# find vulkan headers and lib
find_package( Vulkan REQUIRED ) # Vulkan_INCLUDE_DIR, Vulkan_LIBRARIES
find_package( Shaderc REQUIRED ) # Shaderc_INCLUDE_DIR, Shaderc_combined_LIBRARIES, Shaderc_shared_LIBRARIES

file( GLOB_RECURSE project_source_files src/*.* )
file( GLOB_RECURSE project_header_files include/*.* )

add_library( engine STATIC ${project_source_files} ${project_header_files} )

target_include_directories( engine PRIVATE "include" "src"
  ${GLFW_INCLUDE_DIR}
  ${VMA_INCLUDE_DIR}
  ${GLM_INCLUDE_DIR}
  ${IMGUI_INCLUDE_DIR}
  ${TRACY_INCLUDE_DIR}
  ${ASSIMP_INCLUDE_DIR}
  ${STB_INCLUDE_DIR}
  ${Vulkan_INCLUDE_DIR}
  ${Shaderc_INCLUDE_DIR}
)

target_link_libraries( engine PRIVATE
  glfw
  VulkanMemoryAllocator
  imgui
  Tracy::TracyClient
  assimp
  ${Vulkan_LIBRARIES}
  ${Shaderc_shared_LIBRARIES}
)

# add asset folder path
set( ASSET_FOLDER_PATH  "assets"  )
if ( NOT IS_ABSOLUTE ${ASSET_FOLDER_PATH} )
  get_filename_component( ASSET_FOLDER_PATH "${CMAKE_CURRENT_LIST_DIR}/${ASSET_FOLDER_PATH}" ABSOLUTE )
endif()

file( RELATIVE_PATH MAU_ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${ASSET_FOLDER_PATH} )
add_compile_definitions( "MAU_ASSET_FOLDER=\"${MAU_ASSET_FOLDER}/\"" )


if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
  add_compile_definitions( MAU_DEBUG DEBUG )
elseif( CMAKE_BUILD_TYPE STREQUAL "Release" )
  add_compile_definitions( MAU_RELEASE NDEBUG )
endif()

add_compile_definitions( "MAU_MODULE_NAME=\"engine\"" )
