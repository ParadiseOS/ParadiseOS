deps = [libp_obj, pmath_obj, start_obj]

user_programs = custom_target('sun_programs',
    output: 'binary.sun',
    depends: deps, 
    command: [
        'sh', '-c',
        '''
        set -e
        BUILD_DIR="$(pwd)"
        OUTPUT="$BUILD_DIR/$2"
        cd "$1"
        bash build_programs.sh
        mv binary.sun "$OUTPUT"
        ''',
        '--', 
        meson.current_source_dir(), 
        '@OUTPUT@'
    ],
    build_by_default: true
)