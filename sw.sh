# stupid windows
echo "compiling IDO...."
rm -rf ./ido
gcc -g src/tvm.c src/compiler.c src/main.c src/memory.c src/instruction.c src/table.c src/lexer.c src/sysinf.c src/object.c src/value.c src/parser.c src/butil.c -o ido && ./ido $@
