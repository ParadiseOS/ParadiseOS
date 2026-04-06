#!/bin/bash

set -e

SRC_DIR="src"
BUILD_DIR="build"
BUILD_FLAGS="-Wall -Wextra -fno-pic -fno-asynchronous-unwind-tables -m32 -masm=intel -Iinclude -I../include"

mkdir -p "$BUILD_DIR"

LIBP_OBJS=()

echo "Building libp"

rm -f "$BUILD_DIR"/*.o

# Compile .c files to .o
for file in "$SRC_DIR"/*.c; do
    echo "Compiling $file"

    base=$(basename "$file")
    if [ "$base" = "pmath.c" ]; then
        gcc -c "$file" -o "$BUILD_DIR/libpm.o" $BUILD_FLAGS
        continue
    fi

    obj="$BUILD_DIR/$(basename "${file%.c}.c.o")"
    gcc -c "$file" -o "$obj" $BUILD_FLAGS
    LIBP_OBJS+=("$obj")
done

# Assemble .asm or .s files with fasm or as
for file in "$SRC_DIR"/*.s; do
    echo "Compiling $file"

    base=$(basename "$file")
    if [ "$base" = "start.s" ]; then
        fasm "$file" "$BUILD_DIR/start.o"
        continue
    fi

    obj="$BUILD_DIR/$(basename "${file%.s}.s.o")"
    fasm "$file" "$obj"
    LIBP_OBJS+=("$obj")
done

# Link all objects into a single relocatable .o
ld -r -m elf_i386 "${LIBP_OBJS[@]}" -o "$BUILD_DIR/libp.o"
echo "Built libp"
