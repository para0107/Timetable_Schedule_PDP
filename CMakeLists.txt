cmake_minimum_required(VERSION 3.10)
project(TimetableProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# Find Packages
find_package(MPI REQUIRED)
find_package(OpenCL)

# Include Directories
include_directories(${MPI_INCLUDE_PATH})
if(OpenCL_FOUND)
    include_directories(${OpenCL_INCLUDE_DIRS})
endif()

# Main Executable
add_executable(TimetableProject main.cpp SolverThreads.cpp SolverMPI.cpp SolverOpenCL.cpp)

# Link Libraries
target_link_libraries(TimetableProject ${MPI_CXX_LIBRARIES})
if(OpenCL_FOUND)
    target_link_libraries(TimetableProject ${OpenCL_LIBRARIES})
    add_definitions(-DENABLE_OPENCL) # Only enable OpenCL code if library is found
endif()