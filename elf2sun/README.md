# elf2sun

Turns 32-bit elf executables into sun executables

## How to build

Run the following command build elf2sun.

```bash
gcc elf2sun.c -o elf2sun
```

## How to run

Use [`build_programs.sh`](build_programs.sh) to convert all .c files in `programs/` into 32-bit elf executables that are put into `build/`.

After you have executables in `build/` run elf2sun on all executables using the following command.

```bash
gcc elf2sun build/*
```
