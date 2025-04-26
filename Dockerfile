FROM debian:bullseye-slim

RUN apt update && apt install -y wget grub-pc-bin xorriso build-essential fasm \
    bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo

WORKDIR /usr
RUN mkdir -p app/cross-compiler
WORKDIR app/cross-compiler

# Fetch and extract source code for binutils and gcc
RUN wget "https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.gz"
RUN wget "https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.gz"
RUN tar -xzf "gcc-13.2.0.tar.gz"
RUN tar -xzf "binutils-2.41.tar.gz"
RUN rm "gcc-13.2.0.tar.gz"
RUN rm "binutils-2.41.tar.gz"

ARG PREFIX=/usr/app/cross-compiler
ARG TARGET=i686-elf
ARG PATH=$PREFIX/bin:$PATH

RUN mkdir binutils
RUN mkdir gcc

# Build binutils
WORKDIR binutils
RUN ../binutils-2.41/configure --target=$TARGET --prefix=$PREFIX \
    --with-sysroot --disable-nls --disable-werror
RUN make
RUN make install

# Build gcc
WORKDIR ../gcc
RUN ../gcc-13.2.0/configure --target=$TARGET --prefix=$PREFIX --disable-nls \
    --enable-languages=c --without-headers
RUN make -j 8 all-gcc
RUN make all-target-libgcc
RUN make install-gcc
RUN make install-target-libgcc

WORKDIR /usr/app
RUN mkdir scripts
RUN mkdir elf2sun
RUN mkdir bin

RUN mkdir -p paradise-os/boot/grub

ARG TESTS_ENABLED
ARG BUILD_PROGRAMS

CMD ["sh", "scripts/build.sh"]
