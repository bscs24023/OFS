#include "../include/fs_info.hpp"
#include "../include/fs_core.hpp"
#include <cstring>

using namespace std;

extern FileSystemInstance* g_fs;

int get_metadata(void* session, const char* path, FileMetadata* meta) 
{
    if (!session || !path || !meta) 
    {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }

    for (auto& f : g_fs->files) 
    {
        if (strcmp(f.path, path) == 0) 
        {
            *meta = f;
            return static_cast<int>(OFSErrorCodes::SUCCESS);
        }
    }

    return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
}

int set_permissions(void* session, const char* path, uint32_t permissions) 
{
    if (!session || !path) 
    {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }

    for (auto& f : g_fs->files) 
    {
        if (strcmp(f.path, path) == 0) 
        {
            f.entry.permissions = permissions;
            f.entry.modified_time = static_cast<uint64_t>(time(nullptr));
            return static_cast<int>(OFSErrorCodes::SUCCESS);
        }
    }

    return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
}

int get_stats(void* session, FSStats* stats) 
{
    if (!session || !stats) 
    {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }

    *stats = g_fs->stats;
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

void free_buffer(void* buffer) 
{
    if (buffer) 
    {
        delete[] reinterpret_cast<char*>(buffer);
    }
}

const char* get_error_message(int error_code) 
{
    switch (error_code) 
    {
        case 0: return "SUCCESS";
        case -1: return "ERROR_INVALID_OPERATION";
        case -2: return "ERROR_IO_ERROR";
        case -3: return "ERROR_NOT_FOUND";
        case -4: return "ERROR_FILE_EXISTS";
        case -5: return "ERROR_PERMISSION_DENIED";
        case -6: return "ERROR_INVALID_SESSION";
        case -7: return "ERROR_INVALID_CONFIG";
        case -8: return "ERROR_FILE_TOO_LARGE";
        case -9: return "ERROR_STORAGE_FULL";
        case -10: return "ERROR_DIR_NOT_EMPTY";
        case -11: return "ERROR_UNKNOWN";
        default: return "UNKNOWN_ERROR_CODE";
    }
}
