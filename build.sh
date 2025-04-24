#!/usr/bin/env sh

### Argument Default Values

TESTS_ENABLED=false

### Arguments Checker

while [ "$#" -gt 0 ];
do
    case ${1} in
    -t|--tests)
        TESTS_ENABLED=true ;;
    *)
        echo "Invalid option: $1"
        exit 1 ;;
    esac
    shift
done

### Builds Operating System
docker run                                                                   \
    --mount type=bind,source="$(pwd)"/build,target=/usr/app/build            \
    --mount type=bind,source="$(pwd)"/src,target=/usr/app/src                \
    --mount type=bind,source="$(pwd)"/docker-include,target=/usr/app/scripts \
    -it -e TESTS_ENABLED=${TESTS_ENABLED} paradise-os
