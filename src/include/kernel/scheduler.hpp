#pragma once

#include "utils/types.hpp"

struct TaskState {
    uint32_t eax, ebx, ecx, edx, esi, edi, ebp, esp;
    uint32_t eip, eflags;
    uint32_t cs, ds, es, fs, gs, ss;
};

struct PCB {
    uint32_t pid;
    uint32_t state;
    uint32_t priority;
    uint32_t time_slice;
    uint32_t ticks;
    uint32_t stack_base;
    uint32_t stack_ptr;
    bool kernel_mode;
    char name[32];
    PCB* next;
};

const uint32_t TASK_RUNNING = 1;
const uint32_t TASK_READY = 2;
const uint32_t TASK_BLOCKED = 3;
const uint32_t TASK_DEAD = 4;

const uint32_t MAX_TASKS = 16;

extern PCB* current;
extern PCB* ready_queue;
extern uint32_t next_pid;
extern bool initialized;

void initialize();
void schedule();
uint32_t create_task(void (*entry)(void), const char* name, uint32_t stack_size);
void yield();
void sleep(uint32_t ms);
PCB* get_current();
void tick();
PCB* get_task(uint32_t pid);
PCB* get_all_tasks();
uint32_t get_task_count();
bool kill_task(uint32_t pid);

extern "C" void context_switch(PCB* old_task, PCB* new_task);