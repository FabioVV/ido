#!/bin/bash

gcc -g src/tvm.c src/value.c src/memory.c src/main.c src/lexer.c src/instruction.c src/compiler.c
./a.out
