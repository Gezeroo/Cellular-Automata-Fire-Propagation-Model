#!/bin/bash

OUTPUT="main"
SOURCE="cellularAutomata.c"

CFLAGS="-Wall -std=c99 -O2"
LIBS="-lraylib -lm -ldl -lpthread -lGL -lrt -lX11"

gcc $CFLAGS $SOURCE -o $OUTPUT $LIBS

if [ $? -eq 0 ]; then
    echo "Compilado com sucesso. Rodando..."
    ./main
else
    echo "Erro na compilação."
fi