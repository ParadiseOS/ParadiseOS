# ParadiseOS

**Building the cross compiler**

``` sh
docker build -t paradise-os .
```

**Building the OS**

``` sh
./build.sh
```

**Running the OS**

``` sh
qemu-system-i386 -cdrom build/paradise-os.iso
```

**Debugging the OS**

``` sh
qemu-system-i386 -s -S -cdrom build/paradise-os.iso &
gdb -x gdb-init.txt
```
