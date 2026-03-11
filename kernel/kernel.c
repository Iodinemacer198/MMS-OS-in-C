#include <stdint.h>
#include <stdbool.h>
#include "fs.h"
#include "calc.h"
#include "login.h"

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

void scroll()
{
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

void putchar(char c)
{
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

void print(const char* str)
{
    for (int i = 0; str[i] != 0; i++)
        putchar(str[i]);
}

void println(const char* str)
{
    print(str);
    putchar('\n');
}

void clear_screen()
{
    for (int y = 0; y < VGA_HEIGHT; y++)
        for (int x = 0; x < VGA_WIDTH; x++)
            vga[y * VGA_WIDTH + x] = (color << 8) | ' ';

    cursorX = 0;
    cursorY = 0;
}

// IO

static inline uint8_t inb(uint16_t port)
{
    uint8_t result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t value)
{
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

void reboot()
{
    while (inb(0x64) & 0x02);
    outb(0x64, 0xFE);

    while (1);
}

void shutdown()
{
    outw(0x604, 0x2000);
}

bool isdigit(char c)
{
    return (c >= '0' && c <= '9');
}

int atoi(const char* str)
{
    int result = 0;
    int i = 0;

    while (str[i] >= '0' && str[i] <= '9')
    {
        result = result * 10 + (str[i] - '0');
        i++;
    }

    return result;
}

void printint(int num)
{
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

char keyboard_map[128] =
{
0,27,'1','2','3','4','5','6','7','8','9','0','-','=',8,9,
'q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s',
'd','f','g','h','j','k','l',';',39,'`',0,'\\','z','x','c','v',
'b','n','m',',','.','/',0,'*',0,' ',0
};

char get_key()
{
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

bool strcmp(const char* a, const char* b)
{
    int i = 0;

    while (a[i] && b[i])
    {
        if (a[i] != b[i])
            return false;
        i++;
    }

    return a[i] == b[i];
}

// Command logic

void run_command()
{
    putchar('\n');

    if (strcmp(cmd_buffer, "help"))
    {
        println("Commands:");
        println("help :: This page!");
        println("about :: OS information");
        println("calc :: Simple calculator");
        println("clear :: Clears the screen");
        println(" ");
        println("test read :: Reads test file");
        println("test view :: Simple FEX");
        println(" ");
        println("reboot :: Reboots system");
        println("shutdown :: Shuts down system");
    }
    else if (strcmp(cmd_buffer, "clear"))
    {
        clear_screen();
    }
    else if (strcmp(cmd_buffer, "about"))
    {
        println("Molecular Multiverse Services OS: developed by the realiodinemacer with C.");
        println("If you need support, contact therealiodinemacer or join ZAx3NN5TJY on Discord.");
    }
    else if (strcmp(cmd_buffer, "test read")) 
    {
        char read_buffer[512];
        if (vfs_read_file("0:\\test.ini", read_buffer)) {
            println(read_buffer);
        } else {
            println("Error: 0:\\test.ini not found.");
        }
    }
    else if (strcmp(cmd_buffer, "test view"))
    {
        vfs_list_files();
    }
    else if (strcmp(cmd_buffer, "calc"))
    {
        // clear_screen();
        run_calc();
    }
    else if (strcmp(cmd_buffer, "shutdown")) shutdown();
    else if (strcmp(cmd_buffer, "reboot")) reboot();
    else
    {
        println("Unknown command");
    }

    println("");
    print("shell > ");

    cmd_index = 0;

    for(int i = 0; i < CMD_BUFFER; i++)
        cmd_buffer[i] = 0;
}

// Main loop

void kernel_main()
{
    clear_screen();

    vfs_init();

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
    println("Type 'help' for commands");
    print("shell > ");

    while (1)
    {
        char key = get_key();

        if (!key)
        {
            continue;
        }
        if (key == '\n')
        {
            run_command();
        }
        else if (key == 8)
        {
            if (cmd_index > 0)
            {
                cmd_index--;
                cursorX--;
                putchar(' ');
                cursorX--;
            }
        }
        else
        {
            putchar(key);
            cmd_buffer[cmd_index] = key;
            cmd_index++;
        }
    }
}