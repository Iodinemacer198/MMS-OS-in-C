#ifndef FSC_H
#define FSC_H

#include <stdbool.h>
#include <stdint.h>

void mkf(char* path2);
void read(char* path);
void rmf(char* path2);
void ls();
void cd(const char* path);
void pwd();
void mkdir_cmd(char* name);
void rmdir_cmd(char* name);

#endif
