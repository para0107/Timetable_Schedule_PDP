#ifndef TIMETABLE_H
#define TIMETABLE_H

#include <vector>
#include <iostream>
#include <string>

// --- PROBLEM CONSTANTS ---
const int DAYS = 5;
const int INTERVALS = 6; // 6 slots per day
const int ROOMS = 2;     // Keep small for testing

struct ClassObject {
    int id;
    int teacher_id;
    int group_id;
    std::string name;
};

struct Assignment {
    int class_id;
    int day;      // 0-4
    int interval; // 0-5
    int room_id;  // 0-(ROOMS-1)
};

// Represents a partial or complete schedule
class TimetableState {
public:
    std::vector<Assignment> assignments;

    // The core sequential validation logic
    bool isValid(const Assignment& new_assign, const std::vector<ClassObject>& allClasses) const {
        // Get details of the class we are trying to add
        const ClassObject& currentClass = allClasses[new_assign.class_id];

        for (const auto& a : assignments) {
            // 1. Check Space Collision (Same Room, Same Time)
            if (a.day == new_assign.day &&
                a.interval == new_assign.interval &&
                a.room_id == new_assign.room_id) {
                return false;
            }

            // Get details of the existing assignment
            const ClassObject& existingClass = allClasses[a.class_id];

            // 2. Check Teacher Collision (Same Teacher, Same Time)
            if (a.day == new_assign.day &&
                a.interval == new_assign.interval &&
                existingClass.teacher_id == currentClass.teacher_id) {
                return false;
            }

            // 3. Check Group Collision (Same Group, Same Time)
            if (a.day == new_assign.day &&
                a.interval == new_assign.interval &&
                existingClass.group_id == currentClass.group_id) {
                return false;
            }
        }
        return true;
    }

    void print(const std::vector<ClassObject>& classes) const {
        std::cout << "\n--- Solution Found ---\n";
        for (const auto& a : assignments) {
            std::cout << "Class " << classes[a.class_id].name
                      << " | Day " << a.day
                      << " | Slot " << a.interval
                      << " | Room " << a.room_id << "\n";
        }
        std::cout << "----------------------\n";
    }
};

// Function prototypes
void solve_threaded(const std::vector<ClassObject>& classes);
void solve_mpi(int argc, char** argv, const std::vector<ClassObject>& classes);
void solve_opencl(const std::vector<ClassObject>& classes);

#endif //TIMETABLE_H