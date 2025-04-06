run *args:
    qemu-system-i386 {{args}} -cdrom build/paradise-os.iso &

run-debug:
    qemu-system-i386 -s -S -cdrom build/paradise-os.iso &

run-gdb:
    gdb -x gdb-init.txt

build *args:
    ./build.sh {{args}}

build-cc:
    docker build -t paradise-os .
