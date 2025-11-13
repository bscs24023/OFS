#ifndef FS_INFO_HPP
#define FS_INFO_HPP

#include "../include/odf_types.hpp"

int get_metadata(void* session, const char* path, FileMetadata* meta);
int set_permissions(void* session, const char* path, uint32_t permissions);
int get_stats(void* session, FSStats* stats);
void free_buffer(void* buffer);
const char* get_error_message(int error_code);

#endif
