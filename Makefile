CXX = g++
AS = nasm
LD = ld

CXXFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -Isrc
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

SRC_CXX = $(wildcard src/kernel/*.cpp) $(wildcard src/drivers/*.cpp) $(wildcard src/cli/*.cpp) $(wildcard src/fs/*.cpp) $(wildcard src/apps/*.cpp) $(wildcard src/net/*.cpp) $(wildcard src/gui/*.cpp)
OBJ_CXX = $(patsubst src/%.cpp, build/%.o, $(SRC_CXX))

# Add new assembly files
OBJ = build/boot.o build/kernel/gdt_asm.o build/kernel/interrupts_asm.o $(OBJ_CXX)

build/kernel/gdt_asm.o: src/kernel/gdt.s
	mkdir -p build/kernel
	nasm -f elf32 src/kernel/gdt.s -o build/kernel/gdt_asm.o

build/kernel/interrupts_asm.o: src/kernel/interrupts.s
	mkdir -p build/kernel
	nasm -f elf32 src/kernel/interrupts.s -o build/kernel/interrupts_asm.o


.PHONY: all clean run run-net iso run-iso run-iso-net

all: zilos.bin zilos.img

zilos.bin: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

build/boot.o: src/boot/boot.s
	@mkdir -p $(dir $@)
	$(AS) -f elf32 $< -o $@

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create virtual hard disk (64MB) only if it doesn't exist
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

run-iso: iso zilos.img
	qemu-system-i386 -cdrom zilos.iso -drive file=zilos.img,format=raw,index=0,media=disk

run-iso-net: iso zilos.img
	qemu-system-i386 -cdrom zilos.iso -drive file=zilos.img,format=raw,index=0,media=disk -netdev user,id=n0 -device rtl8139,netdev=n0
