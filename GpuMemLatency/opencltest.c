#include "opencltest.h"

// default test sizes for latency (in KB)
int default_test_sizes[] = {
    2, 4, 8, 16, 24, 32, 48, 64, 96, 128,
    192, 256, 384, 512, 600, 768, 1024, 1536,
    2048, 3072, 4096, 5120, 6144, 8192,
    16384, 32768, 65536, 98304, 131072,
    196608, 262144, 524288, 1048576
};

// Nemes's VK BW test sizes, in bytes
const uint64_t default_bw_test_sizes[] = {
    4096, 8192, 12288, 16384, 20480, 24576, 28672, 32768, 40960, 49152, 57344,
    65536, 81920, 98304, 114688, 131072, 196608, 262144, 393216, 458752, 524288,
    786432, 1048576, 1572864, 2097152, 3145728, 4194304, 6291456, 8388608,
    12582912, 16777216, 25165824, 33554432, 41943040, 50331648, 58720256,
    67108864, 100663296, 134217728, 201326592, 268435456, 402653184,
    536870912, 805306368, 1073741824, 1610579968, 2147483648, 3221225472,
    4294967296
};

/// <summary>
/// Heuristic to adjust the iteration count for large/small test sizes
/// so that the test takes a reasonable duration.
/// </summary>
uint64_t scale_iterations(uint32_t size_kb, uint64_t iterations) {
    // e.g., scale by the fourth root of size
    return (uint64_t)(10.0 * iterations / pow((double)size_kb, 0.25));
}

/// <summary>
/// Heuristic for bandwidth tests. If buffer size is large, do fewer iterations.
/// </summary>
uint32_t scale_bw_iterations(uint32_t base_iterations, uint32_t size_kb)
{
    if (size_kb < 4096) return base_iterations;
    else return base_iterations / 2;
}

// Forward declaration for integer execution-latency test
float int_exec_latency_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint32_t iterations);

/*
// Obtain max buffer size from device (common helper).
cl_ulong get_max_buffer_size() {
    cl_ulong buffer_size = 0;
    if (CL_SUCCESS != clGetDeviceInfo(selected_device_id, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &buffer_size, NULL)) {
        fprintf(stderr, "Failed to get max buffer size\n");
    }
    return buffer_size;
}

// Obtain max constant buffer size from device (common helper).
cl_ulong get_max_constant_buffer_size() {
    cl_ulong constant_buffer_size = 0;
    if (CL_SUCCESS != clGetDeviceInfo(selected_device_id, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong), &constant_buffer_size, NULL)) {
        fprintf(stderr, "Failed to get max constant buffer size\n");
    }
    return constant_buffer_size;
}
*/
enum TestType {
    GlobalMemLatency,
    ConstantMemLatency,
    LocalMemLatency,
    GlobalAtomicLatency,
    LocalAtomicLatency,
    GlobalMemBandwidth,
    MemBandwidthWorkgroupScaling,
    CoreToCore,
    LinkBandwidth,
    InstructionRate,
};

int main(int argc, char* argv[])
{
    cl_int ret;
    uint32_t stride = 1211;
    // By default, test up to 4Kx4K area
    uint32_t list_size = 3840 * 2160 * 4;
    // "Base" chase iteration count
    uint64_t chase_iterations = (uint64_t)1e6 * 7; 
    // skip = 0 means auto spacing for certain bandwidth tests
    uint32_t thread_count = 1, local_size = 1, skip = 0;
    float result;
    int platform_index = -1, device_index = -1;

    enum TestType testType = GlobalMemLatency;

    char thread_count_set = 0, local_size_set = 0, chase_iterations_set = 0, skip_set = 0;

    // Parse arguments
    for (int argIdx = 1; argIdx < argc; argIdx++) {
        if (*(argv[argIdx]) == '-') {
            char* arg = argv[argIdx] + 1;
            if (_strnicmp(arg, "stride", 6) == 0) {
                argIdx++;
                stride = atoi(argv[argIdx]);
                fprintf(stderr, "Using stride = %u\n", stride);
            }
            else if (_strnicmp(arg, "iterations", 10) == 0) {
                argIdx++;
                chase_iterations = (uint64_t)atoll(argv[argIdx]);
                chase_iterations_set = 1;
                fprintf(stderr, "Using %llu iterations\n", (unsigned long long)chase_iterations);
            }
            else if (_strnicmp(arg, "threads", 7) == 0) {
                argIdx++;
                thread_count = atoi(argv[argIdx]);
                thread_count_set = 1;
                fprintf(stderr, "Using %u threads\n", thread_count);
            }
            else if (_strnicmp(arg, "localsize", 9) == 0) {
                argIdx++;
                local_size = atoi(argv[argIdx]);
                local_size_set = 1;
                fprintf(stderr, "Using local size = %u\n", local_size);
            }
            else if (_strnicmp(arg, "platform", 8) == 0) {
                argIdx++;
                platform_index = atoi(argv[argIdx]);
                fprintf(stderr, "Using OpenCL platform index %d\n", platform_index);
            }
            else if (_strnicmp(arg, "device", 6) == 0) {
                argIdx++;
                device_index = atoi(argv[argIdx]);
                fprintf(stderr, "Using OpenCL device index %d\n", device_index);
            }
            else if (_strnicmp(arg, "bwskip", 6) == 0) {
                argIdx++;
                skip = atoi(argv[argIdx]);
                fprintf(stderr, "Workgroups will be spaced %u apart\n", skip);
            }
            else if (_strnicmp(arg, "test", 4) == 0) {
                argIdx++;
                // Figure out test type
                if (_strnicmp(argv[argIdx], "latency", 7) == 0) {
                    testType = GlobalMemLatency;
                    fprintf(stderr, "Testing global memory latency\n");
                }
                else if (_strnicmp(argv[argIdx], "constantlatency", 15) == 0) {
                    testType = ConstantMemLatency;
                    fprintf(stderr, "Testing constant memory latency\n");
                }
                else if (_strnicmp(argv[argIdx], "globalatomic", 12) == 0) {
                    testType = GlobalAtomicLatency;
                    fprintf(stderr, "Testing global atomic latency\n");
                }
                else if (_strnicmp(argv[argIdx], "locallatency", 12) == 0) {
                    testType = LocalMemLatency;
                    fprintf(stderr, "Testing local memory latency\n");
                }
                else if (_strnicmp(argv[argIdx], "localatomic", 11) == 0) {
                    testType = LocalAtomicLatency;
                    fprintf(stderr, "Testing local atomic latency\n");
                }
                else if (_strnicmp(argv[argIdx], "bw", 2) == 0) {
                    testType = GlobalMemBandwidth;
                    fprintf(stderr, "Testing global memory bandwidth\n");
                    // Some somewhat sensible defaults for memory BW test
                    if (!thread_count_set) thread_count = 131072;
                    if (!local_size_set) local_size = 256;
                    if (!chase_iterations_set) chase_iterations = 500000;
                }
                else if (_strnicmp(argv[argIdx], "scaling", 7) == 0) {
                    testType = MemBandwidthWorkgroupScaling;
                    fprintf(stderr, "Testing BW scaling with multiple workgroups\n");
                    if (!chase_iterations_set) chase_iterations = 20000000;
                }
                else if (_strnicmp(argv[argIdx], "c2c", 3) == 0) {
                    testType = CoreToCore;
                    fprintf(stderr, "Testing c2c atomic latency across compute units\n");
                }
                else if (_strnicmp(argv[argIdx], "link", 4) == 0) {
                    testType = LinkBandwidth;
                    fprintf(stderr, "Testing host <-> GPU link bandwidth\n");
                    if (!chase_iterations_set) chase_iterations = 30000000;
                }
                else if (_strnicmp(argv[argIdx], "instructionrate", 15) == 0) {
                    testType = InstructionRate;
                    fprintf(stderr, "Testing instruction rate\n");
                    if (!chase_iterations_set) chase_iterations = 1000;
                    if (!local_size_set && !thread_count_set) {
                        local_size = 256;
                        thread_count = 32768;
                        fprintf(stderr, "Selecting local size = %d, threads = %d\n", local_size, thread_count);
                    }
                }
                else {
                    fprintf(stderr, "Unknown test: %s\n", argv[argIdx]);
                }
            }
        }
    }

    if (argc == 1) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "\t[-test <latency/constantlatency/globalatomic/localatomic/bw/scaling/c2c/link/instructionrate>]\n");
        fprintf(stderr, "\t[-platform <platform id>] [-device <device id>]\n");
        fprintf(stderr, "\t[-threads <global work size>] [-localsize <workgroup size>] [-bwskip <spacing>]\n");
        fprintf(stderr, "threads must be divisible by localsize.\n");
    }

    fprintf(stderr, "Using %u threads with local size %u\n", thread_count, local_size);

    // Create context, build program, etc.
    cl_context context = get_context_from_user(platform_index, device_index);
    if (context == NULL) {
        exit(1);
    }

    cl_program program = build_program(context, "kernel.cl");
    // Create a command queue
    cl_command_queue command_queue = clCreateCommandQueue(context, selected_device_id, 0, &ret);
    fprintf(stderr, "clCreateCommandQueue returned %d\n", ret);

    // Build and create all needed kernels
    ret = clBuildProgram(program, 1, &selected_device_id, NULL, NULL, NULL);
    fprintf(stderr, "clBuildProgram returned %d\n", ret);

    if (ret == CL_BUILD_PROGRAM_FAILURE || ret == -11) {
        size_t log_size;
        fprintf(stderr, "OpenCL kernel build error.\n");
        clGetProgramBuildInfo(program, selected_device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char* log = (char*)malloc(log_size);
        clGetProgramBuildInfo(program, selected_device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        fprintf(stderr, "%s\n", log);
        free(log);
    }

    // Common kernels
    cl_kernel latency_kernel = clCreateKernel(program, "unrolled_latency_test", &ret);
    cl_kernel bw_kernel = clCreateKernel(program, "sum_bw_test", &ret);
    cl_kernel constant_kernel = clCreateKernel(program, "constant_unrolled_latency_test", &ret);
    cl_kernel int_exec_latency_test_kernel = clCreateKernel(program, "int_exec_latency_test", &ret);
    cl_kernel atomic_latency_test_kernel = clCreateKernel(program, "atomic_exec_latency_test", &ret);
    cl_kernel local_atomic_latency_test_kernel = clCreateKernel(program, "local_atomic_latency_test", &ret);
    cl_kernel c2c_atomic_latency_test_kernel = clCreateKernel(program, "c2c_atomic_exec_latency_test", &ret);
    cl_kernel dummy_add_kernel = clCreateKernel(program, "dummy_add", &ret);

    // Check how big a buffer we can do
    max_global_test_size = get_max_buffer_size();

    switch (testType)
    {
    case GlobalAtomicLatency:
    {
        // Pass NULL for time_ms if you don’t need that measurement
        result = int_atomic_latency_test(context, command_queue, atomic_latency_test_kernel,
                                         (uint32_t)chase_iterations, false, NULL);
        printf("global atomic latency: %f\n", result);
        break;
    }
    case LocalAtomicLatency:
    {
        result = int_atomic_latency_test(context, command_queue, local_atomic_latency_test_kernel,
                                         (uint32_t)chase_iterations, true, NULL);
        printf("local atomic latency: %f\n", result);
        break;
    }
    case GlobalMemLatency:
    {
        fprintf(stderr, "Doing ~%llu p-chase iterations with stride %u\n",
                (unsigned long long)chase_iterations, stride);
        printf("\nSattolo, global memory latency (up to %llu K) unroll:\n", (unsigned long long)(max_global_test_size / 1024));

        for (int size_idx = 0; size_idx < (int)(sizeof(default_test_sizes) / sizeof(int)); size_idx++)
        {
            uint64_t needed_bytes = (uint64_t)256 * default_test_sizes[size_idx] * sizeof(int);
            if (needed_bytes > max_global_test_size) {
                printf("%d K would exceed device's max buffer size of %llu K, stopping.\n",
                       default_test_sizes[size_idx], (unsigned long long)(max_global_test_size / 1024));
                break;
            }
            uint64_t scaled_iters = scale_iterations(default_test_sizes[size_idx], chase_iterations);
            result = latency_test(context,
                                  command_queue,
                                  latency_kernel,
                                  256 * default_test_sizes[size_idx],
                                  (uint32_t)scaled_iters,
                                  true,   // sattolo
                                  0,      // amdworkaround
                                  1,      // threads
                                  1,      // local size
                                  1,      // wave size
                                  NULL    // elapsed_ms
            );
            printf("%d,%f\n", default_test_sizes[size_idx], result);
            if (result == 0) {
                printf("Something went wrong, not testing bigger sizes.\n");
                break;
            }
        }
        break;
    }
    case ConstantMemLatency:
    {
        cl_ulong max_constant_test_size = get_max_constant_buffer_size();
        printf("\nSattolo, constant memory (up to %llu K), no-unroll:\n",
               (unsigned long long)(max_constant_test_size / 1024));

        for (int size_idx = 0; size_idx < (int)(sizeof(default_test_sizes) / sizeof(int)); size_idx++)
        {
            uint64_t needed_bytes = (uint64_t)256 * default_test_sizes[size_idx] * sizeof(int);
            if (needed_bytes > max_constant_test_size) {
                printf("%d K would exceed device's max constant buffer size of %llu K, stopping.\n",
                       default_test_sizes[size_idx], (unsigned long long)(max_constant_test_size / 1024));
                break;
            }
            uint64_t scaled_iters = scale_iterations(default_test_sizes[size_idx], chase_iterations);
            result = latency_test(context,
                                  command_queue,
                                  constant_kernel,
                                  256 * default_test_sizes[size_idx],
                                  (uint32_t)scaled_iters,
                                  true, // sattolo
                                  0,    // amdworkaround
                                  1,
                                  1,
                                  1,
                                  NULL);
            printf("%d,%f\n", default_test_sizes[size_idx], result);
            if (result == 0) {
                printf("Something went wrong, not testing bigger sizes.\n");
                break;
            }
        }
        break;
    }
    case LocalMemLatency:
    {
        cl_kernel local_kernel = clCreateKernel(program, "local_unrolled_latency_test", &ret);
        result = latency_test(context,
                              command_queue,
                              local_kernel,
                              1024,
                              (uint32_t)chase_iterations,
                              true,  // sattolo
                              0,     // amdworkaround
                              1,     // threads
                              1,     // local size
                              1,     // wave size
                              NULL);
        printf("Local mem latency: %f\n", result);
        clReleaseKernel(local_kernel);
        break;
    }
    case GlobalMemBandwidth:
    {
        fprintf(stderr, "Using %u threads, %u local size, base iterations=%llu\n",
                thread_count, local_size, (unsigned long long)chase_iterations);
        printf("\nMemory bandwidth (up to %llu K):\n", (unsigned long long)(max_global_test_size / 1024));

        int bw_test_count = (int)(sizeof(default_bw_test_sizes) / sizeof(unsigned long long));
        for (int size_idx = 0; size_idx < bw_test_count; size_idx++) {
            uint64_t testSizeBytes = default_bw_test_sizes[size_idx];
            uint64_t testSizeKb = testSizeBytes / 1024;
            if (testSizeBytes > max_global_test_size) {
                printf("%llu K would exceed device's max buffer size of %llu K, stopping.\n",
                       (unsigned long long)testSizeKb,
                       (unsigned long long)(max_global_test_size / 1024));
                break;
            }

            uint32_t myIterations = scale_bw_iterations((uint32_t)chase_iterations, (uint32_t)testSizeKb);
            // sum_bw_test operates in float units => pass # of floats
            float bw_res = bw_test(context,
                                   command_queue,
                                   bw_kernel,
                                   /* # of floats */ (testSizeKb * 256),
                                   thread_count,
                                   local_size,
                                   skip,
                                   myIterations);

            printf("%llu,%f\n", (unsigned long long)testSizeKb, bw_res);
            if (bw_res == 0) {
                printf("Error or TDR. Not testing bigger sizes.\n");
                break;
            }
        }
        break;
    }
    case MemBandwidthWorkgroupScaling:
    {
        // Scale # of workgroups from 1 up to the number of CUs
        cl_uint cuCount = getCuCount();
        fprintf(stderr, "Device has %u compute units\n", cuCount);
        int bw_test_count = (int)(sizeof(default_bw_test_sizes) / sizeof(unsigned long long));

        float* scalingResults = (float*)malloc(sizeof(float) * cuCount * bw_test_count);
        memset(scalingResults, 0, sizeof(float) * cuCount * bw_test_count);

        for (uint32_t workgroupCount = 1; workgroupCount <= cuCount; workgroupCount++) {
            for (int size_idx = 0; size_idx < bw_test_count; size_idx++) {
                uint64_t testSizeBytes = default_bw_test_sizes[size_idx];
                uint64_t testSizeKb = testSizeBytes / 1024;
                if (testSizeBytes > max_global_test_size) {
                    fprintf(stderr, "%llu K would exceed device's max buffer size of %llu K\n",
                            (unsigned long long)testSizeKb,
                            (unsigned long long)(max_global_test_size / 1024));
                    scalingResults[(workgroupCount - 1) * bw_test_count + size_idx] = 0;
                    continue;
                }
                uint32_t myIterations = scale_bw_iterations((uint32_t)chase_iterations, (uint32_t)testSizeKb);
                float bw_res = bw_test(context,
                                       command_queue,
                                       bw_kernel,
                                       testSizeKb * 256,
                                       local_size * workgroupCount, // total threads
                                       local_size,
                                       skip,
                                       myIterations);

                scalingResults[(workgroupCount - 1) * bw_test_count + size_idx] = bw_res;
                fprintf(stderr, "%u WGs, %llu KB => %f GB/s\n",
                        workgroupCount, (unsigned long long)testSizeKb, bw_res);
            }
        }

        // Print CSV results
        for (uint32_t wgCount = 1; wgCount <= cuCount; wgCount++) {
            printf(",%u", wgCount);
        }
        printf("\n");
        for (int size_idx = 0; size_idx < bw_test_count; size_idx++) {
            uint64_t testSizeKb = default_bw_test_sizes[size_idx] / 1024;
            printf("%llu", (unsigned long long)testSizeKb);
            for (uint32_t wgCount = 1; wgCount <= cuCount; wgCount++) {
                printf(",%f", scalingResults[(wgCount - 1) * bw_test_count + size_idx]);
            }
            printf("\n");
        }

        free(scalingResults);
        break;
    }
    case CoreToCore:
    {
        // test c2c atomic latency
        c2c_atomic_latency_test(context, command_queue, c2c_atomic_latency_test_kernel, (uint32_t)chase_iterations);
        break;
    }
    case LinkBandwidth:
    {
        // dummy kernel just to have an argument for link_bw_test
        link_bw_test(context, command_queue, dummy_add_kernel, (uint32_t)chase_iterations);
        break;
    }
    case InstructionRate:
    {
        // See instruction_rate.c for function details
        instruction_rate_test(context, command_queue, thread_count, local_size, (uint32_t)chase_iterations);
        break;
    }
    default:
        fprintf(stderr, "No test selected or unknown test.\n");
        break;
    }

    // Pause so user can see the output
    printf("Press Enter to exit.\n");
    getchar();

cleanup:
    clFlush(command_queue);
    clFinish(command_queue);
    clReleaseKernel(latency_kernel);
    clReleaseKernel(bw_kernel);
    clReleaseKernel(constant_kernel);
    clReleaseKernel(int_exec_latency_test_kernel);
    clReleaseKernel(atomic_latency_test_kernel);
    clReleaseKernel(local_atomic_latency_test_kernel);
    clReleaseKernel(c2c_atomic_latency_test_kernel);
    clReleaseKernel(dummy_add_kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);

    return 0;
}

/// <summary>
/// Minimal integer execution-latency test (extra example).
/// </summary>
#define INT_EXEC_INPUT_SIZE 16
float int_exec_latency_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint32_t iterations)
{
    cl_int ret;
    cl_int result = 0;
    size_t global_item_size = 1;
    size_t local_item_size = 1;
    float latency;
    uint32_t time_diff_ms;
    uint32_t A[INT_EXEC_INPUT_SIZE];

    for (int i = 0; i < INT_EXEC_INPUT_SIZE; i++) {
        A[i] = i;
    }

    cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
        INT_EXEC_INPUT_SIZE * sizeof(uint32_t), NULL, &ret);
    cl_mem result_obj = clCreateBuffer(context, CL_MEM_READ_WRITE,
        sizeof(cl_int), NULL, &result);

    ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE,
        0, INT_EXEC_INPUT_SIZE * sizeof(uint32_t), A, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, result_obj, CL_TRUE,
        0, sizeof(cl_int), &result, 0, NULL, NULL);
    clFinish(command_queue);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&a_mem_obj);
    clSetKernelArg(kernel, 1, sizeof(cl_int), (void*)&iterations);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&result_obj);

    start_timing();
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
        &global_item_size, &local_item_size, 0, NULL, NULL);
    if (ret != CL_SUCCESS)
    {
        fprintf(stderr, "Failed to enqueue kernel. ret=%d\n", ret);
        latency = 0;
        goto cleanup;
    }
    clFinish(command_queue);
    time_diff_ms = end_timing();

    // Suppose kernel does ~12 ops per iteration
    latency = 1e6f * (float)time_diff_ms / (float)(iterations * 12);

cleanup:
    clFlush(command_queue);
    clFinish(command_queue);
    clReleaseMemObject(a_mem_obj);
    clReleaseMemObject(result_obj);
    return latency;
}

