# ParadiseOS

**Building the cross compiler**

``` sh
docker build paradise-os .
```

**Building the OS**

``` sh
./build.sh
```

To build with tests enabled add flag `--tests` or `-t`

To build elf2sun programs before paradise add flag `--build_programs` or `-b`

**Running the OS**

``` sh
qemu-system-i386 -cdrom build/paradise-os.iso
```

**Debugging the OS**

``` sh
qemu-system-i386 -s -S -cdrom build/paradise-os.iso &
gdb -x gdb-init.txt
```
