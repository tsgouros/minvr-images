project(minvr-images)

# CMakeLists files in this project can refer to the root source directory of the 
# project as ${MINVR_SOURCE_DIR} and to the root binary directory of the project 
# as ${MINVR_BINARY_DIR}.

cmake_minimum_required (VERSION 2.8.2) 

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE 
      Debug CACHE STRING
      "The type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." 
      FORCE)
endif()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX 
      "${${PROJECT_NAME}_SOURCE_DIR}/build/install" CACHE PATH 
      "default install path" 
      FORCE)
endif()

# To distingusih between debugging, release, and other builds, we'll
# add a postfix to the name of the lib or exe that we generate.

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX "d")
endif()
set(CMAKE_RELEASE_POSTFIX "")
set(CMAKE_RELWITHDEBINFO_POSTFIX "rd")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DMinVR_DEBUG")

set(MINVR_DIR /Users/tomfool/tech/16/yurt/MinVR2-master/build/install)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${MINVR_DIR}/include")
message(${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -I${MINVR_DIR}/include")

# Build output directories.
make_directory(${CMAKE_BINARY_DIR}/bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  # Linux specific code
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC") 
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # Apple-specific code
  set(CMAKE_CXX_FLAGS "-DOSX")
endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if (WIN32)
  # Windows-specific
endif (WIN32)

#enable_testing()

add_subdirectory(external)

set(img_src_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(src)
