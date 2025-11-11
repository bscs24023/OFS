#include "../include/user_manager_hash.hpp"
#include <cstring>

using namespace std;

UserManagerHash::UserManagerHash(uint32_t max_users): map_(), max_users_(max_users) 
{
    map_.reserve(max_users_);
}

UserManagerHash::~UserManagerHash() = default;

OFSErrorCodes UserManagerHash::addUser(const UserInfo& user) 
{
    string uname(user.username);
    if (map_.size() >= max_users_) 
    {
        return OFSErrorCodes::ERROR_NO_SPACE;
    }
    if (map_.find(uname) != map_.end()) 
    {
        return OFSErrorCodes::ERROR_FILE_EXISTS;
    }
    map_.emplace(uname, user);
    return OFSErrorCodes::SUCCESS;
}

OFSErrorCodes UserManagerHash::removeUser(const string& username) 
{
    auto it = map_.find(username);
    if (it == map_.end()) 
    {
        return OFSErrorCodes::ERROR_NOT_FOUND;
    }
    map_.erase(it);
    return OFSErrorCodes::SUCCESS;
}

optional<UserInfo> UserManagerHash::findUser(const string& username) const 
{
    auto it = map_.find(username);
    if (it == map_.end()) 
    {
        return nullopt;
    }
    return it->second;
}

OFSErrorCodes UserManagerHash::loginUser(const string& username, const string& password_hash) 
{
    auto it = map_.find(username);
    if (it == map_.end()) 
    {
        return OFSErrorCodes::ERROR_NOT_FOUND;
    }
    const UserInfo& ui = it->second;
    if (!ui.is_active) 
    {
        return OFSErrorCodes::ERROR_INVALID_SESSION;
    }
    if (std::strncmp(ui.password_hash, password_hash.c_str(), sizeof(ui.password_hash)) == 0) 
    {
        UserInfo updated = it->second;
        updated.last_login = static_cast<uint64_t>(std::time(nullptr));
        it->second = updated;
        return OFSErrorCodes::SUCCESS;
    }
    return OFSErrorCodes::ERROR_PERMISSION_DENIED;
}

vector<UserInfo> UserManagerHash::listUsersSorted() const 
{
    vector<UserInfo> v;
    v.reserve(map_.size());
    for (const auto &p : map_) 
    {
        v.push_back(p.second);
    }
    sort(v.begin(), v.end(), [](const UserInfo& a, const UserInfo& b) 
    {
        return std::strncmp(a.username, b.username, sizeof(a.username)) < 0;
    });
    return v;
}

void UserManagerHash::clear() 
{
    map_.clear();
}

uint32_t UserManagerHash::userCount() const 
{
    return static_cast<uint32_t>(map_.size());
}
