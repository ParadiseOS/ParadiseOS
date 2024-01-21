# ParadiseOS

**Building the cross compiler**

``` sh
docker build -t paradise-os .
```

**Building the OS**

``` sh
./run.sh
```

**Running the OS**

``` sh
qemu-system-i386 -cdrom build/paradise-os.iso
```
