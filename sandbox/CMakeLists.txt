
project( sandbox )

file( GLOB_RECURSE project_source_files src/*.* )
file( GLOB_RECURSE project_header_files include/*.* )

add_executable( sandbox ${project_source_files} ${project_header_files} )

target_include_directories( sandbox PRIVATE ${GLM_INCLUDE_DIR} ${ENGINE_INCLUDE_DIR} )

target_link_libraries( sandbox PRIVATE engine )

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
  add_compile_definitions( EG_DEBUG DEBUG )
elseif( CMAKE_BUILD_TYPE STREQUAL "Release" )
  add_compile_definitions( EG_RELEASE NDEBUG )
endif()

add_compile_definitions( "MAU_MODULE_NAME=\"sandbox\"" )
