project(mvImage)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})

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
  ${PNG_LIBRARIES}
  ${ALL_LIBS})

include_directories(
	../external/glfw-3.1.2/include/GLFW/
	../external/glm-0.9.7.1/
	../external/glew-1.13.0/include/
	..
  .
)

# tgl
add_executable(tgl
	tgl.cpp
  mvShape.cpp
  mvShape.h
	common/shader.cpp
	common/shader.h
	common/controls.cpp
	common/controls.h
	common/texture.cpp
	common/texture.h
	common/objloader.cpp
	common/objloader.h
  tinyxml2.h
  tinyxml2.cpp
	
	StandardShading.vertexshader
	StandardShading.fragmentshader
)
target_link_libraries(tgl MinVR
  ${OPENGL_LIBRARY}
  ${PNG_LIBRARIES}
  glfw
  GLEW_1130
  ${ALL_LIBS}
)
                     
