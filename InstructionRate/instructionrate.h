#pragma once

#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif

// make mingw happy for cross compiling
#ifdef __MINGW32__
    #define aligned_alloc(align, size) _aligned_malloc(size, align)
#endif

#define noinline      __attribute__((__noinline__))
#define always_inline __attribute__((__always_inline__))

/* Macros to repeat elements */
#define REP2(x)       x x
#define REP3(x)       x x x
#define REP4(x)       x x x x
#define REP5(x)       x x x x x
#define REP6(x)       x x x x x x
#define REP7(x)       x x x x x x x
#define REP8(x)       x x x x x x x x
#define REP9(x)       x x x x x x x x x
#define REP10(x)      x x x x x x x x x x
#define REP15(x)      REP10(x) REP5(x)
#define REP20(x)      REP10(x) REP10(x)
#define REP25(x)      REP20(x) REP5(x)
#define REP30(x)      REP20(x) REP10(x)
#define REP40(x)      REP20(x) REP20(x)
#define REP50(x)      REP20(x) REP20(x) REP10(x)
