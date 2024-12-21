#pragma once

#ifndef opencltestheader
#define opencltestheader

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "../common/timing.h"

#define false 0
#define true 1

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#ifndef __APPLE__
#include <CL/cl.h>
#else
#include <OpenCL/cl.h>
#endif

#define MAX_SOURCE_SIZE (0x100000)
#define CACHELINE_SIZE 64

#ifndef _MSC_VER
#define _strnicmp strncmp
#endif

// These globals are defined in common.c
extern cl_device_id selected_device_id;
extern cl_platform_id selected_platform_id;
extern cl_ulong max_global_test_size;

// From common.c
cl_context get_context_from_user(int platform_index, int device_index);
cl_program build_program(cl_context context, const char* fname);
void write_program(cl_program program);
void FillPatternArr(uint32_t* pattern_arr, uint32_t list_size, uint32_t byte_increment);
cl_uint getCuCount();
size_t getMaxWorkgroupSize();
short checkExtensionSupport(const char *extension_name);
cl_ulong get_max_constant_buffer_size();
cl_ulong get_max_buffer_size();
cl_ulong get_max_tex_buffer_size();
cl_ulong get_max_2d_tex_width();
cl_ulong get_max_2d_tex_height();

// From atomic_test.c
float int_atomic_latency_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint32_t iterations,
    short local,
    uint32_t *time_ms);

float c2c_atomic_latency_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint32_t iterations);

// From latency_test.c
float latency_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint32_t list_size,
    uint32_t chase_iterations,
    short sattolo,
    short amdworkaround,
    int threads,
    int local_size,
    int wave_size,
    uint32_t *elapsed_ms);

float tex_latency_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint32_t list_size,
    uint32_t chase_iterations,
    int threads,
    int local_size,
    int wave_size);

// From bw_test.c
float bw_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint64_t list_size,
    uint32_t thread_count,
    uint32_t local_size,
    uint32_t skip,
    uint32_t chase_iterations);

float tex_bw_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint64_t width,
    uint64_t height,
    uint32_t thread_count,
    uint32_t local_size,
    uint32_t randomize,
    uint32_t chase_iterations,
    int64_t *time_ms);

float local_bw_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint32_t thread_count,
    uint32_t local_size,
    uint32_t chase_iterations,
    int64_t *time_ms);

float buffer_bw_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint32_t thread_count,
    uint32_t local_size,
    uint32_t chase_iterations,
    int64_t* time_ms);

float local_chase_bw_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint32_t thread_count,
    uint32_t local_size,
    uint32_t chase_iterations,
    uint32_t wave_size,
    int64_t* time_ms);

float local_64_bw_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint32_t thread_count,
    uint32_t local_size,
    uint32_t chase_iterations,
    int64_t* time_ms);

void link_bw_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint32_t iterations);

// From instruction_rate.c
float instruction_rate_test(cl_context context,
    cl_command_queue command_queue,
    uint32_t thread_count,
    uint32_t local_size,
    uint32_t chase_iterations);

float fp64_instruction_rate_test(cl_context context,
    cl_command_queue command_queue,
    uint32_t thread_count,
    uint32_t local_size,
    uint32_t chase_iterations,
    int float4_element_count,
    cl_mem a_mem_obj,
    cl_mem result_obj,
    cl_float *A,
    cl_float *result);

float fp16_instruction_rate_test(cl_context context,
    cl_command_queue command_queue,
    uint32_t thread_count,
    uint32_t local_size,
    uint32_t chase_iterations,
    int float4_element_count,
    cl_mem a_mem_obj,
    cl_mem result_obj,
    cl_float* A,
    cl_float* result);

float run_rate_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint32_t thread_count,
    uint32_t local_size,
    uint32_t chase_iterations,
    int float4_element_count,
    cl_mem a_mem_obj,
    cl_mem result_obj,
    cl_float* A,
    cl_float* result,
    float opsPerIteration);

float run_latency_test(cl_context context,
    cl_command_queue command_queue,
    cl_kernel kernel,
    uint32_t chase_iterations,
    int float4_element_count,
    cl_mem a_mem_obj,
    cl_mem result_obj,
    cl_float* A,
    cl_float* result,
    float opsPerIteration);

#endif

