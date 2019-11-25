#! /bin/bash

bcc -ansi -c -o tstpr2.o tstpr2.c

as86 -o userlib.o userlib.asm

ld86 -d -o tstpr2 tstpr2.o userlib.o

./loadFile tstpr2



echo "done."
