run:
    qemu-system-i386 -cdrom build/paradise-os.iso

run-debug:
    qemu-system-i386 -s -S -cdrom build/paradise-os.iso &

run-gdb:
    gdb -x gdb-init.txt

build:
    ./build.sh

build-test:
    ./build.sh -t

build-cc:
    docker build -t paradise-os .
