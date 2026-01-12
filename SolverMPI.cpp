#include <mpi.h>
#include <vector>
#include <iostream>
#include "Timetable.h"


void backtrackMPI(int sessionIdx, std::vector<int> schedule,
                  const Problem& p, const FlattenedSchedule& fs, int rank) {
    if (sessionIdx == fs.totalSessions) {
        std::cout << "[Rank " << rank << "] SOLUTION FOUND!\n";
        printComplexSchedule(schedule, p, fs);
        MPI_Abort(MPI_COMM_WORLD, 0);
        return;
    }

    int totalSlots = p.numDays * p.slotsPerDay;
    for (int slot = 0; slot < totalSlots; ++slot) {
        if (isValid(schedule, sessionIdx, slot, p, fs)) {
            schedule[sessionIdx] = slot;
            backtrackMPI(sessionIdx + 1, schedule, p, fs, rank);
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    Problem p;

    p.numDays = 5;
    p.slotsPerDay = 4;
    // Course 0: T1, G1, 3 Sessions
    p.courses.push_back({0, 1, 1, 3});
    // Course 1: T2, G1, 2 Sessions (Same group as above -> Conflict if same slot)
    p.courses.push_back({1, 2, 1, 2});

    FlattenedSchedule fs = flatten(p);
    int totalSlots = p.numDays * p.slotsPerDay;

    double start = MPI_Wtime();

    for (int slot = 0; slot < totalSlots; ++slot) {
        if (slot % size == rank) {
            std::vector<int> schedule(fs.totalSessions);
            schedule[0] = slot;
            backtrackMPI(1, schedule, p, fs, rank);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == 0) std::cout << "MPI Finished.\n";

    MPI_Finalize();
    return 0;
}