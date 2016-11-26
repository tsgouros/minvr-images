project(mvImage)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

set(mvImage_source_files
  mvImage.h
  mvImage.cpp
  mvImageApp.h
  mvImageApp.cpp
)

link_directories(${MINVR_DIR}/lib)

# We are only building a single executable.
add_executable(mvImage ${mvImage_source_files})

# Link this with MinVR
target_link_libraries(mvImage MinVR
  ${OPENGL_gl_LIBRARY}
  ${OPENGL_glu_LIBRARY}
  ${ALL_LIBS})

include_directories(
	../external/glfw-3.1.2/include/GLFW/
	../external/glm-0.9.7.1/
	../external/glew-1.13.0/include/
	..
  .
)

# Tutorial 8
add_executable(tgl
	tgl.cpp
	common/shader.cpp
	common/shader.hpp
	common/controls.cpp
	common/controls.hpp
	common/texture.cpp
	common/texture.hpp
	common/objloader.cpp
	common/objloader.hpp
	
	StandardShading.vertexshader
	StandardShading.fragmentshader
)
target_link_libraries(tgl MinVR
  ${OPENGL_LIBRARY}
  glfw
  GLEW_1130
)
                     
