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
extern bool vfs_write_file(const char* path, const char* data);
extern void printintc(int num, uint8_t color);
extern uint8_t get_rtc_register(int reg);
extern uint8_t get_update_in_progress_flag();
extern void scroll();

extern uint16_t* vga;
extern int cursorX;
extern int cursorY;

int dcX;
int dcY;

void dputcharc(unsigned char c, uint8_t color) {
    if (c == '\n')
    {
        dcX = 0;
        dcY++;
    }
    else
    {
        vga[dcY * 80 + dcX] = (color << 8) | c;
        dcX++;

        if (dcX >= 80)
        {
            dcX = 0;
            dcY++;
        }
    }

    if (dcY >= 25)
    {
        scroll();
        dcY = 25 - 1;
    }
}

void dprintmultc(unsigned char c, int l, uint8_t color) {
    int i = 0;
    while (i < l) {
        dputcharc(c, color);
        i++;
    }
}

void dprintc(const char* str, uint8_t color) {
    for (int i = 0; str[i] != 0; i++)
        dputcharc(str[i], color);
}

void dprintintc(int num, uint8_t color) {
    char buf[16];
    int i = 0;

    if (num == 0)
    {
        dputcharc('0', color);
        return;
    }

    while (num > 0)
    {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }

    while (i--)
        dputcharc(buf[i], color);
}

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
    dcX = 10; dcY = 5;
    dputcharc(0xC9, 0x70); dprintmultc(0xCD, 58, 0x70); dputcharc(0xBB, 0x70); // Top

    dcX = 10; dcY = 6;
    while (dcY < 19) {
        dputcharc(0xBA, 0x70);
        dcY++;
        dcX = 10;
    } // Left side

    dcX = 69; dcY = 6;
    while (dcY < 19) {
        dputcharc(0xBA, 0x70);
        dcY++;
        dcX = 69;
    } // right side

    dcX = 10; dcY = 19;
    dputcharc(0xC8, 0x70); dprintmultc(0xCD, 58, 0x70); dputcharc(0xBC, 0x70); // Bottom

    dcX = 11; dcY = 6;
    while (dcY < 19) {
        dprintmultc(0xDB, 58, 0x7);
        dcX = 11;
        dcY++;
    } // Fill

    dcX = 11; dcY = 20;
    dprintmultc(0xDF, 60, 0x10); 
    dcX = 70; dcY = 6;
    while (dcY < 20) {
        dputcharc(0xDB, 0x0);
        dcY++;
        dcX = 70;
    } // Shadow
    
    dcX = 0; dcY = 0;
}

char usin_buffer2[20];
int usin_index2 = 0;

char psin_buffer2[20];
int psin_index2 = 0;

char username_buffer2[20];
char password_buffer2[20];

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

void vgag_login() {
    dcX = 24; dcY = 10;
    if (vfs_read_file("0:\\data\\password.ini", password_buffer2) && vfs_read_file("0:\\data\\username.ini", username_buffer2)) {
        dprintc("Username: ", 0x70);
        dprintmultc(0xDB, 20, 0x7F);
        dcX = 24; dcY = 12;
        dprintc("Password: ", 0x70);
        dprintmultc(0xDB, 20, 0x7F);
        dcX = 22; dcY = 10;
        dprintc(">", 0x70);
        dcX = 34; dcY = 10;
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
                    dcX = 18; dcY = 16;
                    dprintc("Username incorrect! The system will restart.", 0x74);
                    sleep(20000);
                    reboot();
                }
            }
            else if (key == 8) {
                if (usin_index2 > 0) {
                    usin_index2--;
                    usin_buffer2[usin_index2] = '\0';
                    dcX--;
                    dputcharc(' ', 0xF0);
                    dcX--;
                }
            }
            else {
                if (usin_index2 < 20) {
                    dputcharc(key, 0xF0);
                    usin_buffer2[usin_index2] = key;
                    usin_index2++;
                }
                else {
                    continue;
                }
            }
        }
        dcX = 22; dcY = 10;
        dputcharc(0xDB, 0x7);
        dcX = 22; dcY = 12;
        dprintc(">", 0x70);
        dcX = 34; dcY = 12;
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
                    dcX = 18; dcY = 16;
                    dprintc("Password incorrect! The system will restart.", 0x74);
                    sleep(20000);
                    reboot();
                }
            }
            else if (key == 8) {
                if (psin_index2 > 0) {
                    psin_index2--;
                    psin_buffer2[psin_index2] = '\0';
                    dcX--;
                    dputcharc(' ', 0xF0);
                    dcX--;
                }
            }
            else if (key) {
                if (psin_index2 < 20) {
                    dputcharc('*', 0xF0);
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
        dcX = 22; dcY = 10;
        dprintc("New Username: ", 0x70);
        dprintmultc(0xDB, 20, 0x7F);
        dcX = 22; dcY = 12;
        dprintc("New Password: ", 0x70);
        dprintmultc(0xDB, 20, 0x7F);
        dcX = 20; dcY = 10;
        dprintc(">", 0x70);
        dcX = 36; dcY = 10;
        bool userrunning = true;
        while (userrunning) {
            char key = get_key();

            if (!key) {
                continue;
            }
            else if (key == '\n') {
                vfs_write_file("0:\\data\\username.ini", usin_buffer2);
                userrunning = false;
            }
            else if (key == 8) {
                if (usin_index2 > 0) {
                    usin_index2--;
                    usin_buffer2[usin_index2] = '\0';
                    dcX--;
                    dputcharc(' ', 0xF0);
                    dcX--;
                }
            }
            else {
                if (usin_index2 < 20) {
                    dputcharc(key, 0xF0);
                    usin_buffer2[usin_index2] = key;
                    usin_index2++;
                }
                else {
                    continue;
                }
            }
        }
        dcX = 20; dcY = 10;
        dputcharc(0xDB, 0x7);
        dcX = 20; dcY = 12;
        dprintc(">", 0x70);
        dcX = 36; dcY = 12;
        bool passrunning = true;
        while (passrunning) {
            char key = get_key();

            if (!key) {
                continue;
            }
            else if (key == '\n') {
                vfs_write_file("0:\\data\\password.ini", psin_buffer2);
                //println(psin_buffer2);
                passrunning = false;
            }
            else if (key == 8) {
                if (psin_index2 > 0) {
                    psin_index2--;
                    psin_buffer2[psin_index2] = '\0';
                    dcX--;
                    dputcharc(' ', 0xF0);
                    dcX--;
                }
            }
            else if (key) {
                if (psin_index2 < 20) {
                    dputcharc(key, 0xF0);
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
        dcX = 28; dcY = 16;
        dprintc("Successfully registered!", 0x72);
        sleep(20000);
        vgag_blue();
    }
}

void vgag_printlnm(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '*') {
            dputcharc('*', 0x17);
        }
        else {
            dputcharc(str[i], 0x1F); 
        }
    }
    dcX = 16;
    dcY++;
}

void vgag_printlni(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '*') {
            dputcharc('*', 0x79);
        }
        else if (str[i] == '%') {
            dputcharc('%', 0x78);
        }
        else if (str[i] == '#') {
            dputcharc('#', 0x7C);
        }
        if (str[i] == '@') {
            dputcharc('@', 0x70);
        }
    }
    dcX = 48;
    dcY++;
}

void vgag_mol() {
    dcX = 16; dcY = 1;
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
    dcX = 80; dcY = 25;
    sleep(5000);
    beep(660, 500);
    beep(590, 500);
    sleep(5000);
    vgag_blue();
}

void vgag_iodine() {
    dcX = 48; dcY = 6;
    vgag_printlni("********************");
    vgag_printlni("*********#@*********");
    vgag_printlni("******%%*#@*%%******");
    vgag_printlni("****%****#@****%****");
    vgag_printlni("***%***%@##@****%***");
    vgag_printlni("***%**@@@##@@***%***");
    vgag_printlni("***%*@@@@##@@@@*%***");
    vgag_printlni("****@**@@##@@**@****");
    vgag_printlni("***##############***");
    vgag_printlni("*****@@@@##@@@@*****");
    vgag_printlni("*********##*********");
}

char type_buffer[128];
int type_index = 0;

void vgag_hello() {
    dcX = 11; dcY = 6; 
    dprintc("Welcome to the MMS-OS VGAG mode!", 0x70);
    dcX = 11; dcY = 7;
    dprintc("Use function keys to navigate menus", 0x70);
    dcX = 11; dcY = 9;
    dprintc("This graphics mode is extremely WIP", 0x70);
    dcX = 11; dcY = 10;
    dprintc("and may contain some issues", 0x70);
    dcX = 11; dcY = 12;
    dprintc("Developed by iodinemacer with C",0x70);
}

void ind_login() {
    clear_screen();
    vgag_blue();
    vgag_box();
    vgag_login();
    vgag_mol();
    dcX = 0; dcY = 0;
}

void vgag_time() {
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

    dcX = 70; dcY = 0;

    dprintintc(hour, 0x70); dputcharc(':', 0x70);
    if (minute < 10) dprintintc(0, 0x70);
    dprintintc(minute, 0x70); dputcharc(':', 0x70);
    if (second < 10) dprintintc(0, 0x70);
    dprintintc(second, 0x70);
}

void vgag_taskbar() {
    dcX = 0; dcY = 0;
    dprintmultc(0xDB, 80, 0x7);
    dcX = 1; dcY = 0;
    dputcharc(0xF0, 0x70);
    dprintmultc(0xDB, 2, 0x7);
    dprintc("F1", 0x70);
    dprintmultc(0xDB, 2, 0x7);
    dprintc("F2", 0x70);
    dprintmultc(0xDB, 2, 0x7);
    dprintc("F3", 0x70);
    dcX = 0; dcY = 1;
    dprintmultc(0xDF, 80, 0x10); 
    vgag_time();
}

void vgag_run() {
    clear_screen();
    vgag_blue();
    vgag_taskbar();
    vgag_box();
    vgag_hello();
    vgag_iodine();
    dcX = 11; dcY = 18;
    dprintc("Press ESC to return to shell", 0x70);
    dcX = 11; dcY = 6;
    cursorX = 11; cursorY = 6;
    bool running = true;
    while (running) {
        vgag_time();
        unsigned char key = get_key();

        if (!key) {
            continue;
        }
        else if (key == 27) {
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
        else if (key == 128) {
            dcX = 3; dcY = 0;
            dputcharc(0xDB, 0x8);
            dprintc("F1", 0x8F);
            dprintmultc(0xDB, 1, 0x8);
            dprintmultc(0xDB, 1, 0x7);
            dprintc("F2", 0x70);
            dprintmultc(0xDB, 2, 0x7);
            dprintc("F3", 0x70);
            dprintmultc(0xDB, 1, 0x7);
        }
        else if (key == 129) {
            dcX = 3; dcY = 0;
            dprintmultc(0xDB, 1, 0x7);
            dprintc("F1", 0x70);
            dprintmultc(0xDB, 1, 0x7);
            dputcharc(0xDB, 0x8);
            dprintc("F2", 0x8F);
            dprintmultc(0xDB, 1, 0x8);
            dprintmultc(0xDB, 1, 0x7);
            dprintc("F3", 0x70);
            dprintmultc(0xDB, 1, 0x7);
        }
        else if (key == 130) {
            dcX = 3; dcY = 0;
            dprintmultc(0xDB, 1, 0x7);
            dprintc("F1", 0x70);
            dprintmultc(0xDB, 2, 0x7);
            dprintc("F2", 0x70);
            dprintmultc(0xDB, 1, 0x7);
            dputcharc(0xDB, 0x8);
            dprintc("F3", 0x8F);
            dprintmultc(0xDB, 1, 0x8);
            dprintmultc(0xDB, 1, 0x7);
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