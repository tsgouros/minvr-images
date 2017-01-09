project(mvImage)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})

link_directories(${MINVR_DIR}/lib)

include_directories(
	../external/glm-0.9.7.1/
	../external/glew-1.13.0/include/
	..
  .
)

# tgm
add_executable(tgm
	tgm.cpp
  mvShape.cpp
  mvShape.h
  vecTypes.h
	common/shader.cpp
	common/shader.h
	common/texture.cpp
	common/texture.h
	common/objloader.cpp
	common/objloader.h
  tinyxml2.h
  tinyxml2.cpp
	StandardShading.vertexshader
	StandardShading.fragmentshader
)

target_link_libraries(tgm
  MinVR
  ${PNG_LIBRARIES}
  ${OPENGL_LIBRARY}
  GLEW_1130
  ${ALL_LIBS}
)

