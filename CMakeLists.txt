cmake_minimum_required(VERSION 3.15)
project(Lucy)

set(CMAKE_CXX_STANDARD 20)

# Include the header files from the Include/ directory
include_directories(include)

# Add all your .cc files here
add_executable(Lucy main.cpp  blur_image.cc DeconvolutionUtils.cc  deconvolution.cc)
