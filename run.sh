#!/bin/bash
set -e

qemu-system-x86_64 -cdrom iso/output/mms-os.iso -drive format=raw,file=disk.img