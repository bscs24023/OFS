#ifndef USER_MANAGER_HASH_HPP
#define USER_MANAGER_HASH_HPP

#include <iostream>
#include <string>
#include <vector>
#include "odf_types.hpp"

using namespace std;

class UserManagerHash 
{
private:
    struct HashNode {
        string key;
        UserInfo* value;
        HashNode* next;
        HashNode(const string& k, UserInfo* v) : key(k), value(v), next(nullptr) {}
    };

    vector<HashNode*> table;
    size_t capacity;
    uint32_t max_users;
    size_t size;

    size_t hash(const string& key) const;

public:
    explicit UserManagerHash(uint32_t max_users = 1024);
    ~UserManagerHash();

    OFSErrorCodes addUser(const UserInfo& user);
    OFSErrorCodes removeUser(const string& username);
    UserInfo* findUser(const string& username) const;
    OFSErrorCodes loginUser(const string& username, const string& password_hash);
    uint32_t userCount() const;
};

#endif

