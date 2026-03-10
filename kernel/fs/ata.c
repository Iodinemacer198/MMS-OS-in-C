#include "ata.h"

// Wait for the disk drive to be ready (Busy bit clears)
void ata_wait_bsy() {
    while (inb(0x1F7) & 0x80);
}

// Wait for the disk drive to request data (Data Request bit sets)
void ata_wait_drq() {
    while (!(inb(0x1F7) & 0x08));
}

// Read one 512-byte sector from the disk into a buffer
void ata_read_sector(uint32_t lba, uint8_t* buffer) {
    ata_wait_bsy();
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F)); // Select drive 0 (Master) and highest 4 bits of LBA
    outb(0x1F2, 1);                           // Read 1 sector
    outb(0x1F3, (uint8_t)lba);                // LBA low byte
    outb(0x1F4, (uint8_t)(lba >> 8));         // LBA mid byte
    outb(0x1F5, (uint8_t)(lba >> 16));        // LBA high byte
    outb(0x1F7, 0x20);                        // Send "Read with Retry" command

    ata_wait_bsy();
    ata_wait_drq();

    // Read 256 16-bit words (512 bytes total) from the data port
    uint16_t* ptr = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        ptr[i] = inw(0x1F0);
    }
}

// Write one 512-byte sector from a buffer to the disk
void ata_write_sector(uint32_t lba, uint8_t* buffer) {
    ata_wait_bsy();
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x30);                        // Send "Write with Retry" command

    ata_wait_bsy();
    ata_wait_drq();

    // Write 256 16-bit words
    uint16_t* ptr = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        outw(0x1F0, ptr[i]);
    }
    
    // Send cache flush command
    outb(0x1F7, 0xE7);
    ata_wait_bsy();
}