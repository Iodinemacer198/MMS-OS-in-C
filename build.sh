#!/bin/bash
set -e

if ! command -v grub-mkrescue >/dev/null 2>&1; then
    echo "Error: grub-mkrescue is required to build a bootable ISO."
    echo "Install on Debian/Ubuntu with: sudo apt install grub-pc-bin grub-efi-amd64-bin xorriso mtools syslinux-utils"
    exit 1
fi

mkdir -p iso/build
mkdir -p iso/root/boot
mkdir -p iso/output

CFLAGS="-m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -Wall"

echo "Compiling kernel (32-bit)..."
gcc $CFLAGS -Ikernel/fs -Ikernel/commands -Ikernel/commands/calc -Ikernel/commands/login -Ikernel/commands/fs -Ikernel/commands/vgag -Ikernel/commands/wordle -Ikernel/commands/tcc -c kernel/kernel.c -o iso/build/kernel.o

echo "Compiling file system drivers..."
gcc $CFLAGS -c kernel/fs/ata.c -o iso/build/ata.o
gcc $CFLAGS -c kernel/fs/fs.c -o iso/build/fs.o
gcc $CFLAGS -c kernel/commands/calc/calc.c -o iso/build/calc.o
gcc $CFLAGS -c kernel/commands/login/login.c -o iso/build/login.o
gcc $CFLAGS -c kernel/commands/wordle/wordle.c -o iso/build/wordle.o
gcc $CFLAGS -c kernel/commands/fs/fsc.c -o iso/build/fsc.o
gcc $CFLAGS -c kernel/commands/vgag/vgag.c -o iso/build/vgag.o
gcc $CFLAGS -Ikernel/commands/tcc -c kernel/commands/tcc/tinycc.c -o iso/build/tinycc.o

echo "Assembling boot code..."
nasm -f elf32 boot/boot.asm -o iso/build/boot.o

echo "Linking kernel..."
ld -m elf_i386 -T linker.ld -o iso/build/kernel.bin \
iso/build/boot.o iso/build/kernel.o iso/build/ata.o iso/build/fs.o iso/build/calc.o iso/build/login.o iso/build/wordle.o iso/build/fsc.o iso/build/tinycc.o iso/build/vgag.o

echo "Copying kernel..."
cp iso/build/kernel.bin iso/root/boot/kernel.bin

echo "Building ISO..."
grub-mkrescue -o iso/output/mms-os.iso iso/root

if command -v isohybrid >/dev/null 2>&1; then
    echo "Applying isohybrid post-processing..."
    if ! isohybrid --uefi iso/output/mms-os.iso 2>/dev/null; then
        isohybrid iso/output/mms-os.iso
    fi
else
    echo "Warning: isohybrid not found; ISO may still boot, but USB boot compatibility can be lower on some firmware."
fi

if command -v isoinfo >/dev/null 2>&1; then
    echo "Verifying El Torito boot entries..."
    isoinfo -d -i iso/output/mms-os.iso | grep -q "El Torito"
fi

if command -v fdisk >/dev/null 2>&1; then
    echo "Verifying hybrid partition map..."
    fdisk -l iso/output/mms-os.iso >/dev/null
fi

echo "Build complete! Output: iso/output/mms-os.iso"
echo "This ISO is hybrid and can be written directly to a USB drive with:"
echo "sudo dd if=iso/output/mms-os.iso of=/dev/sdX bs=4M status=progress oflag=sync"
echo "If firmware still skips USB, disable Secure Boot and pick the USB entry explicitly in the boot menu."
