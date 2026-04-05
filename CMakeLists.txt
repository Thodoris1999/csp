cmake_minimum_required(VERSION 3.20)
project(csp VERSION 0.1.0)

# Find required tools and libraries
find_program(GLSLC_EXECUTABLE glslc REQUIRED)
find_program(SPIRV_CROSS_EXECUTABLE spirv-cross REQUIRED)
find_package(Vulkan REQUIRED)

# Build the C++ code generator tool first so csp.cmake can reference it
add_subdirectory(tools)

# Include the csp cmake module
include(cmake/csp.cmake)

enable_testing()
add_subdirectory(test)
