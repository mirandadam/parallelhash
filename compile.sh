#!/bin/bash

#gprof:
#gcc -pg -g2 -c -O0 -Wall -Ilib lib/*.c
#g++ -pg -g2 -c -O0 -Wall -fopenmp -I. -Ilib *.cpp
#g++ -pg -g2 -O0 -Wall -fopenmp -o parallelhash *.o

#gcov:
#gcc -c -fprofile-arcs -ftest-coverage -g2 -Wall -Ilib lib/*.c
#g++ -c -fprofile-arcs -ftest-coverage -g2 -Wall -fopenmp -I. -Ilib *.cpp
#g++ -fprofile-arcs -ftest-coverage -g2 -Wall -fopenmp -o parallel_hash *.o

#standard:
gcc -c -O2 -Wall -Ilib lib/*.c
g++ -c -O2 -Wall -fopenmp -I. -Ilib *.cpp
g++ -O2 -Wall -fopenmp -o parallelhash *.o

rm *.o
