# libp

The ParadiseOS Standard Library

## Build

You can build libp with [build_libp](build_libp.sh). This will generate the `libp.o` object file.

To build libp math functions run the above script with `-lm`.

To run the libp unit tests run [libp_test](libp_test.sh). By default the files that pass and tests that fail will appear in the terminal.

**Testing Flags**

| Flag          | Short | Description                        |
|---------------|-------|------------------------------------|
| `--simple`    | `-s`  | Simple testing output              |
| `--compact`   | `-c`  | Standard test output (default)     |
| `--verbose`   | `-v`  | Enables verbose prints             |
| `--files`     | `-f`  | Test specific files                |
| `--no_assert` | `-a`  | Turns off assert prints in tests   |

For more testing information refer to the [README](test/README.md).

## Usage

### For ParadiseOS

If you build ParadiseOS with the `--libp` flag it will build the object file and add it to the elf2sun subdirectory for usage with elf2sun programs.

### For Other Applications

Follow [Build](#build) to build libp. This will generate an object file which you can then compile with other source code.
