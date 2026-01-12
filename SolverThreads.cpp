#include "Timetable.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <chrono>

std::mutex mtx;
std::atomic<bool> found(false);

void backtrack(int sessionIdx, std::vector<int> schedule,
                      const Problem& p, const FlattenedSchedule& fs) {
    if (found) return;

    if (sessionIdx == fs.totalSessions) {
        std::lock_guard<std::mutex> lock(mtx);
        if (!found) {
            found = true;
            printComplexSchedule(schedule, p, fs);
        }
        return;
    }

    int totalSlots = p.numDays * p.slotsPerDay;

    for (int slot = 0; slot < totalSlots; ++slot) {
        if (isValid(schedule, sessionIdx, slot, p, fs)) {
            schedule[sessionIdx] = slot;
            backtrack(sessionIdx + 1, schedule, p, fs);
            if (found) return;
        }
    }
}

void solveThreadsComplex(Problem p, int numThreads) {
    FlattenedSchedule fs = flatten(p);
    int totalSlots = p.numDays * p.slotsPerDay;

    std::cout << "Running Threads (" << numThreads << ")...\n";
    std::cout << "Total Sessions to Schedule: " << fs.totalSessions << "\n";

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> workers;

    for (int t = 0; t < numThreads; ++t) {
        workers.emplace_back([t, numThreads, p, fs, totalSlots]() {
            for (int slot = t; slot < totalSlots; slot += numThreads) {
                if (found) return;

                std::vector<int> schedule(fs.totalSessions);
                schedule[0] = slot;

                backtrack(1, schedule, p, fs);
            }
        });
    }

    for (auto& t : workers) {
        if(t.joinable()) t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";

    if(!found) std::cout << "No solution found.\n";
}