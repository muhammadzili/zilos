#include "kernel/scheduler.hpp"
#include "kernel/memory.hpp"
#include "arch/i386/timer.hpp"
#include "drivers/video/vga.hpp"

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
    idle->total_ticks = 0;
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
    VGA::print("[create_task] ");
    VGA::print(name);
    VGA::print(" count=");
    char buf[8];
    VGA::itoa(task_count, buf, 10);
    VGA::println(buf);
    
    if (task_count >= MAX_TASKS) {
        VGA::println("[create_task] MAX_TASKS reached!");
        return 0;
    }
    
    PCB* task = &tasks[task_count++];
    task->pid = next_pid++;
    task->state = TASK_READY;
    task->priority = 1;
    task->time_slice = 20;
    task->ticks = 0;
    task->total_ticks = 0;
    task->kernel_mode = true;
    
    uint32_t i = 0;
    while (name[i] && i < 31) {
        task->name[i] = name[i];
        i++;
    }
    task->name[i] = '\0';
    
    task->stack_base = (uint32_t)Memory::Heap::allocate(stack_size);
    VGA::print("[create_task] stack_base=");
    VGA::itoa(task->stack_base, buf, 16);
    VGA::println(buf);
    
    if (!task->stack_base) {
        VGA::println("[create_task] FAILED - no heap memory!");
        task_count--;
        return 0;
    }
    
    task->stack_ptr = task->stack_base + stack_size - 4;
    
    uint32_t* stack = (uint32_t*)task->stack_ptr;
    *--stack = (uint32_t)entry;
    *--stack = 0;
    
    task->next = ready_queue;
    ready_queue = task;
    
    VGA::print("[create_task] success, pid=");
    VGA::itoa(task->pid, buf, 10);
    VGA::println(buf);
    
    return task->pid;
}

void schedule() {
    if (!initialized || task_count == 0) return;

    uint32_t start_idx = 0;
    for (uint32_t i = 0; i < task_count; i++) {
        if (&tasks[i] == current) {
            start_idx = i;
            break;
        }
    }

    // Round-robin search for next READY task
    uint32_t i = (start_idx + 1) % task_count;
    PCB* next = &tasks[start_idx]; 

    while (i != start_idx) {
        if (tasks[i].state == TASK_READY || tasks[i].state == TASK_RUNNING) {
            next = &tasks[i];
            break;
        }
        i = (i + 1) % task_count;
    }

    if (current && current->state == TASK_RUNNING) {
        current->state = TASK_READY;
    }

    current = next;
    current->state = TASK_RUNNING;
    
    // Note: full context_switch(old, new) would go here for real multitasking
}

void yield() {
    schedule();
}

void sleep(uint32_t ms) {
    PCB* curr = current;
    if (curr) {
        curr->state = TASK_BLOCKED;
    }
    schedule();
    
    uint32_t start = Timer::get_ticks();
    while (Timer::get_ticks() - start < ms) {
        asm volatile ("hlt");
    }
    
    if (curr) {
        curr->state = TASK_RUNNING;
        current = curr;
    }
}

PCB* get_current() {
    return current;
}

void tick() {
    if (current) {
        current->ticks++;
        current->total_ticks++;
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