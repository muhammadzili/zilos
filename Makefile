CXX = g++
AS = nasm
LD = ld

CXXFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -Isrc/include
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

# source lists with new hierarchical structure fr
SRC_CXX = $(wildcard src/kernel/*.cpp) \
          $(wildcard src/arch/i386/*.cpp) \
          $(wildcard src/drivers/storage/*.cpp) \
          $(wildcard src/drivers/net/*.cpp) \
          $(wildcard src/drivers/input/*.cpp) \
          $(wildcard src/drivers/video/*.cpp) \
          $(wildcard src/drivers/bus/*.cpp) \
          $(wildcard src/hal/*.cpp) \
          $(wildcard src/fs/*.cpp) \
          $(wildcard src/net/*.cpp) \
          $(wildcard src/user/apps/*.cpp) \
          $(wildcard src/user/cli/*.cpp) \
          $(wildcard src/user/net/*.cpp)

OBJ_CXX = $(patsubst src/%.cpp, build/%.o, $(SRC_CXX))

# assembly objects relocated fr
OBJ_ASM = build/arch/i386/boot.o \
          build/arch/i386/gdt_asm.o \
          build/arch/i386/interrupts_asm.o

OBJ = $(OBJ_ASM) $(OBJ_CXX)

.PHONY: all clean run run-net iso run-iso run-iso-net

all: zilos.bin zilos.img

zilos.bin: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

# shared rules for compiling cpp idc about subfolders
build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# assembly rules fr
build/arch/i386/boot.o: src/arch/i386/boot.s
	@mkdir -p $(dir $@)
	$(AS) -f elf32 $< -o $@

build/arch/i386/gdt_asm.o: src/arch/i386/gdt.s
	@mkdir -p $(dir $@)
	$(AS) -f elf32 $< -o $@

build/arch/i386/interrupts_asm.o: src/arch/i386/interrupts.s
	@mkdir -p $(dir $@)
	$(AS) -f elf32 $< -o $@

# disk management idc
zilos.img:
	@if [ ! -f zilos.img ]; then \
		echo "Creating new zilos.img..."; \
		dd if=/dev/zero of=zilos.img bs=1M count=64; \
	fi

clean:
	rm -rf build zilos.bin zilos.iso iso

run: zilos.bin zilos.img
	qemu-system-i386 -kernel zilos.bin -drive file=zilos.img,format=raw,index=0,media=disk

run-net: zilos.bin zilos.img
	qemu-system-i386 -kernel zilos.bin -drive file=zilos.img,format=raw,index=0,media=disk -netdev user,id=n0 -device rtl8139,netdev=n0

iso: zilos.bin
	mkdir -p iso/boot/grub
	cp zilos.bin iso/boot/
	echo 'menuentry "ZilOS" { multiboot /boot/zilos.bin }' > iso/boot/grub/grub.cfg
	grub-mkrescue -o zilos.iso iso
