#!/usr/bin/env sh

set -e

export PATH="/usr/app/cross-compiler/bin:$PATH"

i686-elf-as src/boot.s -o bin/boot.o
i686-elf-gcc -c src/kernel.c -o bin/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
i686-elf-gcc -c src/terminal.c -o bin/terminal.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
i686-elf-gcc -T src/linker.ld -o build/paradise-os.bin -ffreestanding -O2 -nostdlib bin/boot.o bin/kernel.o bin/terminal.o -lgcc

if ! grub-file --is-x86-multiboot build/paradise-os.bin; then
    echo "Failed to build bootable cdrom: paradise-os.bin is not multiboot"
    exit 1
fi

cp build/paradise-os.bin paradise-os/boot
grub-mkrescue -o build/paradise-os.iso paradise-os
