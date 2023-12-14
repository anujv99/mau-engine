cmake_minimum_required( VERSION 3.22 )

project(mau)

# set( CMAKE_BUILD_TYPE Debug )
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )

# ---------------------- THIRD PARTY ----------------------- #

# static libraries #
set( GLFW_INCLUDE_DIR "thirdparty/glfw/include" )
set( VMA_INCLUDE_DIR "thirdparty/vma/include" )
set( IMGUI_INCLUDE_DIR "thirdparty/imgui" )
set( TRACY_INCLUDE_DIR "thirdparty/tracy/public" )
set( ASSIMP_INCLUDE_DIR "thirdparty/assimp/include" )

# header only libraries # 
set( GLM_INCLUDE_DIR "thirdparty/glm/include"  )
set( STB_INCLUDE_DIR "thirdparty/stb" )
set( ENTT_INCLUDE_DIR "thirdparty/entt/include" )

if ( NOT IS_ABSOLUTE ${GLFW_INCLUDE_DIR} )
  get_filename_component( GLFW_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/${GLFW_INCLUDE_DIR}" ABSOLUTE )
endif()

if ( NOT IS_ABSOLUTE ${VMA_INCLUDE_DIR} )
  get_filename_component( VMA_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/${VMA_INCLUDE_DIR}" ABSOLUTE )
endif()

if ( NOT IS_ABSOLUTE ${IMGUI_INCLUDE_DIR} )
  get_filename_component( IMGUI_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/${IMGUI_INCLUDE_DIR}" ABSOLUTE )
endif()

if ( NOT IS_ABSOLUTE ${TRACY_INCLUDE_DIR} )
  get_filename_component( TRACY_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/${TRACY_INCLUDE_DIR}" ABSOLUTE )
endif()

if ( NOT IS_ABSOLUTE ${ASSIMP_INCLUDE_DIR} )
  get_filename_component( ASSIMP_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/${ASSIMP_INCLUDE_DIR}" ABSOLUTE )
endif()

if ( NOT IS_ABSOLUTE ${GLM_INCLUDE_DIR} )
  get_filename_component( GLM_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/${GLM_INCLUDE_DIR}" ABSOLUTE )
endif()

if ( NOT IS_ABSOLUTE ${STB_INCLUDE_DIR} )
  get_filename_component( STB_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/${STB_INCLUDE_DIR}" ABSOLUTE )
endif()

if ( NOT IS_ABSOLUTE ${ENTT_INCLUDE_DIR} )
  get_filename_component( ENTT_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/${ENTT_INCLUDE_DIR}" ABSOLUTE )
endif()

set( GLFW_BUILD_EXAMPLES OFF )
set( GLFW_BUILD_TESTS OFF )
set( GLFW_BUILD_DOCS OFF )
add_subdirectory( thirdparty/glfw )
add_subdirectory( thirdparty/vma )
add_subdirectory( thirdparty/imgui )
add_subdirectory( thirdparty/tracy )

set( BUILD_SHARED_LIBS OFF )
add_subdirectory( thirdparty/assimp )

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
