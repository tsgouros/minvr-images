project(mvImage)

set(mvImage_source_files
  mvImage.h
  mvImage.cpp
)

link_directories(${MINVR_DIR}/lib)

# We are only building a single executable.
add_executable(mvImage ${mvImage_source_files})

# Link this with MinVR
target_link_libraries(mvImage MinVR ${ALL_LIBS})
