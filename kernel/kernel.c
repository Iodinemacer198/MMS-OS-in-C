#include <stdint.h>
#include <stdbool.h>
//#include "fs.h"
#include "calc.h"
#include "login.h"
#include "wordle.h"
#include "ff.h"        
#include "diskio.h" 
#include "io.h"    

FATFS fs;               
FIL fil;                
FRESULT fr;

FIL fil2;
FRESULT f2r;

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

uint16_t* vga = (uint16_t*)0xB8000;

int cursorX = 0;
int cursorY = 0;

uint8_t color = 0x0F;

int memcmp(const void* s1, const void* s2, int n) {
    const unsigned char* p1 = s1;
    const unsigned char* p2 = s2;
    for (int i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    return 0;
}

void sleep(uint32_t count) {
    for (volatile uint32_t i = 0; i < count * 10000; i++) {
        __asm__ volatile("nop");
    }
}

void memset(void* dest, uint8_t val, uint32_t len) {
    uint8_t* ptr = (uint8_t*)dest;
    while (len-- > 0) *ptr++ = val;
}

// Text

void scroll() {
    for (int y = 0; y < VGA_HEIGHT - 1; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            vga[y * VGA_WIDTH + x] = vga[(y + 1) * VGA_WIDTH + x];
        }
    }

    for (int x = 0; x < VGA_WIDTH; x++)
    {
        vga[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (color << 8) | ' ';
    }
}

void putchar(char c) {
    if (c == '\n')
    {
        cursorX = 0;
        cursorY++;
    }
    else
    {
        vga[cursorY * VGA_WIDTH + cursorX] = (color << 8) | c;
        cursorX++;

        if (cursorX >= VGA_WIDTH)
        {
            cursorX = 0;
            cursorY++;
        }
    }

    if (cursorY >= VGA_HEIGHT)
    {
        scroll();
        cursorY = VGA_HEIGHT - 1;
    }
}

void print(const char* str) {
    for (int i = 0; str[i] != 0; i++)
        putchar(str[i]);
}

void println(const char* str) {
    print(str);
    putchar('\n');
}

void clear_screen() {
    for (int y = 0; y < VGA_HEIGHT; y++)
        for (int x = 0; x < VGA_WIDTH; x++)
            vga[y * VGA_WIDTH + x] = (color << 8) | ' ';

    cursorX = 0;
    cursorY = 0;
}

char* strchr(const char* s, int c) {
    while (*s != (char)c) {
        if (!*s++) {
            return 0;
        }
    }
    return (char*)s;
}

// IO

void reboot() {
    while (inb(0x64) & 0x02);
    outb(0x64, 0xFE);

    while (1);
}

void shutdown() {
    outw(0x604, 0x2000);
}

bool isdigit(char c) {
    return (c >= '0' && c <= '9');
}

int atoi(const char* str) {
    int result = 0;
    int i = 0;

    while (str[i] >= '0' && str[i] <= '9')
    {
        result = result * 10 + (str[i] - '0');
        i++;
    }

    return result;
}

void printint(int num) {
    char buf[16];
    int i = 0;

    if (num == 0)
    {
        putchar('0');
        return;
    }

    while (num > 0)
    {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }

    while (i--)
        putchar(buf[i]);
}

// Keyboard

char keyboard_map[128] = {
0,27,'1','2','3','4','5','6','7','8','9','0','-','=',8,9,
'q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s',
'd','f','g','h','j','k','l',';',39,'`',0,'\\','z','x','c','v',
'b','n','m',',','.','/',0,'*',0,' ',0
};

char get_key() {
    if (!(inb(0x64) & 1))
        return 0;

    uint8_t scancode = inb(0x60);

    if (scancode & 0x80)
        return 0;

    if (scancode > 57)
        return 0;

    return keyboard_map[scancode];
}

#define CMD_BUFFER 128
char cmd_buffer[CMD_BUFFER];
int cmd_index = 0;

bool strcmp(const char* a, const char* b) {
    int i = 0;

    while (a[i] && b[i])
    {
        if (a[i] != b[i])
            return false;
        i++;
    }

    return a[i] == b[i];
}

// Time & Date

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

uint8_t get_update_in_progress_flag() {
    outb(CMOS_ADDRESS, 0x0A);
    return (inb(CMOS_DATA) & 0x80);
}

uint8_t get_rtc_register(int reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

void print_time() {
    while (get_update_in_progress_flag());

    uint8_t second = get_rtc_register(0x00);
    uint8_t minute = get_rtc_register(0x02);
    uint8_t hour   = get_rtc_register(0x04);
    uint8_t day    = get_rtc_register(0x07);
    uint8_t month  = get_rtc_register(0x08);
    uint8_t year   = get_rtc_register(0x09);

    second = (second & 0x0F) + ((second / 16) * 10);
    minute = (minute & 0x0F) + ((minute / 16) * 10);
    hour   = ((hour & 0x0F) + (((hour & 0x70) / 16) * 10)) | (hour & 0x80);
    day    = (day & 0x0F) + ((day / 16) * 10);
    month  = (month & 0x0F) + ((month / 16) * 10);
    year   = (year & 0x0F) + ((year / 16) * 10);

    print("Current Time: ");
    printint(hour);   putchar(':');
    printint(minute); putchar(':');
    printint(second); print(" UTC");
    print("  Date: ");
    printint(day);    putchar('/');
    printint(month);  putchar('/');
    print("20"); printint(year); 
    println("");
}

// Command logic

void run_command() {
    putchar('\n');

    if (strcmp(cmd_buffer, "help")) {
        println("Commands:");
        println("help :: This page!");
        println("about :: OS information");
        println("calc :: Simple calculator");
        println("clear :: Clears the screen");
        println("time :: Display current time and date");
        println("wordle :: Plays a game of Wordle");
        println(" ");
        println("test read :: Reads test file");
        println("test view :: Simple FEX");
        println(" ");
        println("reboot :: Reboots system");
        println("shutdown :: Shuts down system");
    }
    else if (strcmp(cmd_buffer, "clear")) {
        clear_screen();
    }
    else if (strcmp(cmd_buffer, "about")) {
        println("Molecular Multiverse Services OS: developed by the the MMS team with C.");
        println("If you need support, contact therealiodinemacer or join ZAx3NN5TJY on Discord.");
    }
    else if (strcmp(cmd_buffer, "test read")) {
        char read_buffer[512];
        UINT br; /* Bytes read */

        // FatFs uses "0:test.ini" format
        fr = f_open(&fil, "0:/System/test.ini", FA_READ);
        
        if (fr == FR_OK) {
            // Read up to 511 bytes (leave 1 for null terminator)
            f_read(&fil, read_buffer, 511, &br);
            read_buffer[br] = '\0'; // Null terminate the string
            
            println("--- File Content ---");
            println(read_buffer);
            println("--------------------");
            
            f_close(&fil);
        } else {
            print("Error: Could not open test.ini. Code: ");
            printint((int)fr);
            println("");
        }
    }
    else if (strcmp(cmd_buffer, "test view")) {
        DIR dir;
        FILINFO fno;

        fr = f_findfirst(&dir, &fno, "0:", "*"); // Start searching in root
        
        if (fr == FR_OK && fno.fname[0]) {
            while (fr == FR_OK && fno.fname[0]) {
                // Check if it's a directory or a file
                if (fno.fattrib & AM_DIR) print("<DIR> ");
                else print("      ");
                
                print(fno.fname);
                print("  -  ");
                printint(fno.fsize);
                println(" bytes");
                
                fr = f_findnext(&dir, &fno); // Find next entry
            }
            f_closedir(&dir);
        } else {
            println("Directory is empty or disk error.");
        }
    }
    else if (strcmp(cmd_buffer, "time")) {
        print_time();
    }
    else if (strcmp(cmd_buffer, "calc")) {
        // clear_screen();
        run_calc();
    }
    else if (strcmp(cmd_buffer, "wordle")) {
        run_wordle();
    }
    else if (strcmp(cmd_buffer, "shutdown")) shutdown();
    else if (strcmp(cmd_buffer, "reboot")) reboot();
    else {
        println("Unknown command");
    }

    println("");
    print("shell > ");

    cmd_index = 0;

    for(int i = 0; i < CMD_BUFFER; i++)
        cmd_buffer[i] = 0;
}

// Main loop

void kernel_main() {
    clear_screen();

    FRESULT res;

    println("Checkpoint 1: Mounting...");

    fr = f_mount(&fs, "0:", 0); 
    if (fr != FR_OK) {
        print("FS Error: Could not mount partition. Code: ");
        printint((int)fr);
        println("");
    } else {
        println("File System: Mounted successfully.");
    }

    println("Checkpoint 2: Mkdir...");

    res = f_mkdir("0:/System");
    if (res != FR_OK && res != FR_EXIST) {
        print("Error creating system directory. Code: ");
        printint((int)res);
        println("");
    }

    res = f_mkdir("0:/Data");
    if (res != FR_OK && res != FR_EXIST) {
        print("Error creating Data directory. Code: ");
        printint((int)res);
        println("");
    }

    FILINFO fno;
    FRESULT res2;

    res2 = f_stat("0:/System/test.ini", &fno);

    println("Checkpoint 3: Stat...");

    if (res2 == FR_NO_FILE) {
        println("test.ini not found. Creating default...");
        
        res2 = f_open(&fil, "0:/System/test.ini", FA_WRITE | FA_CREATE_ALWAYS);
        if (res2 == FR_OK) {
            UINT bw;
            f_write(&fil, "sup twih :wilted_rose:", 22, &bw);
            f_close(&fil);
            println("Done.");
        }
    } else if (res == FR_OK) {
        
    } else {
        println("Disk error occurred.");
    }

    println("                  *                       ");
    println("                   *                      ");
    println("                   *+=----        **      ");
    println("                   =-------      *        ");
    println("    **       *****+--------+*****         ");
    println("     *      **    --------=+     *        ");
    println("      *++++**      ===--=++       **      ");
    println("     +=-----=      =-::::-=               ");
    println("    *=-------+*++++-::::::-+*****       **");
    println("    +--------======-::::::-=    **      * ");
    println("      -----==========-::--=      **   **  ");
    println("      **   ++========----==      ++====++ ");
    println("    **      ++======------==    +=======+*");
    println("    *        **++++-------==-::-=========*");
    println("                   ==------:::::-======== ");
    println("                    *++++=-::::::-++++**  ");
    println("                          --::::-=        ");
    println("                           +====*         ");
    println("                         **      *        ");
    println("                        ***      ***      ");
    println("                          *               ");
    println("                           **             ");

    handle_login();

    println("Welcome to MMS-OS!");
    print_time();
    println("Type 'help' for commands");
    print("shell > ");

    while (1) {
        char key = get_key();

        if (!key) {
            continue;
        }
        if (key == '\n') {
            run_command();
        }
        else if (key == 8) {
            if (cmd_index > 0) {
                cmd_index--;
                cmd_buffer[cmd_index] = '\0';
                cursorX--;
                putchar(' ');
                cursorX--;
            }
        }
        else {
            putchar(key);
            cmd_buffer[cmd_index] = key;
            cmd_index++;
        }
    }
}