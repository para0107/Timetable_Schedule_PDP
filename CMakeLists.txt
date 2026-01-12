cmake_minimum_required(VERSION 3.20)
project(ProjectPDP)

set(CMAKE_CXX_STANDARD 17)

find_package(MPI REQUIRED)
find_package(OpenCL REQUIRED)
find_package(Threads REQUIRED)

set(SHARED_HEADERS Timetable.h)

add_executable(timetable_local
        main.cpp
        SolverThreads.cpp
        SolverOpenCL.cpp
        ${SHARED_HEADERS}
)
target_link_libraries(timetable_local PRIVATE OpenCL::OpenCL Threads::Threads)

add_executable(timetable_mpi
        SolverMPI.cpp
        ${SHARED_HEADERS}
)

add_custom_command(TARGET timetable_local POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/kernel.cl
        ${CMAKE_BINARY_DIR}/kernel.cl
)

target_link_libraries(timetable_mpi PRIVATE MPI::MPI_CXX)