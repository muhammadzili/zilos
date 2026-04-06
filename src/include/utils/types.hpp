#pragma once

/**
 * @file types.hpp
 * @brief Standard Integer Types
 * 
 * Provides fixed-width integer definitions for the freestanding kernel environment.
 */

// --- Unsigned Types ---
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

// --- Signed Types ---
typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed int         int32_t;
typedef signed long long   int64_t;

// --- Standard Definitions ---
typedef uint32_t           size_t;
typedef uint32_t           uintptr_t;

#ifndef NULL
    #define NULL 0
#endif
