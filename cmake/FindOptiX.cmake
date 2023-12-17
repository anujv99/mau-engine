
set( _OptiX_hint_include_dir_search_paths "$ENV{OptiX_INSTALL_DIR}/include" )

find_path( OptiX_INCLUDE_DIR NAMES optix.h HINTS ${_OptiX_hint_include_dir_search_paths} )

if ( OptiX_INCLUDE_DIR )
  set( OptiX_FOUND TRUE )
else()
  set( OptiX_FOUND FALSE )
endif()

