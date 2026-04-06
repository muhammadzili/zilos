MBALIGN  equ  1 << 0
MEMINFO  equ  1 << 1
FLAGS    equ  MBALIGN | MEMINFO
MAGIC    equ  0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
    
    ; multiboot address fields idc
    dd 0, 0, 0, 0, 0
    
    ; vesa/ega mode config fr
    dd 1    ; mode
    dd 80   ; width
    dd 25    ; height
    dd 0    ; bpp

section .bss
align 16
stack_bottom:
    resb 65536 ; 64k kernel stack fr
stack_top:

section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top
    push ebx ; multiboot header ptr
    push eax ; magic value
    call kernel_main
    cli
.hang:
    hlt
    jmp .hang ; system halted fr
