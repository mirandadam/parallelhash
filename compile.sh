#!/bin/bash

GCC=gcc-4.4
GXX=g++-4.4

#degug:
#$GCC -c -O0 -g2 -Wall -Isrc/lib src/lib/*.c
#$GXX -c -O0 -g2 -Wall -fopenmp -Isrc -Isrc/lib src/*.cpp
#$GXX -O0 -g2 -Wall -fopenmp -o parallelhash *.o

#standard:
$GCC -c -O2 -Wall -Isrc/lib src/lib/*.c
$GXX -c -O2 -Wall -fopenmp -Isrc -Isrc/lib src/*.cpp
$GXX -O2 -Wall -fopenmp -o parallelhash *.o

rm *.o
