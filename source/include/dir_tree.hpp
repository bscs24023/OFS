#ifndef DIR_TREE_HPP
#define DIR_TREE_HPP

#include "odf_types.hpp"
#include <string>
#include <vector>
#include <iostream>

using namespace std;

class DirTree 
{
private:
    struct Node 
    {
        FileEntry entry;
        Node* parent;
        vector<Node*> children;
        Node(const FileEntry& e) : entry(e), parent(nullptr) {}
    };

    Node* root_;

    Node* findNode(const string& path) const;
    Node* createPathNodes(const vector<string>& components, bool createLast, const FileEntry* entry);
    static vector<string> splitPath(const string& path);
    void deleteSubtree(Node* node);
public:
    DirTree();
    ~DirTree();

    OFSErrorCodes createEntry(const string& path, const FileEntry& entry);
    OFSErrorCodes removeEntry(const string& path);
    FileEntry* findEntry(const string& path) const;
    OFSErrorCodes listDirectory(const string& dirpath, vector<FileEntry>& out) const;
    void printTree() const;
    void clear();
};

#endif
