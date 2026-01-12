#ifndef TIMETABLE_H
#define TIMETABLE_H

#include <vector>
#include <iostream>
#include <iomanip>


struct Course {
    int id;
    int teacherId;
    int groupId;
    int sessions;
};

struct Problem {
    int numDays;
    int slotsPerDay;
    std::vector<Course> courses;
};


struct FlattenedSchedule {
    std::vector<int> ownerCourseId;
    int totalSessions;
};


inline FlattenedSchedule flatten(const Problem& p) {
    FlattenedSchedule fs;
    fs.totalSessions = 0;
    for(const auto& c : p.courses) {
        for(int i=0; i<c.sessions; ++i) {
            fs.ownerCourseId.push_back(c.id);
        }
        fs.totalSessions += c.sessions;
    }
    return fs;
}


inline bool isValid(const std::vector<int>& schedule, int currentSessionIdx, int currentSlot,
                    const Problem& p, const FlattenedSchedule& fs) {

    int currentDay = currentSlot / p.slotsPerDay;

    int courseId = fs.ownerCourseId[currentSessionIdx];
    const Course& currentCourse = p.courses[courseId];

    for (int i = 0; i < currentSessionIdx; ++i) {
        int prevSlot = schedule[i];
        int prevDay = prevSlot / p.slotsPerDay;

        int prevCourseId = fs.ownerCourseId[i];
        const Course& prevCourse = p.courses[prevCourseId];

        if (prevSlot == currentSlot) {
            if (prevCourse.teacherId == currentCourse.teacherId) return false;
            if (prevCourse.groupId == currentCourse.groupId) return false;
        }

        if (prevCourseId == courseId) {
            if (prevDay == currentDay) return false;
        }
    }
    return true;
}

inline void printComplexSchedule(const std::vector<int>& schedule, const Problem& p, const FlattenedSchedule& fs) {
    std::cout << "\n=== TIMETABLE ===\n";
    const char* days[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

    for (int i = 0; i < fs.totalSessions; ++i) {
        int slot = schedule[i];
        int day = slot / p.slotsPerDay;
        int hour = slot % p.slotsPerDay;
        int cID = fs.ownerCourseId[i];

        std::cout << "Course " << cID
                  << " [T:" << p.courses[cID].teacherId << ", G:" << p.courses[cID].groupId << "]"
                  << " -> " << days[day%7] << " Hour " << hour << "\n";
    }
    std::cout << "=========================\n";
}

// Function
void solveThreadsComplex(Problem p, int numThreads);
void solveOpenCLComplex(Problem p);

#endif //TIMETABLE_H