#!/bin/bash

# Kyle Snow 10/16/19
#
# Bash script to compile a given kernel.c and flash it to 
# a new disk diskc.img. 
#---------------------------------------------------------
# Requires: bootload.asm && project applicable kernel.asm
#


# assemble bootload.asm
FILE=bootload
if [ -e "$FILE" ]; then
    echo -e "\e[96mbootload exists. Skipping assembly."
else
    echo -e "\e[96mAssembling bootload..."
    nasm bootload.asm
fi


# If directory artifacts does not exist, create one
mkdir -p artifacts

#create a blank diskc.img and append the bootloader
echo -e "\e[96m(Re-)Creating diskc.img..."
dd if=/dev/zero of=diskc.img bs=512 count=1000
echo -e "\e[96mAppending bootloader..."
dd if=bootload of=diskc.img bs=512 count=1 conv=notrunc
if [ $? -ne 0 ];then
    echo -e "\e[96mFailed to append bootloader. do you have bootload?"
    exit 1
fi

#Start compilation/linking of kernel.c
echo -e "\e[96mCreating kernel and associated artifacts...\e[33m"

bcc kernel.c -ansi -c -o kernel.o
if [ $? -ne 0 ]; then
     echo -e "\e[31mFailed on compilation of kernel.o!"
     exit 2
fi

as86 kernel.asm -o kernel_asm.o
if [ $? -ne 0 ]; then
     echo -e "\e[31mFailed on assembly of kernel_asm.o!"
     exit 3

fi
ld86 -o kernel -d kernel.o kernel_asm.o
if [ $? -ne 0 ]; then
     echo -e "\e[31mFailed on linking kernel!"
     exit 4
fi


#Compile shell
echo -e "\e[96mCreating shell and associated artifacts...\e[33m"

bcc shell.c -ansi -c -o shell.o
if [ $? -ne 0 ]; then
     echo -e "\e[31mFailed on compilation of shell.o!"
     exit 5
fi

as86 userlib.asm -o userlib.o
if [ $? -ne 0 ]; then
     echo -e "\e[31mFailed on assembly of userlib.o!"
     exit 6

fi
ld86 -o shell -d shell.o userlib.o
if [ $? -ne 0 ]; then
     echo -e "\e[31mFailed on linking shell!"
     exit 7
fi




#load programs to diskc.img
echo -e "\e[96mloading programs..."
./loadFile kernel
./loadFile message.txt
./loadFile tstpr1
./loadFile tstpr2
./loadFile shell



echo -e "\e[96mCleaning up..."
mv kernel.o artifacts
mv kernel_asm.o artifacts
mv userlib.o artifacts
mv shell.o artifacts


echo -e "\e[96mDone."
echo -e "\e39m" #reset to default

exit 0
