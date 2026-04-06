#pragma once

#include "utils/types.hpp"

namespace Panic {

struct RegisterState {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t gs, fs, es, ds;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

typedef void (*PanicHandler)(const char* msg, RegisterState* regs);

void set_handler(PanicHandler handler);
void halt(const char* msg);
void debug(const char* msg);

}