#!/bin/bash

make clean
make run

# Todo: fix these
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
elif [[ "$OSTYPE" == "darwin"* ]]; then
elif [[ "$OSTYPE" == "cygwin" ]]; then
elif [[ "$OSTYPE" == "msys" ]]; then
elif [[ "$OSTYPE" == "win32" ]]; then
elif [[ "$OSTYPE" == "freebsd"* ]]; then
else
    #unknown operating system
fi