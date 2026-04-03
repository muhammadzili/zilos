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
    
    ; Address fields (not needed if not using a special loader format)
    dd 0
    dd 0
    dd 0
    dd 0
    dd 0
    
    ; Graphics fields (starts at offset 32)
    dd 1 ; mode_type (1 = EGA text mode)
    dd 80 ; width
    dd 25  ; height
    dd 0   ; depth (bpp)


section .bss
align 16
stack_bottom:
    resb 65536 ; 64 KiB
stack_top:

section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top
    push ebx ; pointer to multiboot info
    push eax ; magic number
    call kernel_main
    cli
.hang:
    hlt
    jmp .hang
