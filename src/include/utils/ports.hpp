#pragma once

#include "utils/types.hpp"

/**
 * @file ports.hpp
 * @brief x86 I/O Port Communication
 * 
 * Provides inline assembly wrappers for communicating with hardware 
 * using the 'in' and 'out' instructions.
 */

/**
 * @brief Output a byte to a port
 */
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

/**
 * @brief Input a byte from a port
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

/**
 * @brief Output a word (16-bit) to a port
 */
static inline void outw(uint16_t port, uint16_t val) {
    asm volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) );
}

/**
 * @brief Input a word (16-bit) from a port
 */
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ( "inw %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

/**
 * @brief Output a double word (32-bit) to a port
 */
static inline void outl(uint16_t port, uint32_t val) {
    asm volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) );
}

/**
 * @brief Input a double word (32-bit) from a port
 */
static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile ( "inl %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

/**
 * @brief Small delay for slow I/O operations
 */
static inline void io_wait(void) {
    outb(0x80, 0);
}
