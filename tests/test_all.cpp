#include <iostream>
#include <string>
#include <vector>
#include "../source/include/fs_core.hpp"
#include "../source/include/fs_user.hpp"
#include "../source/include/fs_file.hpp"
#include "../source/include/odf_types.hpp"

using namespace std;

int main() {
    cout << "===== OMNI FILE SYSTEM TEST =====" << endl;

    void* fs_instance = nullptr;

    cout << "\n[1] Formatting File System..." << endl;
    int fmt = fs_format("omni_data.omni", "config.cfg");
    cout << "fs_format returned: " << fmt << endl;

    cout << "\n[2] Initializing File System..." << endl;
    int init_code = fs_init(&fs_instance, "omni_data.omni", "config.cfg");
    cout << "fs_init returned: " << init_code << endl;
    if (!fs_instance) {
        cout << "❌ Failed to initialize file system!" << endl;
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

    cout << "\n[7] Create a File..." << endl;
    int create_file = file_create(alice_session, "/hello.txt", "Hello, OmniFS!", 16);
    cout << "file_create returned: " << create_file << endl;

    cout << "\n[8] Read the File..." << endl;
    char* buffer = nullptr;
    size_t size = 0;
    int read_code = file_read(alice_session, "/hello.txt", &buffer, &size);
    cout << "file_read returned: " << read_code << " | Size = " << size << endl;
    if (buffer) {
        cout << "Content: " << buffer << endl;
        delete[] buffer;
    }

    cout << "\n[9] Edit the File..." << endl;
    int edit_code = file_edit(alice_session, "/hello.txt", "Updated Content", 15, 0);
    cout << "file_edit returned: " << edit_code << endl;

    cout << "\n[10] Rename the File..." << endl;
    int rename_code = file_rename(alice_session, "/hello.txt", "/greetings.txt");
    cout << "file_rename returned: " << rename_code << endl;

    cout << "\n[11] Check if File Exists..." << endl;
    int exists_code = file_exists(alice_session, "/greetings.txt");
    cout << "file_exists returned: " << exists_code << endl;

    cout << "\n[12] Truncate the File..." << endl;
    int truncate_code = file_truncate(alice_session, "/greetings.txt");
    cout << "file_truncate returned: " << truncate_code << endl;

    cout << "\n[13] Delete the File..." << endl;
    int delete_code = file_delete(alice_session, "/greetings.txt");
    cout << "file_delete returned: " << delete_code << endl;

    cout << "\n[14] Logout Alice..." << endl;
    int logout_alice = user_logout(alice_session);
    cout << "user_logout (alice): " << logout_alice << endl;

    cout << "\n[15] Logout Admin..." << endl;
    int logout_admin = user_logout(admin_session);
    cout << "user_logout (admin): " << logout_admin << endl;

    cout << "\n[16] Shutdown File System..." << endl;
    fs_shutdown(fs_instance);
    cout << "fs_shutdown complete." << endl;

    cout << "\n===== ✅ ALL TESTS COMPLETE =====" << endl;
    return 0;
}

