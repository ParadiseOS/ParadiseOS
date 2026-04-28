#!/usr/bin/env sh
set -e
export PATH="/usr/app/cross-compiler/bin:$PATH"

MESON_ARGS="-Dtests_enabled=false"
FORCE_SETUP=false

### Arguments Checker

while [ "$#" -gt 0 ];
do
    case ${1} in
    -S|--setup)
        FORCE_SETUP=true ;;
    -t|--tests)
        MESON_ARGS="$(echo $MESON_ARGS | sed 's/-Dtests_enabled=false/-Dtests_enabled=true/')" ;;
    -L|--log)
            if [ -n "$2" ] && ! expr "$2" : '-.*' > /dev/null; then
                MESON_ARGS="$MESON_ARGS -Dlog_level=$(echo "$2" | tr '[:lower:]' '[:upper:]')"
                shift 
            else
                echo "Error: --Log option requires a level (e.g., INFO, DEBUG, CRITICAL)."
                exit 1
            fi
            ;;
    *)
        echo "Invalid option: $1"
        exit 1 ;;
    esac
    shift
done

# Build system stuff
if [ "$FORCE_SETUP" = true ]; then
    rm -rf build/* build/.* 2>/dev/null || true
    meson setup build --cross-file x86_32.txt $MESON_ARGS
elif [ ! -f "build/build.ninja" ]; then
    meson setup build --cross-file x86_32.txt $MESON_ARGS
else
    meson configure build $MESON_ARGS
fi

ninja -C build
