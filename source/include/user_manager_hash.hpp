#ifndef USER_MANAGER_HASH_HPP
#define USER_MANAGER_HASH_HPP

#include "odf_types.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <algorithm>
#include <ctime>

using namespace std;

class UserManagerHash 
{
public:
    explicit UserManagerHash(uint32_t max_users = 1024);
    ~UserManagerHash();

    OFSErrorCodes addUser(const UserInfo& user);
    OFSErrorCodes removeUser(const string& username);
    optional<UserInfo> findUser(const string& username) const;
    OFSErrorCodes loginUser(const string& username, const string& password_hash);

    vector<UserInfo> listUsersSorted() const;
    void clear();
    uint32_t userCount() const;

private:
    unordered_map<string, UserInfo> map_;
    uint32_t max_users_;
};

#endif
