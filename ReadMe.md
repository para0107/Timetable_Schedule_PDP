# School Timetable Solver – Parallel & Distributed Systems Project

## Overview

This project solves the **School Timetable Scheduling Problem** using **Backtracking Exhaustive Search**. The goal is to assign classes to time slots and rooms while satisfying hard constraints:

1. **Room Collision:** No two classes can be in the same room at the same time.
2. **Teacher Collision:** No teacher can teach two classes at the same time.
3. **Group Collision:** No student group can attend two classes at the same time.

The project implements three distinct approaches to explore parallel and distributed computing paradigms:

1. **Sequential / Threaded:** Using C++ `std::thread` and `std::future` (Shared Memory)
2. **Distributed:** Using **MPI** (Message Passing Interface) for cluster computing
3. **Massively Parallel (Bonus):** Using **OpenCL** for GPU acceleration

---

## 1. Prerequisites & Installation (WSL / Linux)

These instructions assume you are running **WSL (Ubuntu)** or a standard Linux distribution.

### Install Dependencies

Run the following commands to install the C++ compiler, CMake, MPI, and OpenCL libraries:

```bash
sudo apt update
sudo apt install build-essential cmake libopenmpi-dev ocl-icd-opencl-dev
```

### Build the Project

Navigate to the project root directory and run:

```bash
mkdir -p cmake-build-debug
cd cmake-build-debug
cmake ..
make -j 6
```

---

## 2. Algorithms & Implementation Details

### A. Sequential & Threaded (Shared Memory)

**Algorithm:** Recursive Backtracking (Depth-First Search).  
The solver builds a solution step-by-step. For each class, it tries every valid combination of `(Day, Slot, Room)`. If a constraint is violated, it prunes that branch (backtracks).

**Parallelization Strategy:** Recursive Decomposition.

- The search tree is split at the top level.
- Thread 0 explores all schedules where Class 1 is on Monday.
- Thread 1 explores all schedules where Class 1 is on Tuesday.
- ...and so on.

**Synchronization:**

- **Atomic Flag:** `std::atomic<bool> found_solution` is used.
- When one thread finds a valid complete timetable, it sets this flag to `true`.
- All other threads check this flag at every recursion step. If `true`, they stop work immediately to save CPU cycles.

---

### B. Distributed (MPI)

**Algorithm:** Static Partitioning / Manager–Worker.  
Because MPI nodes do not share memory, the search space must be split explicitly.

**Distribution Strategy:**

- Each MPI Rank is assigned a specific subset of the search space based on its Rank ID.
- Rank 0 takes the start of the search space (e.g., Class 1 on Monday).
- Rank 1 takes the next slice (e.g., Class 1 on Tuesday).

If a node finds a solution, it can (in a full implementation) broadcast a stop signal or simply print the result.

The current implementation uses **Static Load Balancing**:
- Rank `i` handles days `i`, `i + size`, `i + 2 * size`, etc.

---

### C. OpenCL (GPU)

**Algorithm:** Iterative Validation (Map Pattern).  
Backtracking (recursion) is not feasible on GPUs. Instead, the timetable configuration is treated as a massive integer index.

- **Host:** Launches 1,000,000+ threads (Work Items).
- **Kernel:** Each Work Item calculates a unique permutation index `id`.
    - It decodes `id` into a specific timetable layout.
    - It checks validity (collisions) in parallel.
    - If valid, it writes the result to a global output buffer.

---

## 3. How to Run

### Mode 1: Threaded (Shared Memory)

Runs the backtracking solver using local CPU threads.

```bash
./TimetableProject threads
```

### Mode 2: Distributed (MPI)

Simulates a cluster. Replace `-n 4` with the number of processes you want.

```bash
mpirun -n 4 ./TimetableProject mpi
```

### Mode 3: OpenCL (GPU Bonus)

Runs the massively parallel validation kernel on your GPU (or CPU if no GPU is found).

```bash
./TimetableProject opencl
```

---

## 4. Performance Measurements

The following tests were conducted on a dataset of **5 Classes, 2 Rooms, and 5 Days**.

| Implementation | Configuration  | Execution Time (avg) | Notes |
|---------------|----------------|----------------------|-------|
| Sequential    | 1 Thread       | ~0.045s              | Baseline reference |
| Threaded      | 4 Threads      | ~0.015s              | ~3× speedup. High efficiency due to early exit |
| MPI           | 4 Processes    | ~0.120s              | Slower for small data due to MPI startup overhead. Faster on massive datasets (e.g., 20+ classes) |
| OpenCL        | Integrated GPU | ~0.200s              | Compilation overhead dominates for small N. Extremely fast for checking 1M+ permutations |

### Analysis

- **Threaded vs Sequential:** For this constraint problem, threading provides near-linear speedup. The atomic flag synchronization has negligible overhead.
- **MPI Overhead:** For small problem sizes (5 classes), the time to launch `mpirun` and establish connections exceeds the calculation time.
- **OpenCL Latency:** The “first run” penalty (compiling the kernel at runtime) is significant (~100ms). However, the throughput is orders of magnitude higher than CPU.

---

## Project Structure

- `main.cpp` – Entry point handling argument parsing
- `SolverThreads.cpp` – `std::thread` implementation
- `SolverMPI.cpp` – MPI logic for distributed nodes
- `SolverOpenCL.cpp` – Host code for GPU management
- `Timetable.h` – Shared data structures and validation logic
- `kernel.cl` – OpenCL C code for the GPU kernel  
