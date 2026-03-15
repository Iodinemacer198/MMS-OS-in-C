/*-----------------------------------------------------------------------*/
/* Low level disk I/O module for Bare Metal C OS         (C) Gemini 2026 */
/*-----------------------------------------------------------------------*/

#include "ff.h"         /* Basic definitions of FatFs */
#include "diskio.h"     /* Declarations FatFs MAI */
#include "io.h"     

/* Definitions for ATA IO Ports */
#define ATA_DATA        0x1F0
#define ATA_FEATURES    0x1F1
#define ATA_SECTOR_CNT  0x1F2
#define ATA_LBA_LOW     0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HIGH    0x1F5
#define ATA_DRIVE_SEL   0x1F6
#define ATA_COMMAND     0x1F7
#define ATA_STATUS      0x1F7

/* Primary Drive Mapping */
#define DEV_ATA    0 

static DSTATUS Stat = STA_NOINIT;

/* Helper: Wait for ATA drive to be ready */
static int ata_wait_ready() {
    uint32_t timeout = 1000000; 
    while (timeout--) {
        uint8_t status = inb(ATA_STATUS);
        if ((status & 0x88) == 0x08) return 1; // Success
        if (status & 0x01) return 0; // Error bit set
    }
    return 0; // Timeout
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status (BYTE pdrv) {
    if (pdrv != DEV_ATA) return STA_NOINIT;
    return Stat;
}

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize (BYTE pdrv) {
    if (pdrv != DEV_ATA) return STA_NOINIT;

    // Simple ATA Initialization: Select drive 0
    outb(ATA_DRIVE_SEL, 0xE0); 
    
    // You could add an IDENTIFY command here to verify the drive exists
    
    Stat &= ~STA_NOINIT; // Clear the NOINIT flag
    return Stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT disk_read (BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
    if (pdrv != DEV_ATA || Stat & STA_NOINIT) return RES_NOTRDY;

    for (UINT i = 0; i < count; i++) {
        uint32_t lba = (uint32_t)sector + i;
        
        outb(ATA_DRIVE_SEL,  (0xE0 | ((lba >> 24) & 0x0F)));
        outb(ATA_FEATURES,   0x00);
        outb(ATA_SECTOR_CNT, 0x01);
        outb(ATA_LBA_LOW,    (uint8_t)lba);
        outb(ATA_LBA_MID,    (uint8_t)(lba >> 8));
        outb(ATA_LBA_HIGH,   (uint8_t)(lba >> 16));
        outb(ATA_COMMAND,    0x20); // Read sectors with retry

        ata_wait_ready();

        // Read 256 words (512 bytes)
        for (int j = 0; j < 256; j++) {
            uint16_t data = inw(ATA_DATA);
            buff[(i * 512) + (j * 2)]     = (uint8_t)data;
            buff[(i * 512) + (j * 2 + 1)] = (uint8_t)(data >> 8);
        }
    }

    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#if FF_FS_READONLY == 0
DRESULT disk_write (BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
    if (pdrv != DEV_ATA || Stat & STA_NOINIT) return RES_NOTRDY;

    for (UINT i = 0; i < count; i++) {
        uint32_t lba = (uint32_t)sector + i;

        outb(ATA_DRIVE_SEL,  (0xE0 | ((lba >> 24) & 0x0F)));
        outb(ATA_SECTOR_CNT, 0x01);
        outb(ATA_LBA_LOW,    (uint8_t)lba);
        outb(ATA_LBA_MID,    (uint8_t)(lba >> 8));
        outb(ATA_LBA_HIGH,   (uint8_t)(lba >> 16));
        outb(ATA_COMMAND,    0x30); // Write sectors with retry

        ata_wait_ready();

        // Write 256 words
        for (int j = 0; j < 256; j++) {
            uint16_t data = buff[(i * 512) + (j * 2)] | (buff[(i * 512) + (j * 2 + 1)] << 8);
            outw(ATA_DATA, data);
        }
    }

    return RES_OK;
}
#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void *buff) {
    if (pdrv != DEV_ATA) return RES_PARERR;

    switch (cmd) {
        case CTRL_SYNC:
            return RES_OK;
        case GET_SECTOR_COUNT:
            // For now, hardcode or use IDENTIFY to get real size
            *(LBA_t*)buff = 204800; // Example: 100MB drive
            return RES_OK;
        case GET_SECTOR_SIZE:
            *(WORD*)buff = 512;
            return RES_OK;
        case GET_BLOCK_SIZE:
            *(DWORD*)buff = 1;
            return RES_OK;
    }

    return RES_PARERR;
}

/* Required by FatFs for file timestamps */
DWORD get_fattime (void) {
    // Return a dummy time (Jan 1, 2026) or implement RTC driver
    return ((DWORD)(2026 - 1980) << 25) | ((DWORD)1 << 21) | ((DWORD)1 << 16);
}