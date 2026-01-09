#include "Timetable.h"
#include <mpi.h>
#include <cstring>

std::vector<ClassObject> create_dataset() {
    return {
                {0, 101, 201, "Math"},
                {1, 102, 201, "Physics"},
                {2, 101, 202, "CS"},
                {3, 103, 203, "English"},
                {4, 102, 203, "Sport"}
    };
}

int main(int argc, char** argv) {
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    std::vector<ClassObject> classes = create_dataset();
    std::string mode = "threads";

    if (argc > 1) {
        mode = argv[1];
    }

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        std::cout << "=== School Timetable Solver ===\n";
        std::cout << "Mode: " << mode << "\n";
        std::cout << "Classes: " << classes.size() << ", Rooms: " << ROOMS << "\n";
    }

    if (mode == "threads") {
        if (rank == 0) solve_threaded(classes);
    }
    else if (mode == "mpi") {
        solve_mpi(argc, argv, classes);
    }
    else if (mode == "opencl") {
        if (rank == 0) solve_opencl(classes);
    }
    else {
        if (rank == 0) std::cout << "Unknown mode. Use: threads | mpi | opencl\n";
    }

    MPI_Finalize();
    return 0;
}