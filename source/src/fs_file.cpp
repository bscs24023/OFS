#include "../include/fs_file.hpp"
#include "../include/fs_core.hpp"
#include <cstring>
#include <ctime>

using namespace std;

extern FileSystemInstance* g_fs;

int file_create(void* session, const char* path, const char* data, size_t size) {
    if (!session || !path || !data) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }

    SessionInfo* s = reinterpret_cast<SessionInfo*>(session);
    if (file_exists(session, path) == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        return static_cast<int>(OFSErrorCodes::ERROR_FILE_EXISTS);
    }

    FileMetadata meta;
    memset(&meta, 0, sizeof(meta));
    strncpy(meta.path, path, sizeof(meta.path) - 1);
    meta.entry = FileEntry(path, EntryType::FILE, size, 0644, s->user.username, static_cast<uint32_t>(g_fs->files.size() + 1));
    meta.entry.created_time = meta.entry.modified_time = static_cast<uint64_t>(time(nullptr));
    meta.actual_size = size;
    meta.blocks_used = (size + 4095) / 4096;

    g_fs->files.push_back(meta);
    g_fs->stats.total_files++;
    g_fs->stats.used_space += size;
    g_fs->stats.free_space -= size;

    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int file_read(void* session, const char* path, char** buffer, size_t* size) {
    if (!session || !path || !buffer || !size) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }

    for (auto& f : g_fs->files) {
        if (strcmp(f.path, path) == 0) {
            *size = f.actual_size;
            *buffer = new char[*size + 1];
            memset(*buffer, 0, *size + 1);
            strcpy(*buffer, "[File data placeholder]");
            return static_cast<int>(OFSErrorCodes::SUCCESS);
        }
    }

    return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
}

int file_edit(void* session, const char* path, const char* data, size_t size, unsigned int index) {
    if (!session || !path || !data) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }

    for (auto& f : g_fs->files) {
        if (strcmp(f.path, path) == 0) {
            f.entry.modified_time = static_cast<uint64_t>(time(nullptr));
            f.actual_size = size;
            return static_cast<int>(OFSErrorCodes::SUCCESS);
        }
    }

    return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
}

int file_delete(void* session, const char* path) {
    if (!session || !path) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }

    for (size_t i = 0; i < g_fs->files.size(); i++) {
        if (strcmp(g_fs->files[i].path, path) == 0) {
            g_fs->stats.used_space -= g_fs->files[i].actual_size;
            g_fs->stats.free_space += g_fs->files[i].actual_size;
            g_fs->stats.total_files--;
            g_fs->files.erase(g_fs->files.begin() + i);
            return static_cast<int>(OFSErrorCodes::SUCCESS);
        }
    }

    return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
}

int file_truncate(void* session, const char* path) {
    if (!session || !path) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }

    for (auto& f : g_fs->files) {
        if (strcmp(f.path, path) == 0) {
            g_fs->stats.used_space -= f.actual_size;
            g_fs->stats.free_space += f.actual_size;
            f.actual_size = 0;
            f.entry.size = 0;
            f.entry.modified_time = static_cast<uint64_t>(time(nullptr));
            return static_cast<int>(OFSErrorCodes::SUCCESS);
        }
    }

    return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
}

int file_exists(void* session, const char* path) {
    if (!session || !path) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }

    for (auto& f : g_fs->files) {
        if (strcmp(f.path, path) == 0) {
            return static_cast<int>(OFSErrorCodes::SUCCESS);
        }
    }

    return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
}

int file_rename(void* session, const char* old_path, const char* new_path) {
    if (!session || !old_path || !new_path) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }

    for (auto& f : g_fs->files) {
        if (strcmp(f.path, old_path) == 0) {
            strncpy(f.path, new_path, sizeof(f.path) - 1);
            strncpy(f.entry.name, new_path, sizeof(f.entry.name) - 1);
            f.entry.modified_time = static_cast<uint64_t>(time(nullptr));
            return static_cast<int>(OFSErrorCodes::SUCCESS);
        }
    }

    return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
}
