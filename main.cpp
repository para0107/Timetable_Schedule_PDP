#include "Timetable.h"
#include <iostream>

int main() {
    Problem p;
    int numCourses;

    std::cout << "=== Timetable Generator ===\n";
    std::cout << "Enter number of Days (e.g., 5): ";
    std::cin >> p.numDays;
    std::cout << "Enter Slots per Day (e.g., 4): ";
    std::cin >> p.slotsPerDay;

    std::cout << "Enter number of Courses: ";
    std::cin >> numCourses;

    for(int i=0; i<numCourses; ++i) {
        Course c;
        c.id = i;
        std::cout << "\n--- Course " << i << " ---\n";
        std::cout << "  Teacher ID (int): "; std::cin >> c.teacherId;
        std::cout << "  Group ID (int): ";   std::cin >> c.groupId;
        std::cout << "  Sessions/Week: ";    std::cin >> c.sessions;
        p.courses.push_back(c);
    }

    int choice;
    std::cout << "\nChoose Method:\n1. Threads\n2. OpenCL\n> ";
    std::cin >> choice;

    if (choice == 1) {
        int t;
        std::cout << "Threads count: "; std::cin >> t;
        solveThreadsComplex(p, t);
    } else {
        solveOpenCLComplex(p);
    }

    return 0;
}