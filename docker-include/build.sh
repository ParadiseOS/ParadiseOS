#!/usr/bin/env sh

set -e

export PATH="/usr/app/cross-compiler/bin:$PATH"

if [ "$TESTS_ENABLED" = "true" ]; then
    TESTS_FLAG="-DTESTS_ENABLED"
else
    TESTS_FLAG=""
fi

for file in src/*.c; do
    if [ "$file" = "src/kernel.c" ]; then
        i686-elf-gcc -c "$file" -o "bin/$(basename -s .c $file).o" -std=gnu99 -ffreestanding -g -Wall -Wextra $TESTS_FLAG
    else
        i686-elf-gcc -c "$file" -o "bin/$(basename -s .c $file).o" -std=gnu99 -ffreestanding -g -Wall -Wextra
    fi
done

fasm src/boot.s bin/boot.o
i686-elf-gcc -T src/linker.ld -o build/paradise-os.bin -ffreestanding -nostdlib bin/*.o -lgcc

if ! grub-file --is-x86-multiboot build/paradise-os.bin; then
    echo "Failed to build bootable cdrom: paradise-os.bin is not multiboot"
    exit 1
fi

cp build/paradise-os.bin paradise-os/boot
grub-mkrescue -o build/paradise-os.iso paradise-os