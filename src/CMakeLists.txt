project(mvImage)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

set(mvImage_source_files
  mvImage.h
  mvImage.cpp
)

link_directories(${MINVR_DIR}/lib)

# We are only building a single executable.
add_executable(mvImage ${mvImage_source_files})

# Link this with MinVR
target_link_libraries(mvImage MinVR
                              ${OPENGL_gl_LIBRARY}
                              ${OPENGL_glu_LIBRARY}
                              ${ALL_LIBS})
