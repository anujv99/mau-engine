cmake_minimum_required( VERSION 3.7 )

project(mau)

set( CMAKE_BUILD_TYPE Debug )
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

# ---------------------- THIRD PARTY ----------------------- #

# static libraries #
set( GLFW_INCLUDE_DIR "thirdparty/glfw/include" )

# header only libraries # 
set( GLM_INCLUDE_DIR  "thirdparty/glm/include"  )

if ( NOT IS_ABSOLUTE ${GLFW_INCLUDE_DIR} )
  get_filename_component( GLFW_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/${GLFW_INCLUDE_DIR}" ABSOLUTE )
endif()

if ( NOT IS_ABSOLUTE ${GLM_INCLUDE_DIR} )
  get_filename_component( GLM_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/${GLM_INCLUDE_DIR}" ABSOLUTE )
endif()

add_subdirectory( thirdparty/glfw )

# ---------------------------------------------------------- #

# ---------------------- ENGINE LIBS ----------------------- #

set( ENGINE_INCLUDE_DIR "engine/include" )

if ( NOT IS_ABSOLUTE ${ENGINE_INCLUDE_DIR} )
  get_filename_component( ENGINE_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/${ENGINE_INCLUDE_DIR}" ABSOLUTE )
endif()

add_subdirectory( engine )

# ---------------------------------------------------------- #

# --------------------- EXECUTABLE ------------------------- #

add_subdirectory( sandbox )

# ---------------------------------------------------------- #
