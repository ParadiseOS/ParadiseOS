# ParadiseOS

**Building the cross compiler**

``` sh
docker build -t paradise-os .
```

**Building the OS**

``` sh
./build.sh
```

To build with tests enabled add flag `--tests` or `-t`

To build elf2sun programs before paradise add flag `--build_programs` or `-b`

To build libp for use with programs add flag `--libp` or `-l`

**Running the OS**

``` sh
qemu-system-i386 -cdrom build/paradise-os.iso
qemu-system-i386 build/paradise-os.iso -serial file:logging/log_$(date +%Y-%m-%d_%H-%M-%S).log #With logging
```

**Debugging the OS**

``` sh
qemu-system-i386 -s -S -cdrom build/paradise-os.iso &
gdb -x gdb-init.txt
```
