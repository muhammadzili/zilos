#include "kernel/scheduler.hpp"
#include "kernel/memory.hpp"
#include "arch/i386/timer.hpp"

PCB* current = 0;
PCB* ready_queue = 0;
uint32_t next_pid = 1;
bool initialized = false;

static PCB tasks[MAX_TASKS];
static uint32_t task_count = 0;

void initialize() {
    if (initialized) return;
    
    current = 0;
    ready_queue = 0;
    task_count = 0;
    next_pid = 1;
    
    PCB* idle = &tasks[0];
    idle->pid = next_pid++;
    idle->state = TASK_READY;
    idle->priority = 0;
    idle->time_slice = 10;
    idle->ticks = 0;
    idle->stack_base = 0;
    idle->stack_ptr = 0;
    idle->kernel_mode = true;
    idle->name[0] = 'I';
    idle->name[1] = 'D';
    idle->name[2] = 'L';
    idle->name[3] = 'E';
    idle->name[4] = '\0';
    idle->next = 0;
    
    current = idle;
    ready_queue = idle;
    task_count = 1;
    
    initialized = true;
}

uint32_t create_task(void (*entry)(void), const char* name, uint32_t stack_size) {
    if (task_count >= MAX_TASKS) return 0;
    
    PCB* task = &tasks[task_count++];
    task->pid = next_pid++;
    task->state = TASK_READY;
    task->priority = 1;
    task->time_slice = 20;
    task->ticks = 0;
    task->kernel_mode = true;
    
    uint32_t i = 0;
    while (name[i] && i < 31) {
        task->name[i] = name[i];
        i++;
    }
    task->name[i] = '\0';
    
    task->stack_base = (uint32_t)Memory::Heap::allocate(stack_size);
    task->stack_ptr = task->stack_base + stack_size - 4;
    
    uint32_t* stack = (uint32_t*)task->stack_ptr;
    *--stack = (uint32_t)entry;
    *--stack = 0;
    
    task->next = ready_queue;
    ready_queue = task;
    
    return task->pid;
}

void schedule() {
    if (!current || !ready_queue) return;
    
    PCB* next = ready_queue;
    while (next && next->next != current && next->next != 0) {
        next = next->next;
    }
    
    if (current && current->state == TASK_RUNNING) {
        current->state = TASK_READY;
    }
    
    if (next && next->next) {
        next->next = current->next;
    } else {
        ready_queue = current->next;
    }
    
    if (current && current->state == TASK_READY) {
        PCB* tail = next;
        while (tail && tail->next) tail = tail->next;
        if (tail) tail->next = current;
        else ready_queue = current;
    }
    
    current = ready_queue;
    if (current) {
        current->state = TASK_RUNNING;
    }
}

void yield() {
    schedule();
}

void sleep(uint32_t ms) {
    if (current) {
        current->state = TASK_BLOCKED;
    }
    schedule();
}

PCB* get_current() {
    return current;
}

void tick() {
    if (current) {
        current->ticks++;
        if (current->ticks >= current->time_slice) {
            current->ticks = 0;
            yield();
        }
    }
}

PCB* get_task(uint32_t pid) {
    for (uint32_t i = 0; i < task_count; i++) {
        if (tasks[i].pid == pid) return &tasks[i];
    }
    return 0;
}

PCB* get_all_tasks() {
    return tasks;
}

uint32_t get_task_count() {
    return task_count;
}

bool kill_task(uint32_t pid) {
    if (pid == 1) return false;
    for (uint32_t i = 0; i < task_count; i++) {
        if (tasks[i].pid == pid) {
            tasks[i].state = TASK_DEAD;
            return true;
        }
    }
    return false;
}

extern "C" void context_switch(PCB* old_task, PCB* new_task) {
    if (!old_task || !new_task) return;
    
    asm volatile (
        "mov %%esp, %0"
        : "=r"(old_task->stack_ptr)
    );
    
    new_task->stack_ptr += 4;
    asm volatile (
        "mov %0, %%esp"
        :
        : "r"(new_task->stack_ptr)
    );
}