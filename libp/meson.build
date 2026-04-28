# Include Path and Flags
libp_inc = include_directories('include', '../include')
libp_c_args = ['-fno-pic', '-fno-asynchronous-unwind-tables', '-m32', '-masm=intel']
cc = meson.get_compiler('c', native: true)
fasm = find_program('fasm')
ld = find_program('ld')

# Libp Sources
libp_sources = files(
    'src/pio.c',
    'src/pmail.c',
    'src/pstring.c',
    'src/psys.c',
    'src/psyscall.c',
)

# Libp Math Compilation
pmath_obj = custom_target(
    'libpm.o',
    input: 'src/pmath.c',
    output: 'libpm.o',
    command: cc.cmd_array() + ['-c', '@INPUT@', '-o', '@OUTPUT@', '-I../libp/include', '-I../include/paradise/shared'] + libp_c_args,
    build_by_default: true
)

# Libp Start Compilation
start_obj = custom_target(
    'start.o',
    input: 'src/start.s',
    output: 'start.o',
    command: [fasm, '@INPUT@', '@OUTPUT@'],
    build_by_default: true
)

# Libp Std Compilation
libp_temp = static_library(
    'libp_tmp',
    sources: libp_sources,
    include_directories: libp_inc,
    c_args: libp_c_args
)

libp_obj = custom_target(
    'libp.o',
    input: libp_temp.extract_all_objects(recursive: false),
    output: 'libp.o',
    command: [ld, '-r', '-m', 'elf_i386', '@INPUT@', '-o', '@OUTPUT@'],
    build_by_default: true
)
