#include "fs.h"
#include "ata.h"

extern void print(const char* str);
extern void println(const char* str);
extern void printint(int num);
extern bool strcmp(const char* a, const char* b);
extern void reboot();
extern void putchar(char c);
extern char get_key();
extern void sleep();

extern int cursorX;

#define SECTOR_SIZE 512
#define FS_MAGIC 0x4D4D5346

#define FAT_FREE 0x0000
#define FAT_EOC  0xFFFF

#define ROOT_DIR 0xFFFE

#define MAX_CLUSTERS 224
#define MAX_DIR_ENTRIES 64

#define SUPERBLOCK_LBA 1
#define FAT_LBA 2
#define DIR_LBA 3
#define DIR_SECTORS 8
#define DATA_LBA (DIR_LBA + DIR_SECTORS)

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint16_t max_clusters;
    uint16_t max_dir_entries;
    uint16_t root_id;
    uint16_t reserved;
} Superblock;

typedef struct __attribute__((packed)) {
    char name[24];
    uint16_t parent;
    uint16_t first_cluster;
    uint32_t size;
    uint8_t flags; /* bit0: used, bit1: is_dir */
} DirEntry;

static Superblock superblock;
static uint16_t fat[MAX_CLUSTERS + 2];
static DirEntry dir_table[MAX_DIR_ENTRIES];
static uint8_t io_buffer[SECTOR_SIZE];
static char cwd_path[64] = "0:\\";

static const char* default_demo_source =
    "int main() {\n"
    "println(\"Hello from Tiny C!\");\n"
    "int answer = 2 + 3 * 4;\n"
    "printint(answer);\n"
    "println(\"\");\n"
    "return answer;\n"
    "}";

static int str_len(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

static void str_cpy(char* dest, const char* src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

static bool str_starts_with(const char* str, const char* pref) {
    int i = 0;
    while (pref[i]) {
        if (str[i] != pref[i]) return false;
        i++;
    }
    return true;
}

static bool is_path_sep(char c) {
    return c == '\\' || c == '/';
}

static void mem_zero(uint8_t* ptr, int len) {
    for (int i = 0; i < len; i++) ptr[i] = 0;
}

static bool is_used(const DirEntry* e) { return (e->flags & 0x01) != 0; }
static bool is_dir(const DirEntry* e) { return (e->flags & 0x02) != 0; }

static void fs_flush_fat() {
    ata_write_sector(FAT_LBA, (uint8_t*)fat);
}

static void fs_flush_dirs() {
    const uint8_t* ptr = (const uint8_t*)dir_table;
    for (int i = 0; i < DIR_SECTORS; i++) {
        for (int j = 0; j < SECTOR_SIZE; j++) io_buffer[j] = ptr[i * SECTOR_SIZE + j];
        ata_write_sector(DIR_LBA + i, io_buffer);
    }
}

static void fs_load_fat() {
    ata_read_sector(FAT_LBA, (uint8_t*)fat);
}

static void fs_load_dirs() {
    uint8_t* ptr = (uint8_t*)dir_table;
    for (int i = 0; i < DIR_SECTORS; i++) {
        ata_read_sector(DIR_LBA + i, io_buffer);
        for (int j = 0; j < SECTOR_SIZE; j++) ptr[i * SECTOR_SIZE + j] = io_buffer[j];
    }
}

static int dir_find_child(uint16_t parent, const char* name) {
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (is_used(&dir_table[i]) && dir_table[i].parent == parent && strcmp(dir_table[i].name, name)) {
            return i;
        }
    }
    return -1;
}

static int dir_alloc_entry() {
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (!is_used(&dir_table[i])) return i;
    }
    return -1;
}

static bool parse_component(const char* path, int* p, char* out) {
    int idx = 0;

    while (path[*p] && is_path_sep(path[*p])) (*p)++;
    if (!path[*p]) return false;

    while (path[*p] && !is_path_sep(path[*p])) {
        if (idx < 23) out[idx++] = path[*p];
        (*p)++;
    }

    out[idx] = '\0';
    return idx > 0;
}

static int path_start(const char* path) {
    if (path[0] == '0' && path[1] == ':' && is_path_sep(path[2])) return 3;
    return 0;
}

static bool normalize_path(const char* path, char* out) {
    char stack[8][24];
    int count = 0;

    if (!(path[0] == '0' && path[1] == ':' && is_path_sep(path[2]))) {
        int cp = 3;
        char part[24];
        while (parse_component(cwd_path, &cp, part)) {
            str_cpy(stack[count++], part);
            if (count >= 8) break;
        }
    }

    int p = path_start(path);
    char part[24];
    while (parse_component(path, &p, part)) {
        if (strcmp(part, ".")) {
            continue;
        }
        if (strcmp(part, "..")) {
            if (count > 0) count--;
            continue;
        }
        if (count >= 8) return false;
        str_cpy(stack[count++], part);
    }

    int o = 0;
    out[o++] = '0';
    out[o++] = ':';
    out[o++] = '\\';

    for (int i = 0; i < count; i++) {
        for (int j = 0; stack[i][j] && o < 63; j++) out[o++] = stack[i][j];
        if (i < count - 1 && o < 63) out[o++] = '\\';
    }

    out[o] = '\0';
    return true;
}

static bool is_hidden_path(const char* absolute_path) {
    return strcmp(absolute_path, "0:\\data") || str_starts_with(absolute_path, "0:\\data\\");
}

static bool resolve_parent_and_leaf(const char* absolute_path, uint16_t* parent_out, char* leaf_out, bool create_dirs) {
    int p = path_start(absolute_path);
    uint16_t current = ROOT_DIR;
    char part[24];
    char next_part[24];

    if (!parse_component(absolute_path, &p, part)) return false;

    while (1) {
        int saved = p;
        bool has_next = parse_component(absolute_path, &saved, next_part);

        if (!has_next) {
            *parent_out = current;
            str_cpy(leaf_out, part);
            return true;
        }

        int child = dir_find_child(current, part);
        if (child < 0) {
            if (!create_dirs) return false;
            int n = dir_alloc_entry();
            if (n < 0) return false;
            str_cpy(dir_table[n].name, part);
            dir_table[n].parent = current;
            dir_table[n].first_cluster = FAT_FREE;
            dir_table[n].size = 0;
            dir_table[n].flags = 0x01 | 0x02;
            current = (uint16_t)n;
            fs_flush_dirs();
        } else {
            if (!is_dir(&dir_table[child])) return false;
            current = (uint16_t)child;
        }

        p = saved;
        str_cpy(part, next_part);
    }
}

static int resolve_exact_path(const char* absolute_path) {
    int p = path_start(absolute_path);
    uint16_t current = ROOT_DIR;
    char part[24];

    if (!parse_component(absolute_path, &p, part)) return ROOT_DIR;

    while (1) {
        int child = dir_find_child(current, part);
        if (child < 0) return -1;

        if (!parse_component(absolute_path, &p, part)) return child;

        if (!is_dir(&dir_table[child])) return -1;
        current = (uint16_t)child;
    }
}

static void fat_free_chain(uint16_t first) {
    uint16_t cur = first;
    while (cur >= 2 && cur < (MAX_CLUSTERS + 2)) {
        uint16_t next = fat[cur];
        fat[cur] = FAT_FREE;
        if (next == FAT_EOC || next == FAT_FREE) break;
        cur = next;
    }
}

static uint16_t fat_alloc_chain(int clusters) {
    uint16_t first = FAT_FREE;
    uint16_t prev = FAT_FREE;

    for (int i = 0; i < clusters; i++) {
        uint16_t found = FAT_FREE;
        for (uint16_t c = 2; c < (MAX_CLUSTERS + 2); c++) {
            if (fat[c] == FAT_FREE) {
                found = c;
                break;
            }
        }

        if (found == FAT_FREE) {
            fat_free_chain(first);
            return FAT_FREE;
        }

        fat[found] = FAT_EOC;
        if (first == FAT_FREE) first = found;
        if (prev != FAT_FREE) fat[prev] = found;
        prev = found;
    }

    return first;
}

static void write_file_data(uint16_t first_cluster, const char* data, int size) {
    uint16_t cur = first_cluster;
    int offset = 0;

    while (cur != FAT_FREE) {
        for (int i = 0; i < SECTOR_SIZE; i++) {
            if (offset < size) io_buffer[i] = (uint8_t)data[offset++];
            else io_buffer[i] = 0;
        }

        ata_write_sector(DATA_LBA + (cur - 2), io_buffer);

        if (fat[cur] == FAT_EOC) break;
        cur = fat[cur];
    }
}

static bool read_file_data(uint16_t first_cluster, char* out, int size) {
    uint16_t cur = first_cluster;
    int offset = 0;

    while (cur != FAT_FREE && offset < size) {
        ata_read_sector(DATA_LBA + (cur - 2), io_buffer);
        for (int i = 0; i < SECTOR_SIZE && offset < size; i++) {
            out[offset++] = (char)io_buffer[i];
        }

        if (fat[cur] == FAT_EOC) break;
        cur = fat[cur];
    }

    out[size] = '\0';
    return true;
}

static void vfs_seed_defaults() {
    char read_buffer[SECTOR_SIZE + 1];

    vfs_make_dir("0:\\data");

    if (!vfs_read_file("0:\\test.txt", read_buffer)) {
        vfs_write_file("0:\\test.txt", "Hello, curious user!");
    }
    if (!vfs_read_file("0:\\music\\ode.md", read_buffer)) {
        vfs_write_file("0:\\music\\ode.md", "370 400\n370 400\n392 400\n440 400\n440 400\n392 400\n370 400\n330 400\n294 400\n294 400\n330 400\n370 400\n370 600\n330 200\n330 800");
    }
    if (!vfs_read_file("0:\\demo\\demo.c", read_buffer) || !strcmp(read_buffer, default_demo_source)) {
        vfs_write_file("0:\\demo\\demo.c", default_demo_source);
    }
}

void vfs_init() {
    ata_read_sector(SUPERBLOCK_LBA, (uint8_t*)&superblock);

    if (superblock.magic != FS_MAGIC || superblock.max_clusters != MAX_CLUSTERS || superblock.max_dir_entries != MAX_DIR_ENTRIES) {
        println("Formatting FAT-like drive...");

        superblock.magic = FS_MAGIC;
        superblock.max_clusters = MAX_CLUSTERS;
        superblock.max_dir_entries = MAX_DIR_ENTRIES;
        superblock.root_id = ROOT_DIR;
        superblock.reserved = 0;

        mem_zero((uint8_t*)fat, sizeof(fat));
        mem_zero((uint8_t*)dir_table, sizeof(dir_table));

        ata_write_sector(SUPERBLOCK_LBA, (uint8_t*)&superblock);
        fs_flush_fat();
        fs_flush_dirs();
    } else {
        println("FAT-like disk mounted successfully.");
        fs_load_fat();
        fs_load_dirs();
    }

    str_cpy(cwd_path, "0:\\");
    vfs_seed_defaults();
}

bool vfs_make_dir(const char* path) {
    char abs[64];
    if (!normalize_path(path, abs)) return false;

    int existing = resolve_exact_path(abs);
    if (existing >= 0) return (existing == ROOT_DIR) || is_dir(&dir_table[existing]);

    uint16_t parent;
    char leaf[24];
    if (!resolve_parent_and_leaf(abs, &parent, leaf, true)) return false;

    int idx = dir_alloc_entry();
    if (idx < 0) return false;

    str_cpy(dir_table[idx].name, leaf);
    dir_table[idx].parent = parent;
    dir_table[idx].first_cluster = FAT_FREE;
    dir_table[idx].size = 0;
    dir_table[idx].flags = 0x01 | 0x02;

    fs_flush_dirs();
    return true;
}

bool vfs_remove_dir(const char* path) {
    char abs[64];
    if (!normalize_path(path, abs)) return false;
    if (strcmp(abs, "0:\\") || is_hidden_path(abs)) return false;

    int idx = resolve_exact_path(abs);
    if (idx < 0 || idx == ROOT_DIR || !is_dir(&dir_table[idx])) return false;

    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (is_used(&dir_table[i]) && dir_table[i].parent == (uint16_t)idx) return false;
    }

    dir_table[idx].flags = 0;
    dir_table[idx].name[0] = '\0';
    dir_table[idx].size = 0;
    dir_table[idx].first_cluster = FAT_FREE;
    fs_flush_dirs();
    return true;
}

bool vfs_change_dir(const char* path) {
    char abs[64];
    if (!normalize_path(path, abs)) return false;
    if (is_hidden_path(abs)) return false;

    int idx = resolve_exact_path(abs);
    if (idx == ROOT_DIR || (idx >= 0 && is_dir(&dir_table[idx]))) {
        str_cpy(cwd_path, abs);
        return true;
    }

    return false;
}

void vfs_get_cwd(char* out) {
    str_cpy(out, cwd_path);
}

bool vfs_write_file(const char* path, const char* data) {
    int file_len = str_len(data);
    if (file_len > SECTOR_SIZE * 6) return false;

    char abs[64];
    if (!normalize_path(path, abs)) return false;

    uint16_t parent;
    char leaf[24];
    if (!resolve_parent_and_leaf(abs, &parent, leaf, true)) return false;

    int target = dir_find_child(parent, leaf);
    if (target >= 0 && is_dir(&dir_table[target])) return false;

    if (target < 0) {
        target = dir_alloc_entry();
        if (target < 0) return false;

        str_cpy(dir_table[target].name, leaf);
        dir_table[target].parent = parent;
        dir_table[target].first_cluster = FAT_FREE;
        dir_table[target].size = 0;
        dir_table[target].flags = 0x01;
    }

    if (dir_table[target].first_cluster != FAT_FREE) {
        fat_free_chain(dir_table[target].first_cluster);
    }

    if (file_len == 0) {
        dir_table[target].first_cluster = FAT_FREE;
        dir_table[target].size = 0;
        fs_flush_fat();
        fs_flush_dirs();
        return true;
    }

    int clusters_needed = (file_len + SECTOR_SIZE - 1) / SECTOR_SIZE;
    uint16_t first = fat_alloc_chain(clusters_needed);
    if (first == FAT_FREE) return false;

    dir_table[target].first_cluster = first;
    dir_table[target].size = (uint32_t)file_len;

    write_file_data(first, data, file_len);

    fs_flush_fat();
    fs_flush_dirs();
    return true;
}

bool vfs_read_file(const char* path, char* buffer_out) {
    char abs[64];
    if (!normalize_path(path, abs)) return false;

    int entry = resolve_exact_path(abs);
    if (entry < 0 || entry == ROOT_DIR || is_dir(&dir_table[entry])) return false;

    if (dir_table[entry].size == 0) {
        buffer_out[0] = '\0';
        return true;
    }

    return read_file_data(dir_table[entry].first_cluster, buffer_out, (int)dir_table[entry].size);
}

void vfs_list_current_dir() {
    int cwd = resolve_exact_path(cwd_path);

    println("------ Directory Listing ------");
    bool empty = true;

    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (!is_used(&dir_table[i]) || dir_table[i].parent != (uint16_t)cwd) continue;

        char full[64];
        if (strcmp(cwd_path, "0:\\")) {
            str_cpy(full, "0:\\");
            str_cpy(full + 3, dir_table[i].name);
        } else {
            str_cpy(full, cwd_path);
            int pos = str_len(full);
            full[pos++] = '\\';
            str_cpy(full + pos, dir_table[i].name);
        }

        if (is_hidden_path(full)) continue;

        if (is_dir(&dir_table[i])) print("[DIR] ");
        else print("[FILE] ");
        print(dir_table[i].name);
        if (!is_dir(&dir_table[i])) {
            print(" (");
            printint((int)dir_table[i].size);
            print(" bytes)");
        }
        putchar('\n');
        empty = false;
    }

    if (empty) println("(empty)");
    println("-------------------------------");
}

void vfs_list_files() {
    vfs_list_current_dir();
}

bool vfs_delete_file(const char* path) {
    char abs[64];
    if (!normalize_path(path, abs)) return false;

    int entry = resolve_exact_path(abs);
    if (entry < 0 || entry == ROOT_DIR || is_dir(&dir_table[entry])) return false;

    if (dir_table[entry].first_cluster != FAT_FREE) {
        fat_free_chain(dir_table[entry].first_cluster);
    }

    dir_table[entry].flags = 0;
    dir_table[entry].name[0] = '\0';
    dir_table[entry].size = 0;
    dir_table[entry].first_cluster = FAT_FREE;

    fs_flush_fat();
    fs_flush_dirs();
    return true;
}

bool vfs_read_file_line(const char* path, char* line_out) {
    static char loaded_path[64];
    static char file_buffer[SECTOR_SIZE * 6 + 1];
    static int index = 0;
    static bool loaded = false;

    char abs[64];
    if (!normalize_path(path, abs)) return false;

    if (!loaded || !strcmp(loaded_path, abs)) {
        if (!vfs_read_file(abs, file_buffer)) {
            loaded = false;
            return false;
        }
        str_cpy(loaded_path, abs);
        index = 0;
        loaded = true;
    }

    if (file_buffer[index] == '\0') {
        loaded = false;
        return false;
    }

    int i = 0;
    while (file_buffer[index] != '\n' && file_buffer[index] != '\0') {
        line_out[i++] = file_buffer[index++];
    }

    if (file_buffer[index] == '\n') index++;

    line_out[i] = '\0';
    return true;
}

int vfs_file_count() {
    int count = 0;

    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (is_used(&dir_table[i]) && !is_dir(&dir_table[i])) count++;
    }

    return count;
}

void vfs_reset() {
    print("Are you sure you would like to reset this system? (y/n): ");
    char ans[64] = "";
    int ans_index = 0;
    bool running = true;
    while (running) {
        char key = get_key();

        if (!key) continue;
        if (key == '\n') running = false;
        else if (key == 8) {
            if (ans_index > 0) {
                ans_index--;
                ans[ans_index] = '\0';
                cursorX--;
                putchar(' ');
                cursorX--;
            }
        }
        else if ((key == 'y' || key == 'n') && ans_index <= 1) {
            putchar(key);
            ans[ans_index] = key;
            ans_index++;
        }
    }

    if (strcmp(ans, "y")) {
        mem_zero((uint8_t*)fat, sizeof(fat));
        mem_zero((uint8_t*)dir_table, sizeof(dir_table));
        fs_flush_fat();
        fs_flush_dirs();
        str_cpy(cwd_path, "0:\\");

        vfs_seed_defaults();
        putchar('\n');
        println("System reset. Rebooting...");
        sleep(20000);
        reboot();
    }
    else println("");
}
