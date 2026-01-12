#include "Timetable.h"
#include <CL/cl.h>
#include <fstream>
#include <cmath>
#include <chrono>
#include <iostream>

std::string loadKernel(const char* name) {
    std::ifstream in(name);
    std::string result((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return result;
}

void solveOpenCLComplex(Problem p) {
    FlattenedSchedule fs = flatten(p);
    int totalSlots = p.numDays * p.slotsPerDay;
    cl_ulong totalPermutations = (cl_ulong)std::pow(totalSlots, fs.totalSessions);

    std::cout << "Running OpenCL...\n";
    std::cout << "Search Space Size: " << totalPermutations << "\n";

    int* h_flatOwner = fs.ownerCourseId.data();
    int* h_teachers  = new int[p.courses.size()];
    int* h_groups    = new int[p.courses.size()];

    for(size_t i=0; i<p.courses.size(); ++i) {
        h_teachers[i] = p.courses[i].teacherId;
        h_groups[i]   = p.courses[i].groupId;
    }

    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;
    cl_uint ret_num_devices, ret_num_platforms;
    cl_int err;

    err = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to get platform IDs: " << err << std::endl;
        return;
    }

    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to get device IDs: " << err << std::endl;
        return;
    }

    cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create context: " << err << std::endl;
        return;
    }

    cl_command_queue queue = clCreateCommandQueue(context, device_id, 0, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create command queue: " << err << std::endl;
        return;
    }

    std::string src = loadKernel("kernel.cl");
    if (src.empty()) {
        std::cerr << "Failed to load kernel.cl" << std::endl;
        return;
    }

    const char* source_str = src.c_str();
    size_t source_size = src.size();
    cl_program program = clCreateProgramWithSource(context, 1, &source_str, &source_size, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create program: " << err << std::endl;
        return;
    }

    err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t log_size;
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char* log = new char[log_size];
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        std::cerr << "OpenCL Build Error:\n" << log << std::endl;
        delete[] log;
        return;
    }

    cl_kernel kernel = clCreateKernel(program, "searchComplex", &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create kernel: " << err << std::endl;
        return;
    }

    int h_flag = 0;
    int* h_schedule = new int[fs.totalSessions];

    cl_mem d_flag     = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int), &h_flag, NULL);
    cl_mem d_schedule = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int)*fs.totalSessions, NULL, NULL);
    cl_mem d_owner    = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int)*fs.totalSessions, h_flatOwner, NULL);
    cl_mem d_teachers = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int)*p.courses.size(), h_teachers, NULL);
    cl_mem d_groups   = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int)*p.courses.size(), h_groups, NULL);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_flag);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_schedule);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_owner);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &d_teachers);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), &d_groups);
    clSetKernelArg(kernel, 5, sizeof(int), &fs.totalSessions);
    clSetKernelArg(kernel, 6, sizeof(int), &totalSlots);
    clSetKernelArg(kernel, 7, sizeof(int), &p.slotsPerDay);
    clSetKernelArg(kernel, 8, sizeof(cl_ulong), &totalPermutations);

    auto start = std::chrono::high_resolution_clock::now();

    size_t global_size = (size_t)totalPermutations;
    size_t local_size = 64;
    global_size = (global_size + local_size - 1) / local_size * local_size;

    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to enqueue kernel: " << err << std::endl;
        return;
    }

    clFinish(queue);

    clEnqueueReadBuffer(queue, d_flag, CL_TRUE, 0, sizeof(int), &h_flag, 0, NULL, NULL);

    if (h_flag == 1) {
        clEnqueueReadBuffer(queue, d_schedule, CL_TRUE, 0, sizeof(int)*fs.totalSessions, h_schedule, 0, NULL, NULL);
        std::vector<int> res(h_schedule, h_schedule + fs.totalSessions);
        printComplexSchedule(res, p, fs);
    } else {
        std::cout << "No solution found in searched range.\n";
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "OpenCL Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << " ms\n";

    delete[] h_teachers;
    delete[] h_groups;
    delete[] h_schedule;
    clReleaseMemObject(d_flag);
    clReleaseMemObject(d_schedule);
    clReleaseMemObject(d_owner);
    clReleaseMemObject(d_teachers);
    clReleaseMemObject(d_groups);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
}
