#include <stdint.h>
#include "vgag.h"

extern void print(const char* str);
extern void println(const char* str);
extern void printint(int num);
extern int atoi(const char* str);
extern char get_key();
extern bool isdigit(char c);
extern void putchar(unsigned char c);
extern void sleep(uint32_t count);
extern void reboot();
extern int memcmp(const void* s1, const void* s2, int n);
extern bool strcmp(const char* a, const char* b);
extern void printlnc(const char* str, uint8_t color);
extern void printc(const char* str, uint8_t color);
extern void putcharc(unsigned char c, uint8_t color);
extern void printmult(unsigned char c, int l);
extern void clear_screen();
extern void printmultc(unsigned char c, int l, uint8_t color);
extern void reboot();
extern bool vfs_read_file(const char* path, char* buffer_out);
extern void beep(uint32_t frequency, uint32_t count);

extern uint16_t* vga;
extern int cursorX;
extern int cursorY;

void vgag_blue() {
    for (int y = 0; y < 25; y++)
        for (int x = 0; x < 80; x++)
            vga[y * 80 + x] = (0x11 << 8) | 0xDB;
}

#define TL 0xC9  // ╔
#define TR 0xBB  // ╗
#define BL 0xC8  // ╚
#define BR 0xBC  // ╝
#define H  0xCD  // ═
#define V  0xBA  // ║

void vgag_box() {
    cursorX = 10; cursorY = 5;
    putcharc(0xC9, 0x70); printmultc(0xCD, 58, 0x70); putcharc(0xBB, 0x70); // Top

    cursorX = 10; cursorY = 6;
    while (cursorY < 19) {
        putcharc(0xBA, 0x70);
        cursorY++;
        cursorX = 10;
    } // Left side

    cursorX = 69; cursorY = 6;
    while (cursorY < 19) {
        putcharc(0xBA, 0x70);
        cursorY++;
        cursorX = 69;
    } // right side

    cursorX = 10; cursorY = 19;
    putcharc(0xC8, 0x70); printmultc(0xCD, 58, 0x70); putcharc(0xBC, 0x70); // Bottom

    cursorX = 11; cursorY = 6;
    while (cursorY < 19) {
        printmultc(0xDB, 58, 0x7);
        cursorX = 11;
        cursorY++;
    } // Fill
    
    cursorX = 0; cursorY = 0;
}

char usin_buffer2[20];
int usin_index2 = 0;

char psin_buffer2[20];
int psin_index2 = 0;

char username_buffer2[20];
char password_buffer2[20];

void vgag_login() {
    cursorX = 24; cursorY = 10;
    if (vfs_read_file("0:\\data\\password.ini", password_buffer2) && vfs_read_file("0:\\data\\username.ini", username_buffer2)) {
        printc("Username: ", 0x70);
        printmultc(0xDB, 20, 0x7F);
        cursorX = 24; cursorY = 12;
        printc("Password: ", 0x70);
        printmultc(0xDB, 20, 0x7F);
        cursorX = 22; cursorY = 10;
        printc(">", 0x70);
        cursorX = 34; cursorY = 10;
        bool userrunning = true;
        while (userrunning) {
            char key = get_key();

            if (!key) {
                continue;
            }
            else if (key == '\n') {
                if (memcmp(username_buffer2, usin_buffer2, 20) == 0)
                {
                    userrunning = false;
                }
                else 
                {
                    cursorX = 18; cursorY = 16;
                    printc("Username incorrect! The system will restart.", 0x74);
                    sleep(20000);
                    reboot();
                }
            }
            else if (key == 8) {
                if (usin_index2 > 0) {
                    usin_index2--;
                    usin_buffer2[usin_index2] = '\0';
                    cursorX--;
                    putcharc(' ', 0xF0);
                    cursorX--;
                }
            }
            else {
                if (usin_index2 < 20) {
                    putcharc(key, 0xF0);
                    usin_buffer2[usin_index2] = key;
                    usin_index2++;
                }
                else {
                    continue;
                }
            }
        }
        cursorX = 22; cursorY = 10;
        putcharc(0xDB, 0x7);
        cursorX = 22; cursorY = 12;
        printc(">", 0x70);
        cursorX = 34; cursorY = 12;
        bool passrunning = true;
        while (passrunning) {
            char key = get_key();

            if (!key) {
                continue;
            }
            else if (key == '\n') {
                if (strcmp(password_buffer2, psin_buffer2)) {
                    passrunning = false;
                }
                else {
                    cursorX = 18; cursorY = 16;
                    printc("Password incorrect! The system will restart.", 0x74);
                    sleep(20000);
                    reboot();
                }
            }
            else if (key == 8) {
                if (psin_index2 > 0) {
                    psin_index2--;
                    psin_buffer2[psin_index2] = '\0';
                    cursorX--;
                    putcharc(' ', 0xF0);
                    cursorX--;
                }
            }
            else if (key) {
                if (psin_index2 < 20) {
                    putcharc('*', 0xF0);
                    psin_buffer2[psin_index2] = key;
                    psin_index2++;
                }
                else {
                    continue;
                }
            }
            else {
                continue;
            }
        }
        vgag_blue();
    } else {
        printc("zamn", 0x70);
    }
}

void vgag_printlnm(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '*') {
            putcharc('*', 0x17);
        }
        else {
            putcharc(str[i], 0x1F); 
        }
    }
    cursorX = 16;
    cursorY++;
}

void vgag_mol() {
    cursorX = 16; cursorY = 1;
    vgag_printlnm("                  *                       ");
    vgag_printlnm("                   *                      ");
    vgag_printlnm("                   *+=----        **      ");
    vgag_printlnm("                   =-------      *        ");
    vgag_printlnm("    **       *****+--------+*****         ");
    vgag_printlnm("     *      **    --------=+     *        ");
    vgag_printlnm("      *++++**      ===--=++       **      ");
    vgag_printlnm("     +=-----=      =-::::-=               ");
    vgag_printlnm("    *=-------+*++++-::::::-+*****       **");
    vgag_printlnm("    +--------======-::::::-=    **      * ");
    vgag_printlnm("      -----==========-::--=      **   **  ");
    vgag_printlnm("      **   ++========----==      ++====++ ");
    vgag_printlnm("    **      ++======------==    +=======+*");
    vgag_printlnm("    *        **++++-------==-::-=========*");
    vgag_printlnm("                   ==------:::::-======== ");
    vgag_printlnm("                    *++++=-::::::-++++**  ");
    vgag_printlnm("                          --::::-=        ");
    vgag_printlnm("                           +====*         ");
    vgag_printlnm("                         **      *        ");
    vgag_printlnm("                        ***      ***      ");
    vgag_printlnm("                          *               ");
    vgag_printlnm("                           **             ");
    vgag_printlnm("                                          ");
    vgag_printlnm("                  MMS - OS                ");
    cursorX = 80; cursorY = 25;
    sleep(5000);
    beep(660, 500);
    beep(590, 500);
    sleep(5000);
    vgag_blue();
}

char type_buffer[128];
int type_index = 0;

void vgag_run() {
    clear_screen();
    vgag_blue();
    vgag_box();
    vgag_login();
    vgag_mol();
    cursorX = 0; cursorY = 0;
    printc("Press enter to continue", (0x1 << 4) | 0xF);
    cursorX = 11; cursorY = 6;
    bool running = true;
    while (running) {
        char key = get_key();

        if (!key) {
            continue;
        }
        else if (key == '\n') {
            running = false;
        }
        else if (key == 8) {
            if (type_index > 0) {
                type_index--;
                type_buffer[type_index] = '\0';
                cursorX--;
                putcharc(' ', 0x70);
                cursorX--;
            }
        }
        else {
            if (type_index <= 50) {
                putcharc(key, 0x70);
                type_buffer[type_index] = key;
                type_index++;
            }
            else {
                continue;
            }
        }
    }
    clear_screen();
}