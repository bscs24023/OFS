#include "../include/fs_dir.hpp"
#include "../include/fs_core.hpp"
#include <cstring>
#include <vector>

using namespace std;
#define ERROR_DIR_NOT_EMPTY -8

extern FileSystemInstance* g_fs;

int dir_create(void* session, const char* path) 
{
    if (!session || !path) 
    {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }

    for (auto& f : g_fs->files) 
    {
        if (strcmp(f.path, path) == 0) 
        {
            return static_cast<int>(OFSErrorCodes::ERROR_FILE_EXISTS);
        }
    }

    FileMetadata dirMeta;
    memset(&dirMeta, 0, sizeof(dirMeta));
    strncpy(dirMeta.path, path, sizeof(dirMeta.path) - 1);
    dirMeta.entry = FileEntry(path, EntryType::DIRECTORY, 0, 0755, "system", static_cast<uint32_t>(g_fs->files.size() + 1));
    dirMeta.entry.created_time = dirMeta.entry.modified_time = static_cast<uint64_t>(time(nullptr));

    g_fs->files.push_back(dirMeta);
    g_fs->stats.total_directories++;

    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int dir_list(void* session, const char* path, FileEntry** entries, int* count) {
    if (!session || !path || !entries || !count) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }

    vector<FileEntry> result;
    size_t prefixLen = strlen(path);
    for (auto& f : g_fs->files) 
    {
        if (strncmp(f.path, path, prefixLen) == 0 && f.path[prefixLen] == '/' && strcmp(f.path, path) != 0) 
        {
            result.push_back(f.entry);
        }
    }

    if (result.empty()) {
        *entries = nullptr;
        *count = 0;
        return static_cast<int>(OFSErrorCodes::SUCCESS);
    }

    *count = static_cast<int>(result.size());
    *entries = new FileEntry[*count];
    for (int i = 0; i < *count; i++) {
        (*entries)[i] = result[i];
    }

    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int dir_delete(void* session, const char* path) 
{
    if (!session || !path) 
    {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }

    size_t prefixLen = strlen(path);
    for (auto& f : g_fs->files) 
    {
        if (strncmp(f.path, path, prefixLen) == 0 && f.path[prefixLen] == '/' && strcmp(f.path, path) != 0) 
        {
            return ERROR_DIR_NOT_EMPTY;
        }
    }

    for (size_t i = 0; i < g_fs->files.size(); ++i) 
    {
        if (strcmp(g_fs->files[i].path, path) == 0 && (EntryType)g_fs->files[i].entry.type == EntryType::DIRECTORY) 
        {
            g_fs->files.erase(g_fs->files.begin() + i);
        
            g_fs->stats.total_directories--;
            return static_cast<int>(OFSErrorCodes::SUCCESS);
        }
    }

    return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
}

int dir_exists(void* session, const char* path) 
{
    if (!session || !path) 
    {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }

    for (auto& f : g_fs->files) 
    {

        if (strcmp(f.path, path) == 0 && (EntryType)f.entry.type == EntryType::DIRECTORY) {
            return static_cast<int>(OFSErrorCodes::SUCCESS);
        }
    }

    return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
}
