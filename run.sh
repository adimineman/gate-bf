#!/bin/bash
gcc -g3 -O3 -Wall -Wextra main.c -lm && time ./a.out > rez.out && less rez.out
