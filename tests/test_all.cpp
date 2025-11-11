#include <iostream>
#include <string>
#include <vector>
#include "../source/include/user_manager_hash.hpp"
#include "../source/include/dir_tree.hpp"

using namespace std;

int main() {
    cout << "==== Testing UserManagerHash ====" << endl;
    UserManagerHash um(10);

    UserInfo alice("alice", "hash123", UserRole::NORMAL, 0);

    UserInfo bob("bob", "pass456", UserRole::NORMAL, 0);


    cout << "Add alice: " << (int)um.addUser(alice) << endl;
    cout << "Add bob:   " << (int)um.addUser(bob) << endl;
    cout << "Add alice again (should fail): " << (int)um.addUser(alice) << endl;

    cout << "Login alice (correct pw): " << (int)um.loginUser("alice", "hash123") << endl;
    cout << "Login alice (wrong pw):   " << (int)um.loginUser("alice", "bad") << endl;
    cout << "Login nonexist:           " << (int)um.loginUser("charlie", "123") << endl;

    cout << "User count: " << um.userCount() << endl;

    cout << "==== Testing DirTree ====" << endl;
    DirTree tree;

    FileEntry f1("docs", EntryType::DIRECTORY, 0, 0755, "alice", 0);
    FileEntry f2("file.txt", EntryType::FILE, 100, 0644, "alice", 0);

    cout << "Create /docs: " << (int)tree.createEntry("/docs", f1) << endl;
    cout << "Create /docs/file.txt: " << (int)tree.createEntry("/docs/file.txt", f2) << endl;

    vector<FileEntry> entries;
    cout << "List /docs: " << (int)tree.listDirectory("/docs", entries) << endl;
    for (auto &e : entries) {
        cout << " - " << e.name << endl;
    }

    cout << "Remove /docs/file.txt: " << (int)tree.removeEntry("/docs/file.txt") << endl;
    cout << "Remove /docs: " << (int)tree.removeEntry("/docs") << endl;

    cout << "Final tree:\n";
    tree.printTree();

    return 0;
}
