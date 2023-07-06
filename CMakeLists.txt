cmake_minimum_required(VERSION 3.15)
project(Lucy)

set(CMAKE_CXX_STANDARD 20)  # Enable C++20 standard

# Include the header files from the Include/ directory
include_directories(include)

# Find GTK
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# Add the GTK include directories to the project
include_directories(${GTK3_INCLUDE_DIRS})

# Add the GTK libraries to the project
link_directories(${GTK3_LIBRARY_DIRS})
link_libraries(${GTK3_LIBRARIES})


# Add all your .cc files here
add_executable(Lucy main.cpp  blur_image.cc DeconvolutionUtils.cc  deconvolution.cc ImageViewer.cc)
target_link_libraries(Lucy ${GTK3_LIBRARIES})