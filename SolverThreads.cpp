#include "Timetable.h"
#include <thread>
#include <future>
#include <atomic>

std::atomic<bool> found_solution(false); // Stop other threads if one succeeds

// Recursive Backtracking Function
bool backtrack(TimetableState current_state, std::vector<int> remaining_class_indices, const std::vector<ClassObject>& all_classes) {
    if (found_solution) return true; // Optimization: stop if another thread finished

    if (remaining_class_indices.empty()) {
        if (!found_solution.exchange(true)) { // Only print once
            current_state.print(all_classes);
        }
        return true;
    }

    int next_class_idx = remaining_class_indices.back();
    remaining_class_indices.pop_back();

    // Try all combinations
    for (int d = 0; d < DAYS; ++d) {
        for (int i = 0; i < INTERVALS; ++i) {
            for (int r = 0; r < ROOMS; ++r) {
                if (found_solution) return true;

                Assignment new_assign = {next_class_idx, d, i, r};
                
                if (current_state.isValid(new_assign, all_classes)) {
                    TimetableState next_state = current_state;
                    next_state.assignments.push_back(new_assign);
                    if (backtrack(next_state, remaining_class_indices, all_classes)) return true;
                }
            }
        }
    }
    return false;
}

void solve_threaded(const std::vector<ClassObject>& classes) {
    std::cout << "[Threads] Starting Solver with " << std::thread::hardware_concurrency() << " threads...\n";
    found_solution = false;

    // Prepare indices
    std::vector<int> indices;
    for(size_t i=0; i<classes.size(); ++i) indices.push_back(i);
    
    // We pull the first class out to branch on it
    int first_class = indices.back();
    indices.pop_back();

    std::vector<std::future<void>> futures;

    // Parallelize based on the DAY of the first class
    for (int d = 0; d < DAYS; ++d) {
        futures.push_back(std::async(std::launch::async, [=, &classes]() {
            for (int i = 0; i < INTERVALS; ++i) {
                for (int r = 0; r < ROOMS; ++r) {
                    if (found_solution) return;

                    TimetableState root_state;
                    root_state.assignments.push_back({first_class, d, i, r});

                    backtrack(root_state, indices, classes);
                }
            }
        }));
    }

    for (auto& f : futures) f.wait();
    if (!found_solution) std::cout << "No solution found.\n";
}