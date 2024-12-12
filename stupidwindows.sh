echo "compiling IDO...."
gcc -g src/tvm.c src/compiler.c src/main.c src/memory.c src/instruction.c src/table.c src/lexer.c src/sysinf.c src/object.c src/value.c && ./a