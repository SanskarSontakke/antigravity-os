GPPPARAMS = -m32 -ffreestanding -fno-exceptions -fno-rtti -nostdlib -Isrc -mno-sse -mno-sse2 -mno-mmx
ASMPARAMS = -f elf32
LDPARAMS  = -melf_i386 -T linker.ld

objects = src/boot.o src/kernel.o src/core/mm/kheap.o src/core/gdt.o src/core/interrupts.o src/core/interrupts_asm.o \
          src/drivers/keyboard.o src/drivers/mouse.o src/drivers/rtc.o src/drivers/ata.o \
          src/core/fs/sfs.o src/core/fs/ext4.o \
          src/core/shell/command_registry.o src/core/shell/shell.o src/core/shell/Editor.o \
          src/core/paging.o src/core/graphics/console.o src/core/gui/desktop.o src/core/gui/TerminalWindow.o

run: myos.iso disk.img
	qemu-system-i386 -cdrom myos.iso -drive file=disk.img,format=raw,index=0,media=disk -vga std -serial stdio > qemu.log 2>&1

# NEW: Rule to create disk.img if it doesn't exist
disk.img:
	dd if=/dev/zero of=disk.img bs=1M count=128
	/sbin/mkfs.ext4 -F disk.img


# User Program Build
programs/init.bin: programs/start.asm programs/main.cpp
	nasm -f elf32 programs/start.asm -o programs/start.o
	g++ -m32 -ffreestanding -fno-exceptions -fno-rtti -mno-sse -mno-sse2 -mno-mmx -c programs/main.cpp -o programs/main.o
	# 1. Link to ELF first (to resolve addresses)
	ld -melf_i386 -T programs/link.ld -o programs/init.elf programs/start.o programs/main.o
	# 2. Extract Raw Binary (Crucial Step!)
	objcopy -O binary programs/init.elf programs/init.bin

myos.iso: myos.bin programs/init.bin
	mkdir -p isodir/boot/grub
	cp myos.bin isodir/boot/
	cp programs/init.bin isodir/boot/
	echo 'menuentry "Antigravity" {' > isodir/boot/grub/grub.cfg
	echo '  multiboot /boot/myos.bin' >> isodir/boot/grub/grub.cfg
	echo '  module /boot/init.bin' >> isodir/boot/grub/grub.cfg
	echo '  boot' >> isodir/boot/grub/grub.cfg
	echo '}' >> isodir/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso isodir

myos.bin: $(objects)
	ld $(LDPARAMS) -o myos.bin $(objects)

%.o: %.cpp
	g++ $(GPPPARAMS) -c -o $@ $<

%.o: %.asm
	nasm $(ASMPARAMS) -o $@ $<

clean:
	rm -rf src/*.o src/core/mm/*.o myos.bin myos.iso isodir
