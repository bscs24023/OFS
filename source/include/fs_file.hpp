#ifndef FS_FILE_HPP
#define FS_FILE_HPP

#include <string>
#include <vector>
#include "odf_types.hpp"
#include "fs_user.hpp"
#include "dir_tree.hpp"
#include "path_index.hpp"
#include "free_bitmap.hpp"

using namespace std;

int file_create(void* session, const char* path, const char* data, size_t size);
int file_read(void* session, const char* path, char** buffer, size_t* size);
int file_edit(void* session, const char* path, const char* data, size_t size, unsigned int index);
int file_delete(void* session, const char* path);
int file_truncate(void* session, const char* path);
int file_exists(void* session, const char* path);
int file_rename(void* session, const char* old_path, const char* new_path);

#endif
