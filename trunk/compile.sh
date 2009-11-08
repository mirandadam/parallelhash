#!/bin/bash

GCC="gcc-4.4"
GXX="g++-4.4"
WFLAGS="-Wall -Wextra"
OFLAGS="-O3 -g0 -mtune=native"
DFLAGS="-O0 -g3"

#debug:
#$GCC $WFLAGS $DFLAGS -c -Isrc/lib src/lib/*.c
#$GXX $WFLAGS $DFLAGS -fopenmp -c -Isrc -Isrc/lib src/*.cpp
#$GXX $WFLAGS $DFLAGS -fopenmp -o parallelhash *.o

#standard:
$GCC $WFLAGS $OFLAGS -c -Isrc/lib src/lib/*.c
$GXX $WFLAGS $OFLAGS -fopenmp -c -Isrc -Isrc/lib src/*.cpp
$GXX $WFLAGS $OFLAGS -fopenmp -o parallelhash *.o

rm *.o
