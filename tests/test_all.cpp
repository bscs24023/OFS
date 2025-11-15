#include <iostream>
#include <string>
#include <vector>
#include "../source/include/fs_core.hpp"
#include "../source/include/fs_user.hpp"
#include "../source/include/fs_file.hpp"
#include "../source/include/fs_dir.hpp"
#include "../source/include/fs_info.hpp"
#include "../source/include/odf_types.hpp"

using namespace std;

int main() 
{
    cout << "===== OMNI FILE SYSTEM TEST =====" << endl;

    void* fs_instance = nullptr;
    cout << "\n[1] Formatting File System..." << endl;
    int fmt = fs_format("omni_data.omni", "config.cfg");
    cout << "fs_format returned: " << fmt << endl;

    cout << "\n[2] Initializing File System..." << endl;
    int init_code = fs_init(&fs_instance, "omni_data.omni", "config.cfg");
    cout << "fs_init returned: " << init_code << endl;
    if (!fs_instance) {
        cout << "Failed to initialize file system!" << endl;
        return -1;
    }

    cout << "\n[3] Admin Login..." << endl;
    void* admin_session = nullptr;
    int login_code = user_login(&admin_session, "root", "root");
    cout << "user_login returned: " << login_code << endl;

    cout << "\n[4] Create Users..." << endl;
    int add1 = user_create(admin_session, "alice", "hash123", UserRole::NORMAL);
    int add2 = user_create(admin_session, "bob", "pass456", UserRole::NORMAL);
    cout << "Create alice: " << add1 << endl;
    cout << "Create bob:   " << add2 << endl;

    cout << "\n[5] List Users..." << endl;
    UserInfo* users = nullptr;
    int count = 0;
    int list_code = user_list(admin_session, &users, &count);
    cout << "user_list returned: " << list_code << " | Count = " << count << endl;
    for (int i = 0; i < count; ++i) {
        cout << " - " << users[i].username << " (role=" << (int)users[i].role << ")" << endl;
    }
    delete[] users;

    cout << "\n[6] Login as Alice..." << endl;
    void* alice_session = nullptr;
    int alice_login = user_login(&alice_session, "alice", "hash123");
    cout << "user_login (alice): " << alice_login << endl;

    cout << "\n[7] Create Directories..." << endl;
    int dir1 = dir_create(alice_session, "/docs");
    int dir2 = dir_create(alice_session, "/docs/reports");
    cout << "dir_create /docs: " << dir1 << endl;
    cout << "dir_create /docs/reports: " << dir2 << endl;

    cout << "\n[8] Directory Exists..." << endl;
    int exists1 = dir_exists(alice_session, "/docs");
    int exists2 = dir_exists(alice_session, "/unknown");
    cout << "/docs exists: " << exists1 << endl;
    cout << "/unknown exists: " << exists2 << endl;

    cout << "\n[9] Create Files in /docs..." << endl;
    int f1 = file_create(alice_session, "/docs/readme.txt", "Documentation", 13);
    int f2 = file_create(alice_session, "/docs/reports/summary.txt", "Report Summary", 15);
    cout << "file_create readme.txt: " << f1 << endl;
    cout << "file_create summary.txt: " << f2 << endl;



    cout << "\n[10] List /docs Directory..." << endl;
    FileEntry* entries = nullptr;
    int entry_count = 0;
    int list_dir_code = dir_list(alice_session, "/docs", &entries, &entry_count);
    cout << "dir_list returned: " << list_dir_code << " | Count = " << entry_count << endl;
    for (int i = 0; i < entry_count; ++i) {
        cout << " - " << entries[i].name << " (type=" << (int)entries[i].type << ")" << endl;
    }
    if (entries) {
        free(entries);
    }

    cout << "\n[11] Get Metadata for /docs/readme.txt..." << endl;
    FileMetadata meta;
    int meta_code = get_metadata(alice_session, "/docs/readme.txt", &meta);
    cout << "get_metadata returned: " << meta_code << endl;
    if (meta_code == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << " Name: " << meta.entry.name << endl;
        cout << " Size (logical): " << meta.entry.size << endl;
        cout << " Actual size (on disk): " << meta.actual_size << endl;
        cout << " Owner: " << meta.entry.owner << endl;
    }

    cout << "\n[12] Change File Permissions..." << endl;
    int perm_code = set_permissions(alice_session, "/docs/readme.txt", 0644);
    cout << "set_permissions returned: " << perm_code << endl;

    cout << "\n[13] Get File System Stats..." << endl;
    FSStats stats;
    int stats_code = get_stats(alice_session, &stats);
    cout << "get_stats returned: " << stats_code << endl;
    if (stats_code == static_cast<int>(OFSErrorCodes::SUCCESS)) {
        cout << " Total Files: " << stats.total_files << endl;
        cout << " Total Size: " << stats.total_size << endl;
        cout << " Free Space: " << stats.free_space << endl;
    }

    cout << "\n[14] Delete Directory /docs/reports..." << endl;
    int del_dir1 = dir_delete(alice_session, "/docs/reports");
    cout << "dir_delete returned: " << del_dir1 << endl;

    cout << "\n[15] Delete Directory /docs..." << endl;
    int del_dir2 = dir_delete(alice_session, "/docs");
    cout << "dir_delete returned: " << del_dir2 << endl;

    cout << "\n[16] Error Message Example..." << endl;
    const char* msg = get_error_message(static_cast<int>(OFSErrorCodes::ERROR_INVALID_CONFIG));
    cout << "Error -11: " << (msg ? msg : "(null)") << endl;

    cout << "\n[17] Logout Alice..." << endl;
    int logout_alice = user_logout(alice_session);
    cout << "user_logout (alice): " << logout_alice << endl;

    cout << "\n[18] Logout Admin..." << endl;
    int logout_admin = user_logout(admin_session);
    cout << "user_logout (admin): " << logout_admin << endl;

    cout << "\n[19] Shutdown File System..." << endl;
    fs_shutdown(fs_instance);
    cout << "fs_shutdown complete." << endl;

    cout << "Test complete" << endl;
    return 0;
}
