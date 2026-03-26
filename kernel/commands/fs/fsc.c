#include <stdint.h>
#include <stdbool.h>
#include "fsc.h"

extern void print(const char* str);
extern void println(const char* str);
extern bool vfs_write_file(const char* path, const char* data);
extern bool vfs_read_file(const char* path, char* buffer_out);
extern void path_prepend(char* path);
extern void putchar(char c);
extern char get_key();
extern bool vfs_delete_file(const char* path);
extern int vfs_file_count();
extern bool vfs_make_dir(const char* path);
extern bool vfs_remove_dir(const char* path);
extern bool vfs_change_dir(const char* path);
extern void vfs_get_cwd(char* out);
extern void vfs_list_current_dir();

extern int cursorX;

static bool contains_sep(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\\' || str[i] == '/') return true;
    }
    return false;
}

static void read_input(char* out, int max_len) {
    int idx = 0;
    bool running = true;

    while (running) {
        char key = get_key();
        if (!key) continue;

        if (key == '\n') {
            running = false;
        }
        else if (key == 8) {
            if (idx > 0) {
                idx--;
                out[idx] = '\0';
                cursorX--;
                putchar(' ');
                cursorX--;
            }
        }
        else if (idx < (max_len - 1)) {
            putchar(key);
            out[idx++] = key;
            out[idx] = '\0';
        }
    }
}

void ls() {
    vfs_list_current_dir();
}

void pwd() {
    char cwd[64];
    vfs_get_cwd(cwd);
    println(cwd);
}

void cd() {
    print("Directory: ");
    char path[64] = "";
    read_input(path, 64);

    if (vfs_change_dir(path)) {
        putchar('\n');
    } else {
        putchar('\n');
        println("Could not change directory.");
    }
}

void mkdir_cmd() {
    print("Directory name: ");
    char name[64] = "";
    read_input(name, 64);

    if (contains_sep(name)) {
        putchar('\n');
        println("Use a single directory name (no path separators).");
        return;
    }

    path_prepend(name);
    if (vfs_make_dir(name)) {
        putchar('\n');
        println("Directory created.");
    } else {
        putchar('\n');
        println("Could not create directory.");
    }
}

void rmdir_cmd() {
    print("Directory name: ");
    char name[64] = "";
    read_input(name, 64);

    if (contains_sep(name)) {
        putchar('\n');
        println("Use a single directory name (no path separators).");
        return;
    }

    path_prepend(name);
    if (vfs_remove_dir(name)) {
        putchar('\n');
        println("Directory removed.");
    } else {
        putchar('\n');
        println("Directory not empty, not found, or protected.");
    }
}

void mkf() {
    int fileCount = vfs_file_count();
    if (fileCount >= 56) {
        println("Directory table is full! Use 'rmf' to delete some files!");
        return;
    }

    print("File name: ");
    char path2[64] = "";
    read_input(path2, 64);

    if (contains_sep(path2)) {
        putchar('\n');
        println("Use a single file name (no path separators).");
        return;
    }

    path_prepend(path2);

    char temp[512] = "";
    if (vfs_read_file(path2, temp)) {
        putchar('\n');
        println("A file with this name already exists!");
        return;
    }

    if (!vfs_write_file(path2, "")) {
        putchar('\n');
        println("Error: Could not create file.");
        return;
    }

    putchar('\n');
    println("Write contents (type :s and press enter to save): ");

    char data[512] = "";
    int idx = 0;
    bool running = true;
    while (running) {
        char key = get_key();
        if (!key) continue;

        if (key == '\n' && idx >= 2 && data[idx - 1] == 's' && data[idx - 2] == ':') {
            data[idx - 2] = '\0';
            running = false;
        }
        else if (key == 8) {
            if (idx > 0) {
                idx--;
                data[idx] = '\0';
                cursorX--;
                putchar(' ');
                cursorX--;
            }
        }
        else if (idx < 511) {
            putchar(key);
            data[idx++] = key;
            data[idx] = '\0';
        }
    }

    vfs_write_file(path2, data);
    putchar('\n');
    println("File saved.");
}

void read() {
    print("File name: ");
    char path[64] = "";
    read_input(path, 64);

    if (contains_sep(path)) {
        putchar('\n');
        println("Use a single file name (no path separators).");
        return;
    }

    path_prepend(path);

    char read_buffer[512];
    if (vfs_read_file(path, read_buffer)) {
        putchar('\n');
        println(read_buffer);
    }
    else {
        putchar('\n');
        print("Error: ");
        print(path);
        println(" not found.");
    }
}

void rmf() {
    print("File name: ");
    char path2[64] = "";
    read_input(path2, 64);

    if (contains_sep(path2)) {
        putchar('\n');
        println("Use a single file name (no path separators).");
        return;
    }

    path_prepend(path2);

    if (!vfs_delete_file(path2)) {
        putchar('\n');
        println("File not found.");
    } else {
        putchar('\n');
        println("File deleted.");
    }
}
