# When compiling each program elf2sun will attempt to build from a given build.sh in the root of the program subfolder
# Here is an example of the flags needed to build a file main.c in a program folder called example
gcc main.c -T ../linker.ld -o ../../build/example.out -Wall -Wextra -fno-pic -fno-asynchronous-unwind-tables -m32 -static -nostdlib -Wl,--build-id=none

# Here is an example of the building that same file but using the ParadiseOS Standard Library in the program
gcc -c main.c -o main.o -Wall -Wextra -fno-pic -fno-asynchronous-unwind-tables -m32 -masm=intel -I../../../libp/include/
gcc main.o ../libp.o -T ../linker.ld -o ../../build/example.out -m32 -static -nostdlib -Wl,--build-id=none

# Ensure that you use the provided linker and put the exe in build/

# Other object files you can include
# start.o - This is an alternate entry instead of main. You should use this majority of the time.
# libpm.o - This is the ParadiseOS math library

# If typing this a pain you can also use to let paradise build with libp.o and start.o for you
../../gcc_wrapper.sh main.c

# Alternatively you can also omit a build.sh if you only have a main.c file. In which case build_programs.sh should default compiling main.c