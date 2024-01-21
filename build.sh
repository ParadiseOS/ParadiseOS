#!/usr/bin/env sh

docker run                                                        \
    --mount type=bind,source="$(pwd)"/build,target=/usr/app/build \
    --mount type=bind,source="$(pwd)"/src,target=/usr/app/src     \
    -it                                                           \
    paradise-os
