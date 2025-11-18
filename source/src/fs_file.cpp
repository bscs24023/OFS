#include "../include/fs_file.hpp"
#include "../include/fs_core.hpp"
#include <cstring>
#include <ctime>

using namespace std;

extern FileSystemInstance* g_fs;

int file_create(void* session, const char* path, const char* data, size_t size)
{
    if (!session || !path || !data)
        return (int)OFSErrorCodes::ERROR_INVALID_OPERATION;

    for (auto& f : g_fs->files)
        if (strcmp(f.path, path) == 0)
            return (int)OFSErrorCodes::ERROR_FILE_EXISTS;

    SessionInfo* s = (SessionInfo*)session;

    FileMetadata meta{};
    strncpy(meta.path, path, sizeof(meta.path) - 1);

    meta.entry = FileEntry(
        path,
        EntryType::FILE,
        size,
        0644,
        s->user.username,
        (uint32_t)(g_fs->files.size() + 1)
    );

    meta.entry.created_time = meta.entry.modified_time = time(nullptr);
    meta.actual_size = size;
    meta.blocks_used = (size + 4095) / 4096;

    meta.content = string(data, size);

    g_fs->files.push_back(meta);
    g_fs->stats.total_files++;
    g_fs->stats.used_space += size;
    g_fs->stats.free_space -= size;

    return (int)OFSErrorCodes::SUCCESS;
}


int file_read(void* session, const char* path, char** buffer, size_t* size)
{
    if (!session || !path || !buffer || !size)
        return (int)OFSErrorCodes::ERROR_INVALID_OPERATION;

    for (auto& f : g_fs->files)
    {
        if (strcmp(f.path, path) == 0)
        {
            const string& c = f.content;

            *size = c.size();
            *buffer = new char[*size + 1];

            if (*size > 0)
                memcpy(*buffer, c.data(), *size);

            (*buffer)[*size] = '\0';
            return (int)OFSErrorCodes::SUCCESS;
        }
    }
    return (int)OFSErrorCodes::ERROR_NOT_FOUND;
}

int file_edit(void* session, const char* path, const char* data, size_t size, unsigned int index)
{
    if (!session || !path || !data)
        return (int)OFSErrorCodes::ERROR_INVALID_OPERATION;

    for (auto& f : g_fs->files)
    {
        if (strcmp(f.path, path) == 0)
        {
            string& content = f.content;

            if (index > content.size())
                content.resize(index, '\0');

            if (index + size <= content.size())
            {
                content.replace(index, size, data, size);
            }
            else
            {
                size_t overwrite = (index < content.size() ? content.size() - index : 0);
                if (overwrite > 0)
                    content.replace(index, overwrite, data, overwrite);
                content.append(data + overwrite, size - overwrite);
            }

            uint64_t old_sz = f.actual_size;
            uint64_t new_sz = content.size();

            f.actual_size = new_sz;
            f.entry.size = new_sz;
            f.entry.modified_time = time(nullptr);

            if (new_sz > old_sz)
            {
                uint64_t diff = new_sz - old_sz;
                g_fs->stats.used_space += diff;
                g_fs->stats.free_space -= diff;
            }
            else if (old_sz > new_sz)
            {
                uint64_t diff = old_sz - new_sz;
                g_fs->stats.used_space -= diff;
                g_fs->stats.free_space += diff;
            }

            return (int)OFSErrorCodes::SUCCESS;
        }
    }
    return (int)OFSErrorCodes::ERROR_NOT_FOUND;
}

int file_delete(void* session, const char* path)
{
    if (!session || !path)
        return (int)OFSErrorCodes::ERROR_INVALID_OPERATION;

    for (size_t i = 0; i < g_fs->files.size(); ++i)
    {
        if (strcmp(g_fs->files[i].path, path) == 0)
        {
            uint64_t removed = g_fs->files[i].content.size();

            g_fs->stats.used_space -= removed;
            g_fs->stats.free_space += removed;
            g_fs->stats.total_files--;

            g_fs->files.erase(g_fs->files.begin() + i);
            return (int)OFSErrorCodes::SUCCESS;
        }
    }
    return (int)OFSErrorCodes::ERROR_NOT_FOUND;
}

int file_truncate(void* session, const char* path)
{
    if (!session || !path)
        return (int)OFSErrorCodes::ERROR_INVALID_OPERATION;

    for (auto& f : g_fs->files)
    {
        if (strcmp(f.path, path) == 0)
        {
            uint64_t removed = f.content.size();

            f.content.clear();
            f.entry.size = 0;
            f.actual_size = 0;

            g_fs->stats.used_space -= removed;
            g_fs->stats.free_space += removed;

            f.entry.modified_time = time(nullptr);

            return (int)OFSErrorCodes::SUCCESS;
        }
    }
    return (int)OFSErrorCodes::ERROR_NOT_FOUND;
}

int file_exists(void* session, const char* path)
{
    if (!session || !path)
        return (int)OFSErrorCodes::ERROR_INVALID_OPERATION;

    for (auto& f : g_fs->files)
        if (strcmp(f.path, path) == 0)
            return (int)OFSErrorCodes::SUCCESS;

    return (int)OFSErrorCodes::ERROR_NOT_FOUND;
}

int file_rename(void* session, const char* old_path, const char* new_path)
{
    if (!session || !old_path || !new_path)
        return (int)OFSErrorCodes::ERROR_INVALID_OPERATION;

    for (auto& f : g_fs->files)
    {
        if (strcmp(f.path, old_path) == 0)
        {
            strncpy(f.path, new_path, sizeof(f.path) - 1);

            string p(new_path);
            size_t slash = p.find_last_of('/');
            string name = (slash == string::npos) ? p : p.substr(slash + 1);

            strncpy(f.entry.name, name.c_str(), sizeof(f.entry.name) - 1);

            f.entry.modified_time = time(nullptr);

            return (int)OFSErrorCodes::SUCCESS;
        }
    }
    return (int)OFSErrorCodes::ERROR_NOT_FOUND;
}
