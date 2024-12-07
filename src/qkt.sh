#!/bin/bash

gcc -g tvm.c value.c memory.c main.c lexer.c instruction.c compiler.c
gdb a.out
