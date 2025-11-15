#include "../include/fs_dir.hpp"
#include "../include/fs_core.hpp"
#include <cstring>
#include <vector>

using namespace std;

extern FileSystemInstance* g_fs;

static bool is_direct_child(const char* full, const char* parent)
{
    int plen = strlen(parent);
    if (strncmp(full, parent, plen) != 0) return false;
    if (full[plen] != '/') return false;
    const char* rest = full + plen + 1;
    return strchr(rest, '/') == nullptr;
}

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

int dir_list(void* session, const char* path, FileEntry** entries, int* count)
{
    if (!session || !path || !entries || !count)
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);

    vector<FileEntry> result;
    string base = string(path);

    if (base.size() > 1 && base.back() == '/')
        base.pop_back();

    size_t baseLen = base.size();

    for (auto& f : g_fs->files)
    {
        string fp = string(f.path);

        if (fp == base) continue;

        if (fp.size() > baseLen &&
            fp.compare(0, baseLen, base) == 0 &&
            fp[baseLen] == '/')
        {
            string rest = fp.substr(baseLen + 1);
            if (rest.find('/') == string::npos)
                result.push_back(f.entry);
        }
    }

    *count = result.size();
    if (*count == 0)
    {
        *entries = nullptr;
        return static_cast<int>(OFSErrorCodes::SUCCESS);
    }

    *entries = new FileEntry[*count];
    for (int i = 0; i < *count; ++i)
        (*entries)[i] = result[i];

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
        if (strcmp(f.path, path) != 0 && is_direct_child(f.path, path))
        {
            return (int)OFSErrorCodes::ERROR_DIRECTORY_NOT_EMPTY;
        }

    }

    for (size_t i = 0; i < g_fs->files.size(); ++i) 
    {
        if (strcmp(g_fs->files[i].path, path) == 0 && g_fs->files[i].entry.type == (uint8_t)EntryType::DIRECTORY) 
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

        if (strcmp(f.path, path) == 0 && f.entry.type == (uint8_t)EntryType::DIRECTORY) {
            return static_cast<int>(OFSErrorCodes::SUCCESS);
        }
    }

    return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
}
