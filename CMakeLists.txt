cmake_minimum_required(VERSION 3.10.2)
project(portals_debug)

##############################################################################
# ENABLE WHEN DEBUG
# set(CMAKE_BUILD_TYPE Debug)
##############################################################################

set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY CXX_STANDARD 17)
set(OpenGL_GL_PREFERENCE GLVND)

#-----------------------------------------------------------------------------
# Set here your code locations
set(ALL_DIR ~/testing)
set(MY_STB_DIR ${ALL_DIR}/my_stb)                 # https://github.com/nothings/stb + my_stb (see below)
                                                  # https://github.com/optozorax/my_stb
set(JSON_DIR ${ALL_DIR}/json)                     # https://github.com/nlohmann/json
set(CLIPPER_DIR ${ALL_DIR}/clipper/cpp)           # https://sourceforge.net/projects/polyclipping/
set(SPACE_OBJECTS_DIR ${ALL_DIR}/space_objects)   # https://github.com/optozorax/space_objects
set(PORTALS_OPENGL_DIR ${ALL_DIR}/portals_opengl) # https://github.com/optozorax/portals_opengl
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Install freeglut on linux
# Run: sudo apt install libglew-dev libglm-dev libxi-dev libxrandr-dev libglu1-mesa-dev mesa-common-dev libx11-dev
# Download: https://sourceforge.net/projects/freeglut/
# Run in freeglut folder: cmake -G "Unix Makefiles"
# Run: make install
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# my_stb lib:
# my_stb/
#   include/
#     stb_image.h       # from original repo
#     stb_image_write.h # from original repo
#   src/
#     stb_image.cpp:
#       #define STB_IMAGE_IMPLEMENTATION
#       #define STBI_MSC_SECURE_CRT
#       #include <stb_image.h>
#     stb_image_write.cpp:
#       #define STB_IMAGE_WRITE_IMPLEMENTATION
#       #define STBI_MSC_SECURE_CRT
#       #include <stb_image_write.h>
# It maked because I used stb in many projects and got link errors
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS}  ${GLEW_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})
link_libraries(${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${GLUT_LIBRARIES})
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
add_library(my_stb STATIC 
	${MY_STB_DIR}/src/stb_image.cpp 
	${MY_STB_DIR}/src/stb_image_write.cpp

	${MY_STB_DIR}/include/stb_image.h
	${MY_STB_DIR}/include/stb_image_write.h
)
include_directories(${MY_STB_DIR}/include)
link_libraries(my_stb)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
add_library(json STATIC 
	${JSON_DIR}/single_include/nlohmann/json.hpp
)
include_directories(${JSON_DIR}/single_include/nlohmann/)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
add_library(clipper STATIC 
	${CLIPPER_DIR}/clipper.cpp 

	${CLIPPER_DIR}/clipper.hpp
)
include_directories(${CLIPPER_DIR}/)
link_libraries(clipper)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
add_library(portals_opengl STATIC 
	${PORTALS_OPENGL_DIR}/src/prtl_vis/fragment.cpp
	${PORTALS_OPENGL_DIR}/src/prtl_vis/framebuffer.cpp
	${PORTALS_OPENGL_DIR}/src/prtl_vis/opengl_common.cpp
	${PORTALS_OPENGL_DIR}/src/prtl_vis/portal_window.cpp
	${PORTALS_OPENGL_DIR}/src/prtl_vis/plane.cpp
	${PORTALS_OPENGL_DIR}/src/prtl_vis/scene_reader.cpp
	${PORTALS_OPENGL_DIR}/src/prtl_vis/shader.cpp

	${PORTALS_OPENGL_DIR}/include/prtl_vis/fragment.h
	${PORTALS_OPENGL_DIR}/include/prtl_vis/opengl_common.h
	${PORTALS_OPENGL_DIR}/include/prtl_vis/plane.h
	${PORTALS_OPENGL_DIR}/include/prtl_vis/scene_reader.h
	${PORTALS_OPENGL_DIR}/include/prtl_vis/shader.h
	${PORTALS_OPENGL_DIR}/include/prtl_vis/framebuffer.h
)
include_directories(${PORTALS_OPENGL_DIR}/include/)
link_libraries(portals_opengl)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
add_library(space_objects STATIC 
	${SPACE_OBJECTS_DIR}/src/coordinates2.cpp 
	${SPACE_OBJECTS_DIR}/src/coordinates3.cpp

	${SPACE_OBJECTS_DIR}/include/spob/spob.h
	${SPACE_OBJECTS_DIR}/include/spob/coordinates3.h
	${SPACE_OBJECTS_DIR}/include/spob/vector2.h
	${SPACE_OBJECTS_DIR}/include/spob/coordinates2.h
	${SPACE_OBJECTS_DIR}/include/spob/common.h
	${SPACE_OBJECTS_DIR}/include/spob/vector3.h
	${SPACE_OBJECTS_DIR}/include/spob/spob2glm.h
)
include_directories(${SPACE_OBJECTS_DIR}/include)
link_libraries(space_objects)
#-----------------------------------------------------------------------------

# Replace DEBUG to RELEASE if needed

add_executable(portals_viewer_RELEASE ${PORTALS_OPENGL_DIR}/main.cpp)