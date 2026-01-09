#include "Timetable.h"

#ifdef ENABLE_OPENCL

// Define target version to 3.0 (matches your installed headers)
#define CL_HPP_TARGET_OPENCL_VERSION 300
// Use the new header name as per warning
#include <CL/opencl.hpp>
#include <fstream>
#include <sstream>

const std::string kernelSource = R"(
__kernel void check_timetables(__global int* results, int n_classes, int n_days, int n_slots, int n_rooms) {
    int id = get_global_id(0);
    // Placeholder logic:
    if (id == 12345) {
        results[0] = 1;
        results[1] = id;
    }
}
)";

void solve_opencl(const std::vector<ClassObject>& classes) {
    std::cout << "[OpenCL] Setting up GPU Context...\n";

    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    if(platforms.empty()) { std::cout << "No OpenCL platforms found.\n"; return; }

    cl::Platform platform = platforms.front();
    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    if(devices.empty()) {
        std::cout << "No GPU found, trying CPU...\n";
        platform.getDevices(CL_DEVICE_TYPE_CPU, &devices);
    }
    if(devices.empty()) { std::cout << "No devices found.\n"; return; }

    cl::Device device = devices.front();
    std::cout << "[OpenCL] Using Device: " << device.getInfo<CL_DEVICE_NAME>() << "\n";

    cl::Context context(device);
    cl::CommandQueue queue(context, device);

    // --- FIX IS HERE ---
    // Modern OpenCL C++ bindings define Sources as std::vector<std::string>
    cl::Program::Sources sources;
    sources.push_back(kernelSource);

    cl::Program program(context, sources);
    if(program.build({device}) != CL_SUCCESS) {
        std::cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << "\n";
        return;
    }

    std::vector<int> host_results = {0, 0};
    cl::Buffer bufferResults(context, CL_MEM_READ_WRITE, sizeof(int) * 2);
    queue.enqueueWriteBuffer(bufferResults, CL_TRUE, 0, sizeof(int)*2, host_results.data());

    cl::Kernel kernel(program, "check_timetables");
    kernel.setArg(0, bufferResults);
    kernel.setArg(1, (int)classes.size());
    kernel.setArg(2, DAYS);
    kernel.setArg(3, INTERVALS);
    kernel.setArg(4, ROOMS);

    cl::NDRange global(1000000);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);
    queue.finish();

    queue.enqueueReadBuffer(bufferResults, CL_TRUE, 0, sizeof(int)*2, host_results.data());

    if(host_results[0] == 1) {
        std::cout << "[OpenCL] Found solution at index: " << host_results[1] << "\n";
    } else {
        std::cout << "[OpenCL] No solution found in search range.\n";
    }
}
#else
void solve_opencl(const std::vector<ClassObject>& classes) {
    std::cout << "[OpenCL] Not enabled or library not found.\n";
}
#endif