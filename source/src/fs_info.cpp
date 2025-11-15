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
        case static_cast<int>(OFSErrorCodes::SUCCESS):
            return "SUCCESS";
        case static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND):
            return "ERROR_NOT_FOUND";
        case static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED):
            return "ERROR_PERMISSION_DENIED";
        case static_cast<int>(OFSErrorCodes::ERROR_IO_ERROR):
            return "ERROR_IO_ERROR";
        case static_cast<int>(OFSErrorCodes::ERROR_INVALID_PATH):
            return "ERROR_INVALID_PATH";
        case static_cast<int>(OFSErrorCodes::ERROR_FILE_EXISTS):
            return "ERROR_FILE_EXISTS";
        case static_cast<int>(OFSErrorCodes::ERROR_NO_SPACE):
            return "ERROR_NO_SPACE";
        case static_cast<int>(OFSErrorCodes::ERROR_INVALID_CONFIG):
            return "ERROR_INVALID_CONFIG";
        case static_cast<int>(OFSErrorCodes::ERROR_NOT_IMPLEMENTED):
            return "ERROR_NOT_IMPLEMENTED";
        case static_cast<int>(OFSErrorCodes::ERROR_INVALID_SESSION):
            return "ERROR_INVALID_SESSION";
        case static_cast<int>(OFSErrorCodes::ERROR_DIRECTORY_NOT_EMPTY):
            return "ERROR_DIRECTORY_NOT_EMPTY";
        case static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION):
            return "ERROR_INVALID_OPERATION";
        default:
            return "UNKNOWN_ERROR_CODE";
    }
}
