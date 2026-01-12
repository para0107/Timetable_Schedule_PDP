# Timetable Schedule Generator

A parallel computing solution for automated timetable generation using constraint satisfaction. This project demonstrates three parallelization approaches: **C++ Threads**, **OpenCL (GPU)**, and **MPI (Distributed)**.

## Overview

This application generates valid course schedules by exploring the solution space while enforcing the following constraints:

- **Teacher Conflict**: A teacher cannot teach two courses at the same time slot
- **Group Conflict**: A student group cannot attend two courses at the same time slot
- **Same-Day Constraint**: Multiple sessions of the same course must be scheduled on different days

## Features

- Interactive CLI for problem definition
- Three parallel solving strategies
- Configurable thread count for CPU parallelism
- GPU acceleration via OpenCL
- Distributed computing support via MPI

## Project Structure

```
ProjectPDP/
├── main.cpp              # CLI entry point
├── Timetable.h           # Data structures and utility functions
├── SolverThreads.cpp     # Multi-threaded backtracking solver
├── SolverOpenCL.cpp      # GPU-accelerated brute-force solver
├── SolverMPI.cpp         # MPI distributed solver
├── kernel.cl             # OpenCL kernel for GPU execution
├── sample_input.txt      # Example test inputs
└── CMakeLists.txt        # Build configuration
```

## Requirements

- **CMake** 3.20+
- **C++17** compatible compiler
- **OpenCL** runtime and development headers
- **MPI** implementation (OpenMPI or MPICH)
- **pthreads** (included on Linux/macOS)

### Platform-Specific Setup

**Ubuntu/Debian:**

```
sudo apt install cmake build-essential opencl-headers ocl-icd-opencl-dev libopenmpi-dev
```

**Windows (WSL2 recommended):**

```
sudo apt install cmake build-essential opencl-headers ocl-icd-opencl-dev libopenmpi-dev
```

## Building

```
mkdir build && cd build
cmake ..
make
```

This produces two executables:
- `timetable_local` — Threads and OpenCL solver
- `timetable_mpi` — MPI distributed solver

## Usage

### Interactive Mode

```
./timetable_local
```

Follow the prompts to enter:
1. Number of days
2. Slots per day
3. Number of courses
4. For each course: Teacher ID, Group ID, Sessions per week
5. Solver method (Threads or OpenCL)

### Piped Input

```
echo "5 4 3  1 1 2  2 1 2  1 2 2  1 4" | ./timetable_local
```

**Input format:**

```
<days> <slots> <numCourses> <T1 G1 S1> <T2 G2 S2> ... <method> [threads]
```

### MPI Solver

```
mpirun -np 4 ./timetable_mpi
```

> **Note:** The MPI solver currently uses hardcoded input. Modify `SolverMPI.cpp` to customize.

## Sample Inputs

| Difficulty | Input | Search Space |
|------------|-------|--------------|
| Easy Thread | `5 4 3  1 1 2  2 1 2  1 2 2  1 4` | 20⁶ |
| Hard Thread | `5 6 7  1 1 3  1 2 3  2 1 3  2 2 3  3 3 2  3 1 2  4 2 2  1 8` | 30¹⁸ |
| Easy OpenCL | `3 3 3  1 1 1  2 2 1  3 3 1  2` | 729 |
| Medium OpenCL | `5 4 3  1 1 2  2 1 2  1 2 2  2` | 64M |
| Hard OpenCL | `5 4 6  1 1 2  1 2 2  2 1 2  2 2 2  3 3 2  3 4 2  2` | 4×10¹⁵ |

## Experiments & Results

### Experiment 1: Easy OpenCL (Valid Solution)

**Input:**

```
echo "5 4 3  1 1 2  2 1 2  1 2 2  2" | ./timetable_local
```

**Configuration:**
- 5 days, 4 slots/day = 20 total slots
- 3 courses, 6 total sessions
- Search space: 64,000,000 permutations

**Output:**

```
=== TIMETABLE ===
Course 0 [T:1, G:1] -> Thu Hour 0
Course 0 [T:1, G:1] -> Mon Hour 1
Course 1 [T:2, G:1] -> Tue Hour 2
Course 1 [T:2, G:1] -> Fri Hour 2
Course 2 [T:1, G:2] -> Tue Hour 0
Course 2 [T:1, G:2] -> Mon Hour 0
=========================
OpenCL Time: 110 ms
```

**Constraint Validation:**
- ✅ Same course sessions on different days (Course 0: Thu/Mon, Course 1: Tue/Fri, Course 2: Tue/Mon)
- ✅ No teacher conflicts at same time
- ✅ No group conflicts at same time

---

### Experiment 2: Hard Thread Input

**Input:**

```
echo "5 6 7  1 1 3  1 2 3  2 1 3  2 2 3  3 3 2  3 1 2  4 2 2  1 8" | ./timetable_local
```

**Configuration:**
- 5 days, 6 slots/day = 30 total slots
- 7 courses with 18 total sessions
- 8 threads

| Course | Teacher | Group | Sessions |
|--------|---------|-------|----------|
| 0 | 1 | 1 | 3 |
| 1 | 1 | 2 | 3 |
| 2 | 2 | 1 | 3 |
| 3 | 2 | 2 | 3 |
| 4 | 3 | 3 | 2 |
| 5 | 3 | 1 | 2 |
| 6 | 4 | 2 | 2 |

**Why it's hard:**
- Teacher 1 has 6 sessions (courses 0, 1) — can't overlap
- Group 1 has 8 sessions (courses 0, 2, 5) — can't overlap
- Group 2 has 8 sessions (courses 1, 3, 6) — can't overlap

---

### Experiment 3: OpenCL Overflow Case

**Input:**

```
echo "5 6 8  1 1 3  1 2 3  2 1 3  2 2 3  3 3 2  3 4 2  4 3 2  4 4 2  2" | ./timetable_local
```

**Result:** `Search Space Size: 0` — Integer overflow

**Explanation:** Search space 30²⁰ ≈ 3.5×10²⁹ exceeds `cl_ulong` max (~1.8×10¹⁹), causing overflow to 0.

**Workaround:** Use smaller input that fits in `cl_ulong`:

```
echo "5 4 6  1 1 2  1 2 2  2 1 2  2 2 2  3 3 2  3 4 2  2" | ./timetable_local
```

## Algorithm Details

### Thread Solver (Backtracking)

- Divides the first session's slot choices across threads
- Each thread performs depth-first backtracking
- Uses `std::atomic` for early termination on solution found

### OpenCL Solver (Brute-Force)

- Enumerates all permutations as GPU work items
- Each work item validates one complete schedule
- Uses atomic operations for result synchronization
- **Limitation:** Search space must fit in `cl_ulong` (~1.8×10¹⁹)

### MPI Solver (Distributed Backtracking)

- Distributes first-level slot choices across MPI ranks
- Each rank performs independent backtracking
- Calls `MPI_Abort` on solution found for fast termination

## Output Example

```
=== TIMETABLE ===
Course 0 [T:1, G:1] -> Mon Hour 0
Course 0 [T:1, G:1] -> Tue Hour 0
Course 1 [T:2, G:1] -> Mon Hour 1
Course 1 [T:2, G:1] -> Wed Hour 1
Course 2 [T:1, G:2] -> Wed Hour 0
Course 2 [T:1, G:2] -> Thu Hour 0
=========================
Time: 42 ms
```

## Limitations

- OpenCL solver cannot handle search spaces exceeding ~10¹⁹ permutations
- Maximum 32 sessions supported in OpenCL kernel (hardcoded array size)
- Day names wrap after 7 days

## License

This project was developed for the Parallel and Distributed Programming course (PPD).

## Authors

**para0107** — [GitHub](https://github.com/para0107)
**mezeirobert0** — [GitHub](https://github.com/mezeirobert0)