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

To build with different log levels add flag `--log [level]` or `-L [level]`

- There are 3 log levels: CRITICAL, INFO, DEBUG (in precedence order)

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
