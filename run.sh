#!/bin/bash
gcc -g3 -Og -Wall -Wextra main.c -lm -pthread && time ./a.out > rez.out && less rez.out
