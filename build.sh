#!/bin/bash
set -e

mkdir -p iso/build
mkdir -p iso/root/boot
mkdir -p iso/output

echo "Compiling kernel (32-bit)..."
gcc -m32 -ffreestanding -c kernel/kernel.c -o iso/build/kernel.o -O2 -Wall

echo "Assembling boot code..."
nasm -f elf32 boot/boot.asm -o iso/build/boot.o

echo "Linking kernel..."
ld -m elf_i386 -T linker.ld -o iso/build/kernel.bin \
iso/build/boot.o iso/build/kernel.o

echo "Copying kernel..."
cp iso/build/kernel.bin iso/root/boot/kernel.bin

echo "Building ISO..."
grub-mkrescue -o iso/output/mms-os.iso iso/root

echo "Build complete!"