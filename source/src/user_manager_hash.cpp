#include "../include/user_manager_hash.hpp"
#include <ctime>

using namespace std;

size_t UserManagerHash::hash(const string& key) const 
{
    size_t h = 0;
    for (char c : key) 
    {
        h = (h * 31 + c) % capacity;
    }
    return h;
}

UserManagerHash::UserManagerHash(uint32_t max_users): capacity(101), max_users(max_users), size(0) 
{
    table.resize(capacity, nullptr);
}

UserManagerHash::~UserManagerHash() 
{
    for (size_t i = 0; i < capacity; ++i) 
    {
        HashNode* node = table[i];
        while (node) 
        {
            HashNode* tmp = node;
            node = node->next;
            delete tmp->value;
            delete tmp;
        }
    }
}

OFSErrorCodes UserManagerHash::addUser(const UserInfo& user) 
{
    if (size >= max_users) 
    {
        return OFSErrorCodes::ERROR_NO_SPACE;
    }
    string uname = user.username;
    size_t idx = hash(uname);
    HashNode* node = table[idx];
    while (node) 
    {
        if (node->key == uname) 
        {
            return OFSErrorCodes::ERROR_FILE_EXISTS;
        }
        node = node->next;
    }
    UserInfo* u = new UserInfo(user);
    HashNode* new_node = new HashNode(uname, u);
    new_node->next = table[idx];
    table[idx] = new_node;
    ++size;
    return OFSErrorCodes::SUCCESS;
}

OFSErrorCodes UserManagerHash::removeUser(const string& username) 
{
    size_t idx = hash(username);
    HashNode* node = table[idx];
    HashNode* prev = nullptr;
    while (node) 
    {
        if (node->key == username) 
        {
            if (prev) prev->next = node->next;
            else table[idx] = node->next;
            delete node->value;
            delete node;
            --size;
            return OFSErrorCodes::SUCCESS;
        }
        prev = node;
        node = node->next;
    }
    return OFSErrorCodes::ERROR_NOT_FOUND;
}

UserInfo* UserManagerHash::findUser(const string& username) const 
{
    size_t idx = hash(username);
    HashNode* node = table[idx];
    while (node) 
    {
        if (node->key == username) 
        {
            return node->value;
        }
        node = node->next;
    }
    return nullptr;
}

OFSErrorCodes UserManagerHash::loginUser(const string& username, const string& password_hash) 
{
    UserInfo* u = findUser(username);
    if (!u) 
    {
        return OFSErrorCodes::ERROR_NOT_FOUND;
    }
    if (!u->is_active) 
    {
        return OFSErrorCodes::ERROR_INVALID_SESSION;
    }
    if (string(u->password_hash) == password_hash) 
    {
        u->last_login = static_cast<uint64_t>(time(nullptr));
        return OFSErrorCodes::SUCCESS;
    }
    return OFSErrorCodes::ERROR_PERMISSION_DENIED;
}

uint32_t UserManagerHash::userCount() const 
{
    return static_cast<uint32_t>(size);
}
