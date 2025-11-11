#include "../include/fs_core.hpp"
#include "../include/odf_types.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
#include <ctime>

using namespace std;

FileSystemInstance* g_fs = nullptr;

static const unsigned long long DEFAULT_TOTAL_SIZE = 4ULL * 1024 * 1024;
static const unsigned long long DEFAULT_BLOCK_SIZE = 4096ULL;

static int write_header_to_file(const char* path, const OMNIHeader& hdr)
{
    ofstream ofs(path, ios::binary | ios::trunc);
    if (!ofs) return static_cast<int>(OFSErrorCodes::ERROR_IO_ERROR);
    ofs.write(reinterpret_cast<const char*>(&hdr), sizeof(OMNIHeader));
    return ofs ? static_cast<int>(OFSErrorCodes::SUCCESS): static_cast<int>(OFSErrorCodes::ERROR_IO_ERROR);
}

static int read_header_from_file(const char* path, OMNIHeader& out_hdr)
{
    ifstream ifs(path, ios::binary);
    if (!ifs) return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
    ifs.read(reinterpret_cast<char*>(&out_hdr), sizeof(OMNIHeader));
    return ifs ? static_cast<int>(OFSErrorCodes::SUCCESS) : static_cast<int>(OFSErrorCodes::ERROR_IO_ERROR);
}

int fs_format(const char* omni_path, const char* config_path)
{
    if (!omni_path || !config_path)
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_CONFIG);

    OMNIHeader hdr = {};
    strncpy(hdr.magic, "OMNIFS01", sizeof(hdr.magic) - 1);
    hdr.format_version = 0x00010000;
    hdr.total_size = DEFAULT_TOTAL_SIZE;
    hdr.header_size = sizeof(OMNIHeader);
    hdr.block_size = DEFAULT_BLOCK_SIZE;
    hdr.config_timestamp = static_cast<uint64_t>(time(nullptr));
    hdr.user_table_offset = hdr.header_size;
    hdr.max_users = 1024;

    return write_header_to_file(omni_path, hdr);
}

int fs_init(void** instance, const char* omni_path, const char* config_path)
{
    if (!instance || !omni_path || !config_path)
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_CONFIG);

    OMNIHeader hdr;
    int r = read_header_from_file(omni_path, hdr);
    if (r != static_cast<int>(OFSErrorCodes::SUCCESS))
    {
        int f = fs_format(omni_path, config_path);
        if (f != static_cast<int>(OFSErrorCodes::SUCCESS))
            return f;
        r = read_header_from_file(omni_path, hdr);
        if (r != static_cast<int>(OFSErrorCodes::SUCCESS))
            return r;
    }

    auto* fs = new FileSystemInstance();
    fs->omni_path = omni_path;
    fs->config_path = config_path;

    unsigned long long block_size = hdr.block_size ? hdr.block_size : DEFAULT_BLOCK_SIZE;
    unsigned long long total_size = hdr.total_size ? hdr.total_size : DEFAULT_TOTAL_SIZE;
    unsigned long long total_blocks = total_size / block_size;
    if (total_blocks == 0) total_blocks = 1024ULL;

    fs->bitmap.init(static_cast<unsigned int>(total_blocks));

    fs->stats.total_size = total_size;
    fs->stats.used_space = 0;
    fs->stats.free_space = total_size;
    fs->stats.total_files = 0;
    fs->stats.total_directories = 1;
    fs->stats.total_users = 0;
    fs->stats.active_sessions = 0;
    fs->stats.fragmentation = 0.0;

    UserInfo admin("root", "root", UserRole::ADMIN, static_cast<uint64_t>(time(nullptr)));
    fs->users.push_back(admin);

    g_fs = fs;
    *instance = fs;

    cout << "[fs_init] Filesystem initialized successfully.\n";
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

void fs_shutdown(void* instance)
{
    auto* fs = reinterpret_cast<FileSystemInstance*>(instance);
    if (!fs) return;

    for (auto* s : fs->sessions)
        delete s;
    fs->sessions.clear();

    if (g_fs == fs)
        g_fs = nullptr;

    delete fs;
    cout << "[fs_shutdown] Filesystem shutdown complete.\n";
}
