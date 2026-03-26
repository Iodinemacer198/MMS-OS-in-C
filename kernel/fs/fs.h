#ifndef FS_H
#define FS_H

#include <stdbool.h>
#include <stdint.h>

void vfs_init();
bool vfs_write_file(const char* path, const char* data);
bool vfs_read_file(const char* path, char* buffer_out);
void vfs_list_files();
bool vfs_delete_file(const char* path);
bool vfs_read_file_line(const char* path, char* line_out);
int vfs_file_count();
void vfs_reset();

bool vfs_make_dir(const char* path);
bool vfs_remove_dir(const char* path);
bool vfs_change_dir(const char* path);
void vfs_get_cwd(char* out);
void vfs_list_current_dir();

#endif
