#!/bin/bash
set -e

FILE="disk.img"

if [ -f "$FILE" ]; then
    qemu-system-x86_64 -boot d -cdrom iso/output/mms-os.iso -drive file=disk.img,format=raw,index=0,media=disk
else
    qemu-img create -f raw disk.img 100M
    mkfs.vfat -F 32 disk.img
    echo "Created filesystem image"
    qemu-system-x86_64 -boot d -cdrom iso/output/mms-os.iso -drive file=disk.img,format=raw,index=0,media=disk
fi