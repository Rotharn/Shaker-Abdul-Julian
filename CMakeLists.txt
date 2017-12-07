cmake_minimum_required(VERSION 2.6)

# Name of project
project(EvacAttack)

# Specify project files: header files and source files
set(HDRS
    Enemy.h helicopter.h asteroid.h camera.h game.h resource.h resource_manager.h scene_graph.h scene_node.h
 shader_attribute.h)
 
set(SRCS
    Enemy.cpp helicopter.cpp asteroid.cpp camera.cpp game.cpp main.cpp resource.cpp resource_manager.cpp scene_graph.cpp scene_node.cpp shader_attribute.cpp material_vp.glsl material_fp.glsl shiny_blue_fp.glsl 
	shiny_blue_vp.glsl toon_fp.glsl toon_vp.glsl texture_fp.glsl texture_vp.glsl toon_heli_vp.glsl 
	toon_heli_fp.glsl missile_vp.glsl fire_vp.glsl fire_fp.glsl fire_gp.glsl missile_fp.glsl missile_gp.glsl
)


# Add path name to configuration file
configure_file(path_config.h.in path_config.h)

# Add executable based on the source files
add_executable(EvacAttack ${HDRS} ${SRCS})

# Require OpenGL library
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
target_link_libraries(EvacAttack ${OPENGL_gl_LIBRARY})

# Other libraries needed
set(LIBRARY_PATH "" CACHE PATH "Folder with GLEW, GLFW, GLM, and SOIL libraries")
include_directories(${LIBRARY_PATH}/include)
if(NOT WIN32)
    find_library(GLEW_LIBRARY GLEW)
    find_library(GLFW_LIBRARY glfw)
    find_library(SOIL_LIBRARY SOIL)
elseif(WIN32)
    find_library(GLEW_LIBRARY glew32s HINTS ${LIBRARY_PATH}/lib)
    find_library(GLFW_LIBRARY glfw3 HINTS ${LIBRARY_PATH}/lib)
    find_library(SOIL_LIBRARY SOIL HINTS ${LIBRARY_PATH}/lib)
endif(NOT WIN32)
target_link_libraries(EvacAttack ${GLEW_LIBRARY})
target_link_libraries(EvacAttack ${GLFW_LIBRARY})
target_link_libraries(EvacAttack ${SOIL_LIBRARY})

# The rules here are specific to Windows Systems
if(WIN32)
    # Avoid ZERO_CHECK target in Visual Studio
    set(CMAKE_SUPPRESS_REGENERATION TRUE)
 
    # This will use the proper libraries in debug mode in Visual Studio
    set_target_properties(EvacAttack PROPERTIES DEBUG_POSTFIX _d)
endif(WIN32)
