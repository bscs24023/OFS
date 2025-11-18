#ifndef FS_CORE_HPP
#define FS_CORE_HPP

#include <vector>
#include <string>
#include "odf_types.hpp"
#include "fs_user.hpp"

using namespace std;

struct FileSystemInstance
{
    vector<FileMetadata> files;
    vector<string> file_contents;
    vector<UserInfo> users;
    vector<SessionInfo*> sessions;
    string omni_path;
    string config_path;
    FreeBitmap bitmap;
    FSStats stats;
};

extern FileSystemInstance* g_fs;

int fs_init(void** instance, const char* omni_path, const char* config_path);
int fs_format(const char* omni_path, const char* config_path);
void fs_shutdown(void* instance);

#endif

