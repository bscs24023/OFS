#ifndef FS_DIR_HPP
#define FS_DIR_HPP

#include "../include/odf_types.hpp"

int dir_create(void* session, const char* path);
int dir_list(void* session, const char* path, FileEntry** entries, int* count);
int dir_delete(void* session, const char* path);
int dir_exists(void* session, const char* path);

#endif
