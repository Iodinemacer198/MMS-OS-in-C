#include "fs.h"
#include "ata.h"

// Forward declarations from kernel for string utils and printing
extern int strlen(const char* str);
extern void print(const char* str);
extern void println(const char* str);
extern void printint(int num);

int strlen(const char* str) {
    int len = 0;
    while(str[len]) len++;
    return len;
}

void strcpy(char* dest, const char* src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

#define VFS_MAX_FILES 10
#define SECTOR_SIZE 512
#define FS_MAGIC 0x4D4D5346 // "MMSF" signature

typedef struct {
    char path[32];
    uint32_t lba;     // Sector where this file's data lives
    uint32_t size;    // Size in bytes
    bool exists;
} FileEntry;

// The structure that gets saved to Sector 1
typedef struct {
    uint32_t magic; 
    FileEntry files[VFS_MAX_FILES];
} DirectoryTable;

DirectoryTable current_dir;
uint8_t io_buffer[SECTOR_SIZE];

void vfs_init() {
    // Read sector 1 into our directory struct
    ata_read_sector(1, (uint8_t*)&current_dir);

    // If the magic number isn't there, the drive is unformatted. Format it.
    if (current_dir.magic != FS_MAGIC) {
        println("Formatting blank drive...");
        current_dir.magic = FS_MAGIC;
        for (int i = 0; i < VFS_MAX_FILES; i++) {
            current_dir.files[i].exists = false;
        }
        ata_write_sector(1, (uint8_t*)&current_dir);
        
        // Write a default file to prove it works
        vfs_write_file("0:\\system.ini", "OS=MMS-OS\nVERSION=1.1\nPERSISTENT=TRUE");
    } else {
        println("Disk mounted successfully.");
    }
}

bool vfs_write_file(const char* path, const char* data) {
    int file_len = strlen(data);
    if (file_len > SECTOR_SIZE) return false; // Currently limited to 1 sector per file

    int target_idx = -1;

    // Check if file exists to overwrite
    for (int i = 0; i < VFS_MAX_FILES; i++) {
        if (current_dir.files[i].exists && strcmp(current_dir.files[i].path, path)) {
            target_idx = i;
            break;
        }
    }

    // Otherwise find an empty slot
    if (target_idx == -1) {
        for (int i = 0; i < VFS_MAX_FILES; i++) {
            if (!current_dir.files[i].exists) {
                target_idx = i;
                strcpy(current_dir.files[target_idx].path, path);
                // Assign a sector on the disk. LBA 1 is directory, so files start at LBA 2.
                current_dir.files[target_idx].lba = 2 + i; 
                current_dir.files[target_idx].exists = true;
                break;
            }
        }
    }

    if (target_idx == -1) return false; // Disk full

    current_dir.files[target_idx].size = file_len;

    // Clear IO buffer, copy string, and write to the file's assigned LBA
    for(int i = 0; i < SECTOR_SIZE; i++) io_buffer[i] = 0;
    strcpy((char*)io_buffer, data);
    ata_write_sector(current_dir.files[target_idx].lba, io_buffer);

    // Save the updated directory back to LBA 1
    ata_write_sector(1, (uint8_t*)&current_dir);
    return true;
}

bool vfs_read_file(const char* path, char* buffer_out) {
    for (int i = 0; i < VFS_MAX_FILES; i++) {
        if (current_dir.files[i].exists && strcmp(current_dir.files[i].path, path)) {
            // Found it. Read its LBA from disk.
            ata_read_sector(current_dir.files[i].lba, (uint8_t*)buffer_out);
            // Null-terminate it based on saved size just to be safe
            buffer_out[current_dir.files[i].size] = '\0';
            return true;
        }
    }
    return false;
}

void vfs_list_files() {
    println("--- Persistent Disk 0:\\ ---");
    bool empty = true;
    for(int i = 0; i < VFS_MAX_FILES; i++) {
        if(current_dir.files[i].exists) {
            print(current_dir.files[i].path);
            print(" (");
            printint(current_dir.files[i].size);
            println(" bytes)");
            empty = false;
        }
    }
    if(empty) println("No files found.");
    println("----------------------------");
}