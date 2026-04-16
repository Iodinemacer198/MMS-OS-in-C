# MMS-OS
Molecular Multiverse Services Operating System, or simply MMS-OS, is a WIP operating system made with C.

## Current Features & Overview
- Command line interface like setup
- Calculator
- Wordle game
- Basic file system with commands
- Basic Tiny CC integration (close enough to actual C syntax, uses source functions)
- Login system
- A WIP VGA "graphics" (VGAG) system

## Use/Build instructions
**Build from source:** (recommended)
- Requires a Linux system or Windows Subsystem for Linux
- Git clone project to your preferred directory
- Make sure you are in the project directory in your terminal AND have Qemu installed
- For ISO creation, install: `grub-pc-bin grub-efi-amd64-bin xorriso mtools syslinux-utils`
- Run `chmod +x build.sh` and `chmod +x run.sh`
- First run `./build.sh`, then if successful run `./run.sh`
- Congrats! You are now in MMS-OS!

**Use from release ISO:** (iso/output)
- Load ISO in your system of choice. It should work fine out the box.
- The ISO is built as a GRUB hybrid image and can be written directly to USB media with `dd`.

**Boot on real hardware**
- Build with `./build.sh`
- Write to a USB drive (replace `/dev/sdX` carefully):
  - `sudo dd if=iso/output/mms-os.iso of=/dev/sdX bs=4M status=progress oflag=sync`
- Boot target machine from the USB device in BIOS or UEFI mode.
- `build.sh` also runs `isohybrid` (if installed) to improve USB firmware compatibility.

**Windows (Rufus) USB creation**
- Yes, you can use Rufus to flash `iso/output/mms-os.iso`.
- In Rufus:
  - Select the ISO.
  - Partition scheme: `MBR` for widest BIOS/UEFI compatibility (or `GPT` for UEFI-only systems).
  - Target system should match your firmware mode (BIOS/UEFI).
  - If prompted for write mode, prefer `DD Image mode` for low-level OS images.

**If the PC still boots the old OS instead of MMS-OS**
- Use the one-time boot menu (often `F12`, `Esc`, or `F11`) and pick the USB device explicitly.
- Disable `Secure Boot` (this hobby OS is not signed for secure boot).
- Keep compatibility mode aligned with how USB was prepared:
  - Rufus `MBR` + firmware `Legacy/CSM` OR `UEFI with CSM`
  - Rufus `GPT` + firmware `UEFI`
- Re-flash the USB and use `DD mode` in Rufus if prompted.
- Try another USB port (prefer USB 2.0 on older machines) and verify boot order in firmware settings.

## Contact
This project is developed and maintained by the Molecular Multiverse Services team (just me so far, therealiodinemacer :D)
For any questions, suggestions, issues, etc, feel free to reach out to @therealiodinemacer on Discord or join our server [here](https://discord.gg/ZAx3NN5TJY))

## Roadmap
- ~~Better filesystem (preferably FAT)~~
- ~~Better command handling~~
- Possible graphics implementations

## Disclaimers
MMS-OS is a VERY work-in-progress project. Lag, bugs, etc are to be expected. As more of a hobby project, also expect slow progress and a lack of functionality. This project is just for fun, don't expect much :)

Some AI was used in the making of this project, especially with parts of the initial setup or other complicated pieces. I really don't know too much of C, and this is the easiest way for me to learn the language while also making progress. Outside of the initial setup and components that I needed help with, it is however my work. If you doubt that, check some of the scripts, there's no way anyone other than a stupid human could make some of those...
