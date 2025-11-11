#include <iostream>
#include <string>
#include <vector>
#include "../source/include/user_manager_hash.hpp"
#include "../source/include/dir_tree.hpp"
#include "../source/include/path_index.hpp"
#include "../source/include/free_bitmap.hpp"

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

    cout << "User count: " << um.userCount() << endl << endl;



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
    cout << endl;



    cout << "==== Testing PathIndex ====" << endl;
    PathIndex pathIndex;

    FileMetadata meta1("/docs/file1.txt", f2);
    FileMetadata meta2("/docs/file2.txt", f2);

    cout << "Insert /docs/file1.txt: " << (int)pathIndex.insert("/docs/file1.txt", meta1) << endl;
    cout << "Insert /docs/file2.txt: " << (int)pathIndex.insert("/docs/file2.txt", meta2) << endl;
    cout << "Insert duplicate (should fail): " << (int)pathIndex.insert("/docs/file1.txt", meta1) << endl;

    FileMetadata* found = pathIndex.find("/docs/file1.txt");
    if (found != nullptr)
        cout << "Found /docs/file1.txt owned by " << found->entry.owner << endl;
    else
        cout << "File not found" << endl;

    cout << "List all paths:" << endl;
    for (const auto &p : pathIndex.listPaths())
        cout << " - " << p << endl;

    cout << "Remove /docs/file2.txt: " << (int)pathIndex.remove("/docs/file2.txt") << endl;
    cout << "Find removed file (should be null): " << (pathIndex.find("/docs/file2.txt") == nullptr ? "nullptr" : "exists") << endl;
    cout << endl;



    cout << "==== Testing FreeBitmap ====" << endl;
    FreeBitmap bitmap;
    bitmap.init(16);

    cout << "Initially free: " << bitmap.freeCount() << "/" << bitmap.totalBlocks() << endl;

    auto alloc = bitmap.allocateBlocks(5);
    cout << "Allocate 5 blocks -> status: " << (int)alloc.first << endl;
    cout << "Allocated: ";
    for (auto b : alloc.second) cout << b << " ";
    cout << endl;

    cout << "Free count after alloc: " << bitmap.freeCount() << endl;

    bitmap.freeBlocks(alloc.second);
    cout << "Free count after freeing: " << bitmap.freeCount() << endl;

    cout << endl << "==== All tests completed successfully ====" << endl;

    return 0;
}
