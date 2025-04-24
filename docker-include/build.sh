#!/usr/bin/env sh

set -e

export PATH="/usr/app/cross-compiler/bin:$PATH"

cp scripts/grub.cfg paradise-os/boot/grub

if [ "$TESTS_ENABLED" = "true" ]; then
    TESTS_FLAG="-DTESTS_ENABLED"
else
    TESTS_FLAG=""
fi

find src -type f -name "*.c" | while read -r file; do
    output="bin/$(basename -s .c "$file").o"
    i686-elf-gcc -c "$file" -o "$output" -std=gnu99 -ffreestanding -ggdb -masm=intel -Wall -Wextra -Isrc $TESTS_FLAG
done

find src -type f -name "*.s" | while read -r file; do
    output="bin/$(basename -s .s "$file").s.o"
    fasm "$file" "$output"
done

i686-elf-gcc -T src/boot/linker.ld -o build/paradise-os.bin -ffreestanding -nostdlib bin/*.o -lgcc

if ! grub-file --is-x86-multiboot build/paradise-os.bin; then
    echo "Failed to build bootable cdrom: paradise-os.bin is not multiboot"
    exit 1
fi

cp build/paradise-os.bin paradise-os/boot
grub-mkrescue -o build/paradise-os.iso paradise-os
