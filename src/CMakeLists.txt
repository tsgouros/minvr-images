project(mvImage)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
message("cmake module path: >>" ${CMAKE_MODULE_PATH} "<<")
find_package(OpenGL REQUIRED)

find_package(PNG 1.4 REQUIRED MODULE)

set(GLM_INCLUDE_DIR /users/tsgouros/projects/glew-and-friends/glm-0.9.7.1)

set(GLEW_INCLUDE_DIR    
  /users/tsgouros/projects/glew-and-friends/glew-1.13.0/include)
set(GLEW_LIB_1130 
  /users/tsgouros/projects/glew-and-friends/glew-1.13.0/lib/libGLEW.a)

include_directories(
  ${OPENGL_INCLUDE_DIR}
  ${PNG_INCLUDE_DIRS}
  ${MINVR_INCLUDE_DIR}
  ${GLM_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIR}
  )

# tgm
add_executable(tgm
  tgm.cpp
  mvShape.cpp
  mvShape.h
  vecTypes.h
  shader.cpp
  shader.h
  texture.cpp
  texture.h
  objloader.cpp
  objloader.h
  tinyxml2.h
  tinyxml2.cpp
)

target_link_libraries(tgm
  ${MINVR_LIBRARY}
  ${PNG_LIBRARIES}
  ${OPENGL_LIBRARY}
  ${GLEW_LIB_1130}
  ${ALL_LIBS}
)

