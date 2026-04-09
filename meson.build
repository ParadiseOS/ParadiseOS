project('ParadiseOS', 'c',
    version: '0.1',
    default_options: ['c_std=gnu99', 'warning_level=2']
)
add_project_arguments('-ffreestanding', '-masm=intel', language: 'c')

if get_option('tests_enabled')
    add_project_arguments('-DTESTS_ENABLED', language: 'c')
endif

# Helper Modules
if get_option('build_libp')
    subdir('libp')
endif

if get_option('build_programs')
    subdir('elf2sun')
endif


log_level = get_option('log_level')
if log_level == 'INFO'
    add_project_arguments('-DLOG_INFO', language: 'c')
elif log_level == 'DEBUG'
    add_project_arguments('-DLOG_DEBUG', language: 'c')
elif log_level == 'CRITICAL'
    add_project_arguments('-DLOG_CRITICAL', language: 'c')
endif

# Kernel Sources
c_sources = files(
    'src/drivers/keyboard/keyboard.c',
    'src/drivers/serial/serial.c',
    'src/drivers/timer/timer.c',
    'src/interrupts/interrupt.c',
    'src/ipc/mailbox.c',
    'src/kernel/init.c',
    'src/kernel/kernel.c',
    'src/lib/error.c',
    'src/lib/libp.c',
    'src/lib/logging.c',
    'src/lib/strings.c',
    'src/lib/util.c',
    'src/memory/heap.c',
    'src/memory/mem.c',
    'src/process/pool.c',
    'src/process/processes.c',
    'src/process/queue.c',
    'src/process/rb_tree.c',
    'src/sun/sun.c',
    'src/syscall/syscall.c',
    'src/terminal/terminal.c',
    'src/tests/allocator_test.c',
    'src/tests/testing.c',
)

s_sources = files(
    'src/boot/boot.s',
    'src/boot/start.s',
    'src/interrupts/interrupt.s',
    'src/kernel/init.s',
    'src/lib/util.s',
    'src/memory/mem.s',
    'src/process/processes.s',
)

# Run Fasm on asm files
fasm = find_program('fasm')
fasm_gen = generator(fasm,
    output: '@BASENAME@.s.o',
    arguments: ['@INPUT@', '@OUTPUT@']
)
asm_objects = fasm_gen.process(s_sources)

# Sun.s
sun_deps = []
sun_bin_path = 'binary.sun'

if get_option('build_programs')
    sun_deps += user_programs
    sun_bin_path = user_programs
endif

sun_obj = custom_target('sun.s.o',
    input: 'src/sun/sun.s',
    output: 'sun.s.o',
    command: ['sh', '-c', 'cp "$1" . && fasm "$2" "$3"', '--', sun_bin_path, '@INPUT@', '@OUTPUT@'],
    depends: sun_deps,
    build_by_default: true
)


# Build Kernel
kernel_elf = executable('paradise-os.bin',
    sources: [c_sources, asm_objects, sun_obj],
    include_directories: 'include',
    link_args: ['-T', meson.current_source_dir() / 'src/boot/linker.ld', '-nostdlib', '-lgcc']
)

# Iso Generation
grub_rescue = find_program('grub-mkrescue')
custom_target(
    'paradise_iso',
    input: [kernel_elf, 'scripts/grub.cfg'],
    output: 'paradise-os.iso',
    command: [
        'sh', '-c',
        '''
        mkdir -p iso/boot/grub 
        cp "$1" iso/boot/ 
        cp "$2" iso/boot/grub/ 
        grub-mkrescue -o "$3" iso
        ''',
        '--',
        '@INPUT0@', # kernel_elf
        '@INPUT1@', # scripts/grub.cfg
        '@OUTPUT@'
    ],
    build_by_default: true
)