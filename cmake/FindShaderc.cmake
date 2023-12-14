
set( _Shaderc_hint_library_search_paths "$ENV{VULKAN_SDK}/lib" )
set( _Shaderc_hit_include_dir_search_paths "$ENV{VULKAN_SDK}/include" )

find_library( Shaderc_combined_LIBRARIES NAMES shaderc_combined HINTS ${_Shaderc_hint_library_search_paths} )
find_library( Shaderc_shared_LIBRARIES NAMES shaderc_shared HINTS ${_Shaderc_hint_library_search_paths} )
find_path( Shaderc_INCLUDE_DIR NAMES shaderc/shaderc.h HINTS ${_Shaderc_hit_include_dir_search_paths}  )

if( Shaderc_combined_LIBRARIES AND Shaderc_INCLUDE_DIR )
  set( Shaderc_combined_FOUND TRUE )
else()
  set( Shaderc_combined_FOUND FALSE )
endif()
