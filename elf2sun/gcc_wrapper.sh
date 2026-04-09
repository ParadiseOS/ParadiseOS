#!/usr/bin/env sh

# Assuming  is ran in program/"program_name"
PROGRAMS_DIR=".." 
ROOT_DIR="../.."
WORKSPACE="/workspace"
BUILD_DIR="$WORKSPACE/build/elf2sun"
LIBP_BUILD_DIR="$WORKSPACE/build/libp"

# Ensure C source file
if [ -z "$1" ] || [ "$1" != *.c ]; then
    echo "USAGE: $0 <source.c>"
    exit 1
fi

SOURCE_FILE="$1"
PROGRAM_NAME=$(basename "$(pwd)") # Program Directory Name
OBJ_FILE="${SOURCE_FILE%.c}.o"
BUILD_FILE="$BUILD_DIR/$PROGRAM_NAME.out"

echo "Building $PROGRAM_NAME with gcc_wrapper.sh"

mkdir -p "$BUILD_DIR"

/usr/bin/gcc \
    -c "$SOURCE_FILE" \
    -o "$OBJ_FILE" \
    -Wall -Wextra \
    -fno-pic \
    -fno-asynchronous-unwind-tables \
    -m32 \
    -masm=intel \
    -I$WORKSPACE/libp/include/

if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

/usr/bin/gcc \
    "$OBJ_FILE" \
    "$LIBP_BUILD_DIR/libp.o" \
    "$LIBP_BUILD_DIR/start.o" \
    -T "$PROGRAMS_DIR/linker.ld" \
    -o "$BUILD_FILE" \
    -m32 -static -nostdlib \
    -Wl,--build-id=none

if [ $? -ne 0 ]; then
    echo -n "Linking failed."
    exit 1
fi

echo "Successfully built $BUILD_FILE"