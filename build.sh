#!/bin/bash
set -e

mkdir -p iso/build
mkdir -p iso/root/boot
mkdir -p iso/output

echo "Compiling kernel (32-bit)..."
# Added -Ikernel/fs so the compiler can find fs.h and ata.h
gcc -m32 -ffreestanding -Ikernel/fs -Ikernel/commands -c kernel/kernel.c -o iso/build/kernel.o -O2 -Wall

echo "Compiling file system drivers..."
gcc -m32 -ffreestanding -c kernel/fs/ata.c -o iso/build/ata.o -O2 -Wall
gcc -m32 -ffreestanding -c kernel/fs/fs.c -o iso/build/fs.o -O2 -Wall
gcc -m32 -ffreestanding -c kernel/commands/calc.c -o iso/build/calc.o -O2 -Wall
gcc -m32 -ffreestanding -c kernel/commands/login.c -o iso/build/login.o -O2 -Wall

echo "Assembling boot code..."
nasm -f elf32 boot/boot.asm -o iso/build/boot.o

echo "Linking kernel..."
# Added ata.o and fs.o to the linker command
ld -m elf_i386 -T linker.ld -o iso/build/kernel.bin \
iso/build/boot.o iso/build/kernel.o iso/build/ata.o iso/build/fs.o iso/build/calc.o iso/build/login.o

echo "Copying kernel..."
cp iso/build/kernel.bin iso/root/boot/kernel.bin

echo "Building ISO..."
grub-mkrescue -o iso/output/mms-os.iso iso/root

echo "Build complete!"