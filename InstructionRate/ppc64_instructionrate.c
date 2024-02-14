#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
    #include <sys/types.h>
#endif

#include "instructionrate.h"

#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


extern noinline uint64_t clktest(uint64_t iterations, void *data);
extern noinline uint64_t nooptest(uint64_t iterations, void *data);
extern noinline uint64_t addtest(uint64_t iterations, void *data);
extern noinline uint64_t addlattest(uint64_t iterations, void *data);
extern noinline uint64_t faddtest(uint64_t iterations, void *data);
extern noinline uint64_t faddlattest(uint64_t iterations, void *data);
extern noinline uint64_t faddstest(uint64_t iterations, void *data);
extern noinline uint64_t faddslattest(uint64_t iterations, void *data);

/*
extern noinline uint64_t xmmaddtest(uint64_t iterations, void *data);
extern noinline uint64_t xmmaddlattest(uint64_t iterations, void *data);
extern noinline uint64_t xmmfaddtest(uint64_t iterations, void *data);
extern noinline uint64_t xmmfaddlattest(uint64_t iterations, void *data);
extern noinline uint64_t ymmaddtest(uint64_t iterations, void *data);
extern noinline uint64_t ymmaddlattest(uint64_t iterations, void *data);
extern noinline uint64_t ymmfaddtest(uint64_t iterations, void *data);
extern noinline uint64_t ymmfaddlattest(uint64_t iterations, void *data);
extern noinline uint64_t multest(uint64_t iterations, void *data);
extern noinline uint64_t mullattest(uint64_t iterations, void *data);
extern noinline uint64_t fmultest(uint64_t iterations, void *data);
extern noinline uint64_t fmullattest(uint64_t iterations, void *data);
extern noinline uint64_t xmmmultest(uint64_t iterations, void *data);
extern noinline uint64_t xmmmullattest(uint64_t iterations, void *data);
extern noinline uint64_t xmmfmultest(uint64_t iterations, void *data);
extern noinline uint64_t xmmfmullattest(uint64_t iterations, void *data);
extern noinline uint64_t ymmmultest(uint64_t iterations, void *data);
extern noinline uint64_t ymmmullattest(uint64_t iterations, void *data);
extern noinline uint64_t ymmfmultest(uint64_t iterations, void *data);
extern noinline uint64_t ymmfmullattest(uint64_t iterations, void *data);
extern noinline uint64_t divtest(uint64_t iterations, void *data);
extern noinline uint64_t divlattest(uint64_t iterations, void *data);
extern noinline uint64_t fdivtest(uint64_t iterations, void *data);
extern noinline uint64_t fdivlattest(uint64_t iterations, void *data);
extern noinline uint64_t fmatest(uint64_t iterations, void *data);
extern noinline uint64_t fmalattest(uint64_t iterations, void *data);
extern noinline uint64_t xmmfmatest(uint64_t iterations, void *data);
extern noinline uint64_t xmmfmalattest(uint64_t iterations, void *data);
extern noinline uint64_t ymmfmatest(uint64_t iterations, void *data);
extern noinline uint64_t ymmfmalattest(uint64_t iterations, void *data);
extern noinline uint64_t ymmshuffletest(uint64_t iterations, void *data);
extern noinline uint64_t xmmloadtest(uint64_t iterations, void *data);
extern noinline uint64_t xmmstoretest(uint64_t iterations, void *data);
extern noinline uint64_t ymmloadtest(uint64_t iterations, void *data);
extern noinline uint64_t ymmstoretest(uint64_t iterations, void *data);
*/

/* single-precision floats */
float     fpTestArr[8] __attribute__((aligned(64)))  = { 0.2, 1.5, 2.7, 3.14, 5.16, 6.3, 7.7, 9.45 };
float     fpSinkArr[8] __attribute__((aligned(64)))  = { 2.1, 3.2, 4.3, 5.4, 6.2, 7.8, 8.3, 9.4 };
/* double-precision floats */
double    dpTestArr[8] __attribute__((aligned(64)))  = { 0.2, 1.5, 2.7, 3.14, 5.16, 6.3, 7.7, 9.45 };
double    dpSinkArr[8] __attribute__((aligned(64)))  = { 0.2, 1.5, 2.7, 3.14, 5.16, 6.3, 7.7, 9.45 };
/* 64-bit integers (no point to 32, not faster here) */
int       intSinkArr[8] __attribute__((aligned(64))) = { 2, 3, 4, 5, 6, 7, 8, 9 };
uint64_t *intTestArr;  // dynamically allocated at runtime

int       threads = 0, hardaffinity = 0;
cpu_set_t cpuset;

typedef uint64_t (*testfunc_t)(uint64_t iterations, void *data);

float measureFunction(uint64_t iterations, float clockSpeedGhz, void *arr, testfunc_t testfunc);

int main(int argc, char *argv[]) {
    struct timespec start_ts, end_ts;

    uint64_t intTestArrLength = 1024;
    uint64_t iterations       = 500000000;
    uint64_t iterationsHigh   = iterations * 5;
    double   time_diff_ms     = 0.0F;
    float    latency_ns = 0.0F, clockSpeedGhz = 0.0F;
    // float    val = 0.0F;

    intTestArr = aligned_alloc(64, sizeof(uint64_t) * intTestArrLength);
    for (uint64_t i = 0; i < intTestArrLength; i++) {
        intTestArr[i] = i;
    }

    if (argc > 1) {
        for (int argIdx = 1; argIdx < argc; argIdx++) {
            if (*(argv[argIdx]) == '-') {
                char *arg = argv[argIdx] + 1;
                if (strncmp(arg, "affinity", 8) == 0) {
                    argIdx++;
                    int targetCpu = atoi(argv[argIdx]);
                    CPU_ZERO(&cpuset);
                    CPU_SET(targetCpu, &cpuset);
                    sched_setaffinity(gettid(), sizeof(cpu_set_t), &cpuset);
                    fprintf(stderr, "Set affinity to %d\n", targetCpu);
                } else if (strncmp(arg, "hardaffinity", 12) == 0) {
                    CPU_ZERO(&cpuset);
                    CPU_SET(0, &cpuset);
                    CPU_SET(1, &cpuset);
                    sched_setaffinity(gettid(), sizeof(cpu_set_t), &cpuset);
                    fprintf(stderr, "Set affinity 2,3\n");
                    hardaffinity = 1;
                } else if (strncmp(arg, "threads", 7) == 0) {
                    argIdx++;
                    threads = atoi(argv[argIdx]);
                    fprintf(stderr, "Multithreading mode, %d threads\n", threads);
                } else if (strncmp(arg, "iter", 4) == 0) {
                    argIdx++;
                    int iterMul = atoi(argv[argIdx]);
                    iterations *= iterMul;
                    iterationsHigh *= iterMul;
                    fprintf(stderr, "Scaled iterations by %d\n", iterMul);
                }
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &start_ts);
    (void)clktest(iterationsHigh, NULL);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end_ts);

    time_diff_ms =
        ((end_ts.tv_sec - start_ts.tv_sec) * 1000.0F) + ((end_ts.tv_nsec - start_ts.tv_nsec) / 1000000.0F);
    latency_ns = (1000000.0F * (float)time_diff_ms) / (float)iterationsHigh;
    // printf("Estimated latency: %.3f ns\n", latency_ns);

    // clk speed should be 1/latency, assuming we got one add per clk, roughly
    clockSpeedGhz = 1 / latency_ns;
    printf("Estimated clock speed: %.2f GHz\n", clockSpeedGhz);

    // throughput
    if (argc == 1 || (argc > 1 && strncmp(argv[1], "nop", 3) == 0))
        printf("nops per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, &nooptest));


    if (argc == 1 || (argc > 1 && strncmp(argv[1], "add", 3) == 0))
        printf(
            "64b int adds per clk: %.2f\n",
            measureFunction(iterationsHigh, clockSpeedGhz, intTestArr, &addtest)
        );

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "addlatency", 3) == 0))
        printf(
            "64b int add latency (cycles): %.2f\n",
            1 / measureFunction(iterationsHigh, clockSpeedGhz, intTestArr, &addlattest)
        );

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "fadd", 3) == 0))
        printf(
            "64b floating point adds per clk: %.2f\n",
            measureFunction(iterationsHigh, clockSpeedGhz, dpTestArr, &faddtest)
        );

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "faddlatency", 3) == 0))
        printf(
            "64b floating point add latency (cycles): %.2f\n",
            1 / measureFunction(iterationsHigh, clockSpeedGhz, fpTestArr, &faddlattest)
        );

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "fadds", 3) == 0))
        printf(
            "32b floating point adds per clk: %.2f\n",
            measureFunction(iterationsHigh, clockSpeedGhz, fpTestArr, &faddstest)
        );

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "faddslatency", 3) == 0))
        printf(
            "32b floating point add latency (cycles): %.2f\n",
            1 / measureFunction(iterationsHigh, clockSpeedGhz, fpTestArr, &faddslattest)
        );

    /*

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "xmmadd", 3) == 0))
        printf("128b integer adds per clk: %.2f\n",
                measureFunction(iterationsHigh, clockSpeedGhz, NULL, &xmmaddtest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "xmmaddlatency", 3) == 0))
        printf("128b integer add latency (cycles): %.2f\n",
                1 / measureFunction(iterationsHigh, clockSpeedGhz, NULL, &xmmaddlattest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "xmmfadd", 3) == 0))
        printf("128b floating point adds per clk: %.2f\n",
                measureFunction(iterationsHigh, clockSpeedGhz, NULL, &xmmfaddtest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "xmmaddlatency", 3) == 0))
        printf("128b floating point add latency (cycles): %.2f\n",
                1 / measureFunction(iterationsHigh, clockSpeedGhz, NULL, &xmmfaddlattest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "ymmadd", 3) == 0))
        printf("256b integer adds per clk: %.2f\n",
                measureFunction(iterationsHigh, clockSpeedGhz, NULL, &ymmaddtest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "ymmaddlatency", 3) == 0))
        printf("256b integer add latency (cycles): %.2f\n",
                1 / measureFunction(iterationsHigh, clockSpeedGhz, NULL, &ymmaddlattest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "ymmfadd", 3) == 0))
        printf("256b floating point adds per clk: %.2f\n",
                measureFunction(iterationsHigh, clockSpeedGhz, NULL, &ymmfaddtest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "ymmfaddlatency", 3) == 0))
        printf("256b floating point add latency (cycles): %.2f\n",
                1 / measureFunction(iterationsHigh, clockSpeedGhz, NULL, &ymmfaddlattest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "mul", 3) == 0))
        printf("64b integer multiplies per clk: %.2f\n",
                measureFunction(iterationsHigh, clockSpeedGhz, intTestArr, &multest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "mullatency", 3) == 0))
        printf("64b integer multiply latency (cycles): %.2f\n",
                1 / measureFunction(iterationsHigh, clockSpeedGhz, NULL, &mullattest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "fmul", 3) == 0))
        printf("64b floating point multiplies per clk: %.2f\n",
                measureFunction(iterationsHigh, clockSpeedGhz, NULL, &fmultest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "fmullatency", 3) == 0))
        printf("64b floating point multiply latency (cycles): %.2f\n",
                1 / measureFunction(iterationsHigh, clockSpeedGhz, NULL, &fmullattest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "xmmmul", 3) == 0))
        printf("128b integer multiply per clk: %.2f\n",
                measureFunction(iterationsHigh, clockSpeedGhz, NULL, &xmmmultest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "xmmmullatency", 3) == 0))
        printf("128b integer multiply (cycles): %.2f\n",
                1 / measureFunction(iterationsHigh, clockSpeedGhz, NULL, &xmmmullattest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "xmmfmul", 3) == 0))
        printf("128b floating point multiplies per clk: %.2f\n",
                measureFunction(iterationsHigh, clockSpeedGhz, NULL, &xmmfmultest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "xmmfmullatency", 3) == 0))
        printf("128b floating point multiply latency (cycles): %.2f\n",
                1 / measureFunction(iterationsHigh, clockSpeedGhz, NULL, &xmmfmullattest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "ymmmul", 3) == 0))
        printf("256b integer multiplies per clk: %.2f\n",
                measureFunction(iterationsHigh, clockSpeedGhz, NULL, &ymmmultest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "ymmmullatency", 3) == 0))
        printf("256b integer multiply latency (cycles): %.2f\n",
                1 / measureFunction(iterationsHigh, clockSpeedGhz, NULL, &ymmmullattest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "ymmfmul", 3) == 0))
        printf("256b floating point multiplies per clk: %.2f\n",
                measureFunction(iterationsHigh, clockSpeedGhz, NULL, &ymmfmultest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "ymmfmullatency", 3) == 0))
        printf("256b floating point multiply latency (cycles): %.2f\n",
                1 / measureFunction(iterationsHigh, clockSpeedGhz, NULL, &ymmfmullattest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "div", 3) == 0))
        printf("divs per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, &divtest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "divlatency", 3) == 0))
        printf("int div latency (cycles): %.2f\n",
                1 / measureFunction(iterationsHigh, clockSpeedGhz, NULL, &divlattest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "fdiv", 3) == 0))
        printf("fdivs per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL, &fdivtest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "fdivlatency", 3) == 0))
        printf("fdiv latency (cycles): %.2f\n",
                1 / measureFunction(iterationsHigh, clockSpeedGhz, NULL, &fdivlattest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "fma", 3) == 0))
        printf("64b fmas per clk: %.2f\n",
                measureFunction(iterationsHigh, clockSpeedGhz, fpTestArr, &fmatest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "fmalatency", 3) == 0))
        printf("64b fma latency (cycles): %.2f\n",
                1 / measureFunction(iterationsHigh, clockSpeedGhz, NULL, &fmalattest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "xmmfma", 3) == 0))
        printf("128b fmas per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL,
    &xmmfmatest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "xmmfmalatency", 3) == 0))
        printf("128b fma latency (cycles): %.2f\n",
                1 / measureFunction(iterationsHigh, clockSpeedGhz, NULL, &xmmfmalattest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "ymmfma", 3) == 0))
        printf("256b fmas per clk: %.2f\n", measureFunction(iterationsHigh, clockSpeedGhz, NULL,
    &ymmfmatest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "ymmfmalatency", 3) == 0))
        printf("256b fma latency (cycles): %.2f\n",
                1 / measureFunction(iterationsHigh, clockSpeedGhz, NULL, &ymmfmalattest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "ymmshuffle", 3) == 0))
        printf("256b shuffles per clk: %.2f\n",
                measureFunction(iterationsHigh, clockSpeedGhz, NULL, &ymmshuffletest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "xmmload", 4) == 0))
        printf("128b load per clk: %.2f\n",
                measureFunction(iterationsHigh, clockSpeedGhz, intSinkArr, &xmmloadtest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "xmmstore", 4) == 0))
        printf("128b store per clk: %.2f\n",
                measureFunction(iterationsHigh, clockSpeedGhz, intSinkArr, &xmmstoretest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "ymmload", 4) == 0))
        printf("256b load per clk: %.2f\n",
                measureFunction(iterationsHigh, clockSpeedGhz, intSinkArr, &ymmloadtest));

    if (argc == 1 || (argc > 1 && strncmp(argv[1], "ymmstore", 4) == 0))
        printf("256b store per clk: %.2f\n",
                measureFunction(iterationsHigh, clockSpeedGhz, intSinkArr, &ymmstoretest));

    return 0;
    */
}

float measureFunction(uint64_t iterations, float clockSpeedGhz, void *arr, testfunc_t testfunc) {
    struct timespec start_ts, end_ts;

    uint64_t val = 0;
    double   time_diff_ms;
    float    latency = 0.0F, opsPerNs = 0.0F, ret = 0.0F;

    clock_gettime(CLOCK_MONOTONIC_RAW, &start_ts);
    val = testfunc(iterations, arr);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end_ts);
    (void)val;

    time_diff_ms =
        ((end_ts.tv_sec - start_ts.tv_sec) * 1000.0F) + ((end_ts.tv_nsec - start_ts.tv_nsec) / 1000000.0F);
    latency  = 1000000.0F * (float)time_diff_ms / (float)iterations;
    opsPerNs = 1 / latency;

    ret = (float)(opsPerNs / clockSpeedGhz);
    return ret;
}
