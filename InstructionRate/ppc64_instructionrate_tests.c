#include "instructionrate.h"

#include <stdint.h>


// clang-format off
#define LATENCY_LOOP(x)                 \
    "1:\n\t"                            \
    REP50(x "\n\t")                     \
    "sub %[iter], %[iter], %[ops]\n\t"  \
    "cmpldi %[iter], 0\n\t"             \
    "bgt+ 1b\n\t"
// clang-format on


uint64_t noinline clktest(uint64_t iterations, void *data) {
    register uint64_t iter = iterations;
    register uint64_t ops  = 50;
    register uint64_t one  = 1;

    asm volatile("xor %%r8, %%r8, %%r8\n\t"

                 LATENCY_LOOP("add 8, 8, %[one]")

                 : [iter] "+r"(iter)
                 : [ops] "r"(ops), [one] "r"(one)
                 : "cc", "r8");
    return iter;
}


uint64_t noinline nooptest(uint64_t iterations, void *data) {
    register uint64_t iter = iterations;
    register uint64_t ops  = 50;

    asm volatile(LATENCY_LOOP("ori 0, 0, 0") : [iter] "+r"(iter) : [ops] "r"(ops) : "cc", "r0");
    return iter;
}


uint64_t noinline addtest(uint64_t iterations, void *data) {
    register uint64_t iter = iterations;
    register uint64_t ops  = 40;

    uint64_t         *testArr = (uint64_t *)data;
    register uint64_t val1    = testArr[0];
    register uint64_t val2    = testArr[1];
    register uint64_t val3    = testArr[2];
    register uint64_t val4    = testArr[3];
    register uint64_t val5    = testArr[4];
    register uint64_t val6    = testArr[5];
    register uint64_t val7    = testArr[6];
    register uint64_t val8    = testArr[7];

    asm volatile("1:\n\t"

                 REP5("add %[val1], %[val1], %[val1]\n\t"
                      "add %[val2], %[val2], %[val2]\n\t"
                      "add %[val3], %[val3], %[val3]\n\t"
                      "add %[val4], %[val4], %[val4]\n\t"
                      "add %[val5], %[val5], %[val5]\n\t"
                      "add %[val6], %[val6], %[val6]\n\t"
                      "add %[val7], %[val7], %[val7]\n\t"
                      "add %[val8], %[val8], %[val8]\n\t")

                     "sub %[iter], %[iter], %[ops]\n\t"
                     "cmpldi %[iter], 0\n\t"
                     "bgt+ 1b\n\t"
                 : [iter] "+r"(iter),
                   [val1] "+r"(val1),
                   [val2] "+r"(val2),
                   [val3] "+r"(val3),
                   [val4] "+r"(val4),
                   [val5] "+r"(val5),
                   [val6] "+r"(val6),
                   [val7] "+r"(val7),
                   [val8] "+r"(val8)
                 : [ops] "r"(ops)
                 : "cc");
    return iter;
}

uint64_t noinline addlattest(uint64_t iterations, void *data) {
    register uint64_t iter = iterations;
    register uint64_t ops  = 50;
    register uint64_t one  = 1;

    asm volatile("xor %%r8, %%r8, %%r8\n\t"

                 LATENCY_LOOP("add 8, 8, %[one]")

                 : [iter] "+r"(iter)
                 : [ops] "r"(ops), [one] "r"(one)
                 : "cc", "r8");
    return iter;
}


uint64_t noinline faddtest(uint64_t iterations, void *data) {
    register uint64_t iter = iterations;
    register uint64_t ops  = 40;

    double         *testArr = (double *)data;
    register double val1    = testArr[0];
    register double val2    = testArr[1];
    register double val3    = testArr[2];
    register double val4    = testArr[3];
    register double val5    = testArr[4];
    register double val6    = testArr[5];
    register double val7    = testArr[6];
    register double val8    = testArr[7];

    asm volatile("1:\n\t"

                 REP5("fadd %[val1], %[val1], %[val1]\n\t"
                      "fadd %[val2], %[val2], %[val2]\n\t"
                      "fadd %[val3], %[val3], %[val3]\n\t"
                      "fadd %[val4], %[val4], %[val4]\n\t"
                      "fadd %[val5], %[val5], %[val5]\n\t"
                      "fadd %[val6], %[val6], %[val6]\n\t"
                      "fadd %[val7], %[val7], %[val7]\n\t"
                      "fadd %[val8], %[val8], %[val8]\n\t")

                     "sub %[iter], %[iter], %[ops]\n\t"
                     "cmpldi %[iter], 0\n\t"
                     "bgt+ 1b\n\t"
                 : [iter] "+r"(iter),
                   [val1] "+f"(val1),
                   [val2] "+f"(val2),
                   [val3] "+f"(val3),
                   [val4] "+f"(val4),
                   [val5] "+f"(val5),
                   [val6] "+f"(val6),
                   [val7] "+f"(val7),
                   [val8] "+f"(val8)
                 : [ops] "r"(ops)
                 : "cc");
    return iter;
}

uint64_t noinline faddlattest(uint64_t iterations, void *data) {
    register uint64_t iter = iterations;
    register uint64_t ops  = 50;
    register double   zero = 0.0;
    register double   one  = 1.0;

    asm volatile(LATENCY_LOOP("fadd %[zero], %[zero], %[one]")
                 : [iter] "+r"(iter), [zero] "+r"(zero)
                 : [ops] "r"(ops), [one] "r"(one)
                 : "cc");
    return iter;
}


uint64_t noinline faddstest(uint64_t iterations, void *data) {
    register uint64_t iter = iterations;
    register uint64_t ops  = 40;

    float         *testArr = (float *)data;
    register float val1    = testArr[0];
    register float val2    = testArr[1];
    register float val3    = testArr[2];
    register float val4    = testArr[3];
    register float val5    = testArr[4];
    register float val6    = testArr[5];
    register float val7    = testArr[6];
    register float val8    = testArr[7];

    asm volatile("1:\n\t"

                 REP5("fadds %[val1], %[val1], %[val1]\n\t"
                      "fadds %[val2], %[val2], %[val2]\n\t"
                      "fadds %[val3], %[val3], %[val3]\n\t"
                      "fadds %[val4], %[val4], %[val4]\n\t"
                      "fadds %[val5], %[val5], %[val5]\n\t"
                      "fadds %[val6], %[val6], %[val6]\n\t"
                      "fadds %[val7], %[val7], %[val7]\n\t"
                      "fadds %[val8], %[val8], %[val8]\n\t")

                     "sub %[iter], %[iter], %[ops]\n\t"
                     "cmpldi %[iter], 0\n\t"
                     "bgt+ 1b\n\t"
                 : [iter] "+r"(iter),
                   [val1] "+f"(val1),
                   [val2] "+f"(val2),
                   [val3] "+f"(val3),
                   [val4] "+f"(val4),
                   [val5] "+f"(val5),
                   [val6] "+f"(val6),
                   [val7] "+f"(val7),
                   [val8] "+f"(val8)
                 : [ops] "r"(ops)
                 : "cc");
    return iter;
}


uint64_t noinline faddslattest(uint64_t iterations, void *data) {
    register uint64_t iter = iterations;
    register uint64_t ops  = 50;
    register float    zero = 0.0;
    register float    one  = 1.0;

    asm volatile(LATENCY_LOOP("fadds %[zero], %[zero], %[one]")
                 : [iter] "+r"(iter), [zero] "+r"(zero)
                 : [ops] "r"(ops), [one] "r"(one)
                 : "cc");
    return iter;
}
