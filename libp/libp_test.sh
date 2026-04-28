#!/bin/bash

FILES_TO_TEST=()
PRINT_FLAG="-D COMPACT_PRINT" # default mode
NO_ASSERT_PRINT_FLAG=""
IS_SIMPLE_PRINT=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --files|-f)
            shift
            while [[ $# -gt 0 && "$1" != --* && "$1" != -* ]]; do
                FILES_TO_TEST+=("$1")
                shift
            done
            ;;
        --simple|-s)
            PRINT_FLAG="-D SIMPLE_PRINT"
            IS_SIMPLE_PRINT=true
            NO_ASSERT_PRINT_FLAG="-D NO_ASSERT_PRINT"
            shift
            ;;
        --verbose|-v)
            PRINT_FLAG="-D VERBOSE_PRINT"
            IS_SIMPLE_PRINT=false
            shift
            ;;
        --compact|-c)
            PRINT_FLAG="-D COMPACT_PRINT"
            IS_SIMPLE_PRINT=false
            shift
            ;;
        --no_assert|-a)
            NO_ASSERT_PRINT_FLAG="-D NO_ASSERT_PRINT"
            shift
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# If no files specified, use all *_test.c files in test/
if [ ${#FILES_TO_TEST[@]} -eq 0 ]; then
    for file in test/*_test.c; do
        base=$(basename "$file" _test.c)
        FILES_TO_TEST+=("$base")
    done
fi

printf -- "---------------------------------------\n"
printf -- "-------------- libp Test --------------\n"
printf -- "---------------------------------------\n"
echo ""

# Compile and run each test
for base in "${FILES_TO_TEST[@]}"; do
    SRC="src/${base}.c"
    TEST="test/${base}_test.c"
    TESTLIB="test/testlib.c"
    OUTPUT="build/${base}_test.out"

    gcc "$TEST" "$TESTLIB" "$SRC" -I include/ $PRINT_FLAG $NO_ASSERT_PRINT_FLAG -o "$OUTPUT" || {
        echo "Compilation failed for $base"
        continue
    }

    if [ $IS_SIMPLE_PRINT != true ]; then
      echo "Running $base tests"
    fi
    "./$OUTPUT"
    if  [ $IS_SIMPLE_PRINT != true ]; then
        echo ""
    fi
done
