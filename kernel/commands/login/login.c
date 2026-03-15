#include <stdint.h>
#include <stdbool.h>
#include "login.h"
#include "../../fatfs/ff.h"

extern void print(const char* str);
extern void println(const char* str);
extern void printint(int num);
extern int atoi(const char* str);
extern char get_key();
extern bool isdigit(char c);
extern void putchar(char c);
extern void sleep(uint32_t count);
extern void reboot();
extern int memcmp(const void* s1, const void* s2, int n);
extern bool strcmp(const char* a, const char* b);

extern int cursorX;
extern int cursorY;

extern FIL fil2;
extern FRESULT fr2;

#define USIN_BUFFER 20
char usin_buffer[USIN_BUFFER];
int usin_index = 0;

#define PSIN_BUFFER 20
char psin_buffer[PSIN_BUFFER];
int psin_index = 0;

char username_buffer[20];
char password_buffer[20];

bool fat_read_helper(const char* path, char* buffer, UINT len) {
    UINT br;
    if (f_open(&fil2, path, FA_READ) == FR_OK) {
        f_read(&fil2, buffer, len, &br);
        buffer[br] = '\0';
        f_close(&fil2);
        return true;
    }
    return false;
}

bool fat_write_helper(const char* path, const char* data, UINT len) {
    UINT bw;
    if (f_open(&fil2, path, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
        f_write(&fil2, data, len, &bw);
        f_close(&fil2);
        return true;
    }
    return false;
}

void handle_login() {
    for(int i=0; i<20; i++) { username_buffer[i] = 0; password_buffer[i] = 0; }
    if (fat_read_helper("0:/Data/password.ini", password_buffer, 19) && 
        fat_read_helper("0:/Data/username.ini", username_buffer, 19)) {
        
        print("Username > ");
        bool userrunning = true;
        while (userrunning) {
            char key = get_key();
            if (!key) continue;

            if (key == '\n') {
                if (strcmp(username_buffer, usin_buffer)) {
                    userrunning = false;
                }
                else {
                    putchar('\n');
                    println("Username incorrect! The system will restart.");
                    sleep(20000);
                    reboot();
                }
            }
            else if (key == 8) {
                if (usin_index > 0) {
                    usin_index--;
                    usin_buffer[usin_index] = '\0';
                    cursorX--; putchar(' '); cursorX--;
                }
            }
            else {
                if (usin_index < 19) {
                    putchar(key);
                    usin_buffer[usin_index] = key;
                    usin_index++;
                    usin_buffer[usin_index] = '\0';
                }
            }
        }

        putchar('\n');
        print("Password > ");
        bool passrunning = true;
        while (passrunning) {
            char key = get_key();
            if (!key) continue;

            if (key == '\n') {
                if (strcmp(password_buffer, psin_buffer)) {
                    putchar('\n');
                    passrunning = false;
                }
                else {
                    putchar('\n');
                    println("Password incorrect! The system will restart.");
                    sleep(20000);
                    reboot();
                }
            }
            else if (key == 8) {
                if (psin_index > 0) {
                    psin_index--;
                    psin_buffer[psin_index] = '\0';
                    cursorX--; putchar(' '); cursorX--;
                }
            }
            else {
                if (psin_index < 19) {
                    putchar('*'); 
                    psin_buffer[psin_index] = key;
                    psin_index++;
                    psin_buffer[psin_index] = '\0';
                }
            }
        }
    }
    else {
        print("New Username > ");
        bool userrunning = true;
        while (userrunning) {
            char key = get_key();
            if (!key) continue;

            if (key == '\n') {
                fat_write_helper("0:/Data/username.ini", usin_buffer, usin_index);
                userrunning = false;
            }
            else if (key == 8) {
                if (usin_index > 0) {
                    usin_index--;
                    usin_buffer[usin_index] = '\0';
                    cursorX--; putchar(' '); cursorX--;
                }
            }
            else {
                if (usin_index < 19) {
                    putchar(key);
                    usin_buffer[usin_index] = key;
                    usin_index++;
                    usin_buffer[usin_index] = '\0';
                }
            }
        }

        putchar('\n');
        print("New Password > ");
        bool passrunning = true;
        while (passrunning) {
            char key = get_key();
            if (!key) continue;

            if (key == '\n') {
                fat_write_helper("0:/Data/password.ini", psin_buffer, psin_index);
                passrunning = false;
            }
            else if (key == 8) {
                if (psin_index > 0) {
                    psin_index--;
                    psin_buffer[psin_index] = '\0';
                    cursorX--; putchar(' '); cursorX--;
                }
            }
            else {
                if (psin_index < 19) {
                    putchar(key);
                    psin_buffer[psin_index] = key;
                    psin_index++;
                    psin_buffer[psin_index] = '\0';
                }
            }
        }
        putchar('\n');
        println("Successfully registered!");
    }
}