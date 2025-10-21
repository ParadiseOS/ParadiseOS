#!/usr/bin/env sh

### Argument Default Values

TESTS_ENABLED=false
BUILD_PROGRAMS=false
LIBP=false
LOG_LEVEL=""

### Arguments Checker

while [ "$#" -gt 0 ];
do
    case ${1} in
    -t|--tests)
        TESTS_ENABLED=true ;;
    -b|--build_programs)
        BUILD_PROGRAMS=true ;;
    -l|--libp)
        LIBP=true ;;
	-L|--Log)
			if [ -n "$2" ] && ! expr "$2" : '-.*' > /dev/null; then
                LOG_LEVEL="$2"
                shift 
            else
                echo "Error: --Log option requires a level (e.g., INFO, DEBUG, CRITICAL)."
                exit 1
            fi
            ;;
    *)
        echo "Invalid option: $1"
        exit 1 ;;
    esac
    shift
done

set -e

export PATH="/usr/app/cross-compiler/bin:$PATH"

cp scripts/grub.cfg paradise-os/boot/grub

if [ "$TESTS_ENABLED" = true ]; then
    TESTS_FLAG="-DTESTS_ENABLED"
else
    TESTS_FLAG=""
fi


LOGGING_FLAG=""

if [ -n "$LOG_LEVEL" ]; then
	LOG_LEVEL_UPPER=$(echo "$LOG_LEVEL" | tr '[:lower:]' '[:upper:]')

    case "$LOG_LEVEL_UPPER" in
        "INFO")
            LOGGING_FLAG="-DLOG_INFO"
            ;;
        "DEBUG")
            LOGGING_FLAG="-DLOG_DEBUG"
            ;;
        "CRITICAL")
            LOGGING_FLAG="-DLOG_CRITICAL"
            ;;
        *)
            echo "Warning: Invalid log level '$LOG_LEVEL'. No log flag will be set."
            ;;
    esac
fi

if [ "$LIBP" = true ]; then
    printf -- "---------------------------------------\n"
    printf -- "------------ BUILDING LIBP ------------\n"
    printf -- "---------------------------------------\n"
    cd libp
    ./build_libp.sh
    cd ..
    mv ./libp/build/libp.o ./libp/build/libpm.o ./libp/build/start.o ./elf2sun/programs/
    printf "\n"
fi

if [ "$BUILD_PROGRAMS" = true ]; then
    printf -- "---------------------------------------\n"
    printf -- "---------- BUILDING PROGRAMS ----------\n"
    printf -- "---------------------------------------\n"
    cd elf2sun
    printf "Clearing elf2sun/build.sh\n"
    rm -f build/*.out
    ./build_programs.sh
    cd ..
    mv ./elf2sun/binary.sun ./build
    printf "\n"
fi

printf -- "---------------------------------------\n"
printf -- "---------- BUILDING PARADISE ----------\n"
printf -- "---------------------------------------\n"

find src -type f -name "*.c" | while read -r file; do
    output="bin/$(basename -s .c "$file").o"
    i686-elf-gcc -c "$file" -o "$output" -std=gnu99 -ffreestanding -ggdb -masm=intel -Wall -Wextra -Isrc $TESTS_FLAG $LOGGING_FLAG
done

find src -type f -name "*.s" | while read -r file; do
    output="bin/$(basename -s .s "$file").s.o"
    fasm "$file" "$output"
done

i686-elf-gcc -T src/boot/linker.ld -o build/paradise-os.bin -ffreestanding -nostdlib bin/*.o -lgcc

if ! grub-file --is-x86-multiboot build/paradise-os.bin; then
    printf "Failed to build bootable cdrom: paradise-os.bin is not multiboot"
    exit 1
fi

cp build/paradise-os.bin paradise-os/boot
grub-mkrescue -o build/paradise-os.iso paradise-os
