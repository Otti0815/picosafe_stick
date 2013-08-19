#!/bin/sh

. /opt/eldk-5.2.1/picosafe.sh

cmake .. -DCMAKE_TOOLCHAIN_FILE=./crosscompile.cmake -DENABLE_XSLT=off -DENABLE_TOMAHAWK=off -DENABLE_TOOLKIT=off -DENABLE_SSL=ON -DENABLE_RPROXY=off -DENABLE_IPV6=off
make

# copy polarssl library
cp -a polarssl/library/libpolarssl.so* ../../initramfs-root/lib/

# copy binary to initramfs
cp hiawatha ../../initramfs-root/usr/bin
