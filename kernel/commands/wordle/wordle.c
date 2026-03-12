#include <stdint.h>
#include "wordle.h"

extern void print(const char* str);
extern void println(const char* str);
extern void printint(int num);
extern int atoi(const char* str);
extern char get_key();
extern bool isdigit(char c);
extern void putchar(char c);

bool contains(const char* buffer, int length, char target) {
    for (int i = 0; i < length; i++) {
        if (buffer[i] == target) {
            return true;
        }
    }

    return false;
}

extern int cursorX;
extern int cursorY;

#define W_BUFFER 128
char w_buffer[W_BUFFER];
int w_index = 0;

bool isletter(char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        return true; 
    } else {
        return false; 
    }
}

int strlength(char* str) {
    int len = 0;
    while (str[len] != '\0')
        len++;
    return len;
}

const char* word = "ocean";
int wc_index = 0;
int win_check = 0;

void run_wordle() {
    println("=== Wordle ===");
    bool running = true;
    bool runningcheck = true;
    while (running) {
        char key = get_key();

        if (!key) {
            continue;
        }
        else if (isletter(key)) {
            putchar(key);
            w_buffer[w_index] = key;
            w_index++;
        }
        else if (key == 8) {
            if (w_index > 0) {
                w_index--;
                w_buffer[w_index] = '\0';
                cursorX--;
                putchar(' ');
                cursorX--;
            }
        }
        else if (key == '\n') {
            putchar('\n');
            if (strlength(w_buffer) == 5) {
                while (runningcheck)
                {
                    if (wc_index == 5) runningcheck = false;
                    else if (word[wc_index] == w_buffer[wc_index]) {
                        print("[G]");
                        wc_index++;
                        win_check++;
                    }
                    else if (contains(word, 5, w_buffer[wc_index])) {
                        print("[Y]");
                        wc_index++;
                    }
                    else {
                        print("[-]");
                        wc_index++;
                    }
                }
                running = false;
            }
            else {
                println("You need to a guess a 5 letter word!");
                for (int i = 0; i < W_BUFFER; i++) {
                    w_buffer[i] = 0;
                }
                wc_index = 0;
                w_index = 0;
            }
        }
        else {
            continue;
        }
    }
    
    putchar('\n');
    if (win_check == 5) {
        println("Correct!");
        wc_index = 0;
        w_index = 0;
        win_check = 0;
        running = false;
        runningcheck = false;
        for (int i = 0; i < W_BUFFER; i++) {
            w_buffer[i] = 0;
        }
        return;
    }
    wc_index = 0;
    w_index = 0;
    win_check = 0;
    running = true;
    runningcheck = true;
    for (int i = 0; i < W_BUFFER; i++) {
        w_buffer[i] = 0;
    }
    while (running) {
        char key = get_key();

        if (!key) {
            continue;
        }
        else if (isletter(key)) {
            putchar(key);
            w_buffer[w_index] = key;
            w_index++;
        }
        else if (key == 8) {
            if (w_index > 0) {
                w_index--;
                w_buffer[w_index] = '\0';
                cursorX--;
                putchar(' ');
                cursorX--;
            }
        }
        else if (key == '\n') {
            putchar('\n');
            if (strlength(w_buffer) == 5) {
                while (runningcheck)
                {
                    if (wc_index == 5) runningcheck = false;
                    else if (word[wc_index] == w_buffer[wc_index]) {
                        print("[G]");
                        wc_index++;
                        win_check++;
                    }
                    else if (contains(word, 5, w_buffer[wc_index])) {
                        print("[Y]");
                        wc_index++;
                    }
                    else {
                        print("[-]");
                        wc_index++;
                    }
                }
                running = false;
            }
            else {
                println("You need to a guess a 5 letter word!");
                for (int i = 0; i < W_BUFFER; i++) {
                    w_buffer[i] = 0;
                }
                wc_index = 0;
                w_index = 0;
            }
        }
        else {
            continue;
        }
    }
    putchar('\n');
    if (win_check == 5) {
        println("Correct!");
        wc_index = 0;
        w_index = 0;
        win_check = 0;
        running = true;
        runningcheck = true;
        for (int i = 0; i < W_BUFFER; i++) {
            w_buffer[i] = 0;
        }
        return;
    }
    wc_index = 0;
    w_index = 0;
    win_check = 0;
    running = true;
    runningcheck = true;
    for (int i = 0; i < W_BUFFER; i++) {
        w_buffer[i] = 0;
    }
    while (running) {
        char key = get_key();

        if (!key) {
            continue;
        }
        else if (isletter(key)) {
            putchar(key);
            w_buffer[w_index] = key;
            w_index++;
        }
        else if (key == 8) {
            if (w_index > 0) {
                w_index--;
                w_buffer[w_index] = '\0';
                cursorX--;
                putchar(' ');
                cursorX--;
            }
        }
        else if (key == '\n') {
            putchar('\n');
            if (strlength(w_buffer) == 5)
            {
                while (runningcheck)
                {
                    if (wc_index == 5) runningcheck = false;
                    else if (word[wc_index] == w_buffer[wc_index]) {
                        print("[G]");
                        wc_index++;
                        win_check++;
                    }
                    else if (contains(word, 5, w_buffer[wc_index])) {
                        print("[Y]");
                        wc_index++;
                    }
                    else {
                        print("[-]");
                        wc_index++;
                    }
                }
                running = false;
            }
            else {
                println("You need to a guess a 5 letter word!");
                for (int i = 0; i < W_BUFFER; i++) {
                    w_buffer[i] = 0;
                }
                wc_index = 0;
                w_index = 0;
            }
        }
        else {
            continue;
        }
    }
    putchar('\n');
    if (win_check == 5) {
        println("Correct!");
        wc_index = 0;
        w_index = 0;
        win_check = 0;
        running = true;
        runningcheck = true;
        for (int i = 0; i < W_BUFFER; i++) {
            w_buffer[i] = 0;
        }
        return;
    }
    wc_index = 0;
    w_index = 0;
    win_check = 0;
    running = true;
    runningcheck = true;
    for (int i = 0; i < W_BUFFER; i++) {
        w_buffer[i] = 0;
    }
    while (running) {
        char key = get_key();

        if (!key) {
            continue;
        }
        else if (isletter(key)) {
            putchar(key);
            w_buffer[w_index] = key;
            w_index++;
        }
        else if (key == 8) {
            if (w_index > 0) {
                w_index--;
                w_buffer[w_index] = '\0';
                cursorX--;
                putchar(' ');
                cursorX--;
            }
        }
        else if (key == '\n') {
            putchar('\n');
            if (strlength(w_buffer) == 5) {
                while (runningcheck)
                {
                    if (wc_index == 5) runningcheck = false;
                    else if (word[wc_index] == w_buffer[wc_index]) {
                        print("[G]");
                        wc_index++;
                        win_check++;
                    }
                    else if (contains(word, 5, w_buffer[wc_index])) {
                        print("[Y]");
                        wc_index++;
                    }
                    else {
                        print("[-]");
                        wc_index++;
                    }
                }
                running = false;
            }
            else {
                println("You need to a guess a 5 letter word!");
                for (int i = 0; i < W_BUFFER; i++) {
                    w_buffer[i] = 0;
                }
                wc_index = 0;
                w_index = 0;
            }
        }
        else {
            continue;
        }
    }
    putchar('\n');
    if (win_check == 5) {
        println("Correct!");
        wc_index = 0;
        w_index = 0;
        win_check = 0;
        running = true;
        runningcheck = true;
        for (int i = 0; i < W_BUFFER; i++) {
            w_buffer[i] = 0;
        }
        return;
    }
    wc_index = 0;
    w_index = 0;
    win_check = 0;
    running = true;
    runningcheck = true;
    for (int i = 0; i < W_BUFFER; i++) {
        w_buffer[i] = 0;
    }
    while (running) {
        char key = get_key();

        if (!key) {
            continue;
        }
        else if (isletter(key)) {
            putchar(key);
            w_buffer[w_index] = key;
            w_index++;
        }
        else if (key == 8) {
            if (w_index > 0) {
                w_index--;
                w_buffer[w_index] = '\0';
                cursorX--;
                putchar(' ');
                cursorX--;
            }
        }
        else if (key == '\n') {
            putchar('\n');
            if (strlength(w_buffer) == 5) {
                while (runningcheck)
                {
                    if (wc_index == 5) runningcheck = false;
                    else if (word[wc_index] == w_buffer[wc_index]) {
                        print("[G]");
                        wc_index++;
                        win_check++;
                    }
                    else if (contains(word, 5, w_buffer[wc_index])) {
                        print("[Y]");
                        wc_index++;
                    }
                    else {
                        print("[-]");
                        wc_index++;
                    }
                }
                running = false;
            }
            else {
                println("You need to a guess a 5 letter word!");
                for (int i = 0; i < W_BUFFER; i++) {
                    w_buffer[i] = 0;
                }
                wc_index = 0;
                w_index = 0;
            }
        }
        else {
            continue;
        }
    }
    putchar('\n');
    if (win_check == 5) {
        println("Correct!");
        wc_index = 0;
        w_index = 0;
        win_check = 0;
        running = true;
        runningcheck = true;
        for (int i = 0; i < W_BUFFER; i++) {
            w_buffer[i] = 0;
        }
        return;
    }
    else {
        println("Out of guesses! Word was:");
        print(word);
    }
    wc_index = 0;
    w_index = 0;
    win_check = 0;
    running = true;
    runningcheck = true;
    for (int i = 0; i < W_BUFFER; i++) {
        w_buffer[i] = 0;
    }
}
