#!/usr/bin/env sh

### Builds Operating System
docker run                                                                   \
    --mount type=bind,source="$(pwd)"/build,target=/usr/app/build            \
    --mount type=bind,source="$(pwd)"/src,target=/usr/app/src                \
    --mount type=bind,source="$(pwd)"/include,target=/usr/app/include            \
    --mount type=bind,source="$(pwd)"/scripts,target=/usr/app/scripts \
    --mount type=bind,source="$(pwd)"/elf2sun,target=/usr/app/elf2sun \
    --mount type=bind,source="$(pwd)"/libp,target=/usr/app/libp \
    --mount type=bind,source="$(pwd)",target=/workspace \
    -w /workspace \
    -it paradise-os sh /usr/app/scripts/build.sh "$@"
