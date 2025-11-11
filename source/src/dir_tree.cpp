#include "../include/dir_tree.hpp"

using namespace std;

DirTree::DirTree() 
{
    FileEntry rootEntry("/", EntryType::DIRECTORY, 0, static_cast<uint32_t>(FilePermissions::OWNER_READ), "root", 0);
    root_ = new Node(rootEntry);
}

DirTree::~DirTree() 
{
    deleteSubtree(root_);
}

void DirTree::deleteSubtree(Node* node) 
{
    if (!node) return;
    for (auto child : node->children) 
    {
        deleteSubtree(child);
    }
    delete node;
}

void DirTree::clear() {
    deleteSubtree(root_);
    FileEntry rootEntry("/", EntryType::DIRECTORY, 0, static_cast<uint32_t>(FilePermissions::OWNER_READ), "root", 0);
    root_ = new Node(rootEntry);
}

vector<string> DirTree::splitPath(const string& path) {
    vector<string> parts;
    string temp;
    for (char c : path) 
    {
        if (c == '/') 
        {
            if (!temp.empty()) 
            {
                parts.push_back(temp);
                temp.clear();
            }
        } 
        else 
        {
            temp.push_back(c);
        }
    }
    if (!temp.empty()) 
    {
        parts.push_back(temp);
    }
    return parts;
}

DirTree::Node* DirTree::findNode(const string& path) const 
{
    if (path == "/" || path.empty()) 
    {
        return root_;
    }
    vector<string> parts = splitPath(path);
    Node* cur = root_;
    for (auto& part : parts) 
    {
        bool found = false;
        for (auto child : cur->children) 
        {
            if (child->entry.name == part) 
            {
                cur = child;
                found = true;
                break;
            }
        }
        if (!found) 
        {
            return nullptr;
        }
    }
    return cur;
}

DirTree::Node* DirTree::createPathNodes(const vector<string>& components, bool createLast, const FileEntry* entry) {
    Node* cur = root_;
    for (size_t i = 0; i < components.size(); ++i) 
    {
        const string& part = components[i];
        Node* found = nullptr;
        for (auto child : cur->children) 
        {
            if (child->entry.name == part) 
            {
                found = child;
                break;
            }
        }

        if (!found) 
        {
            if (i == components.size() - 1 && !createLast) 
            {
                return nullptr;
            }
            FileEntry fe = (i == components.size() - 1 && entry)? *entry : FileEntry(part, EntryType::DIRECTORY, 0, 0755, "root", 0);
            Node* newNode = new Node(fe);
            newNode->parent = cur;
            cur->children.push_back(newNode);
            cur = newNode;
        } 
        else 
        {
            cur = found;
        }
    }
    return cur;
}

OFSErrorCodes DirTree::createEntry(const string& path, const FileEntry& entry) 
{
    if (path.empty() || path[0] != '/') 
    {
        return OFSErrorCodes::ERROR_INVALID_PATH;
    }
    vector<string> parts = splitPath(path);
    if (parts.empty()) 
    {
        return OFSErrorCodes::ERROR_INVALID_OPERATION;
    }

    vector<string> parent_parts(parts.begin(), parts.end() - 1);
    Node* parent = root_;
    if (!parent_parts.empty()) 
    {
        parent = createPathNodes(parent_parts, false, nullptr);
        if (!parent) 
        {
            return OFSErrorCodes::ERROR_INVALID_PATH;
        }
        if (parent->entry.getType() != EntryType::DIRECTORY)
        {
            return OFSErrorCodes::ERROR_INVALID_OPERATION;
        }
    }

    string name = parts.back();
    for (auto child : parent->children) 
    {
        if (child->entry.name == name)
        {
            return OFSErrorCodes::ERROR_FILE_EXISTS;
        }
    }

    Node* newNode = new Node(entry);
    newNode->parent = parent;
    parent->children.push_back(newNode);
    return OFSErrorCodes::SUCCESS;
}

OFSErrorCodes DirTree::removeEntry(const string& path) 
{
    if (path.empty() || path == "/") 
    {
        return OFSErrorCodes::ERROR_INVALID_OPERATION;
    }
    Node* node = findNode(path);
    if (!node) 
    {
        return OFSErrorCodes::ERROR_NOT_FOUND;
    }
    if (!node->children.empty()) 
    {
        return OFSErrorCodes::ERROR_DIRECTORY_NOT_EMPTY;
    }

    Node* parent = node->parent;
    if (!parent) 
    {
        return OFSErrorCodes::ERROR_INVALID_OPERATION;
    }

    for (auto it = parent->children.begin(); it != parent->children.end(); ++it) 
    {
        if (*it == node) 
        {
            parent->children.erase(it);
            delete node;
            return OFSErrorCodes::SUCCESS;
        }
    }
    return OFSErrorCodes::ERROR_NOT_FOUND;
}

FileEntry* DirTree::findEntry(const string& path) const 
{
    Node* node = findNode(path);
    return node ? &node->entry : nullptr;
}

OFSErrorCodes DirTree::listDirectory(const string& dirpath, vector<FileEntry>& out) const 
{
    Node* n = findNode(dirpath);
    if (!n) return OFSErrorCodes::ERROR_NOT_FOUND;
    if (n->entry.getType() != EntryType::DIRECTORY)
    {
        return OFSErrorCodes::ERROR_INVALID_OPERATION;
    }

    for (auto child : n->children)
    {
        out.push_back(child->entry);
    }
    return OFSErrorCodes::SUCCESS;
}

void DirTree::printTree() const 
{
    vector<pair<Node*, int>> stack = {{root_, 0}};
    while (!stack.empty()) 
    {
        auto [node, level] = stack.back();
        stack.pop_back();

        for (int i = 0; i < level; ++i) cout << "  ";
        cout << node->entry.name;
        if (node->entry.getType() == EntryType::DIRECTORY)
        {
            cout << "/";
        }
        cout << "\n";

        for (auto it = node->children.rbegin(); it != node->children.rend(); ++it) 
        {
            stack.push_back({*it, level + 1});
        }
    }
}
