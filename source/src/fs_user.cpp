#include "../include/fs_user.hpp"
#include "../include/fs_core.hpp"
#include "../include/user_manager_hash.hpp"

#include <iostream>
#include <cstring>
#include <ctime>

using namespace std;

extern FileSystemInstance* g_fs;

static bool is_admin_session(void* admin_session)
{
    if (!admin_session) {
        return false;
    }
    SessionInfo* s = reinterpret_cast<SessionInfo*>(admin_session);
    return (s->user.role == UserRole::ADMIN);
}

int user_login(void** session, const char* username, const char* password)
{
    if (!session || !username || !password) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    if (!g_fs) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_CONFIG);
    }

    UserInfo* u = nullptr;
    for (size_t i = 0; i < g_fs->users.size(); ++i) {
        if (std::strncmp(g_fs->users[i].username, username, sizeof(g_fs->users[i].username)) == 0) {
            u = &g_fs->users[i];
            break;
        }
    }
    if (!u) {
        return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
    }
    if (!u->is_active) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_SESSION);
    }
    if (std::strncmp(u->password_hash, password, sizeof(u->password_hash)) != 0) {
        return static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED);
    }

    u->last_login = static_cast<uint64_t>(time(nullptr));

    SessionInfo* s = new SessionInfo();
    std::memset(s, 0, sizeof(*s));
    std::strncpy(s->session_id, u->username, sizeof(s->session_id) - 1);
    s->user = *u;
    s->login_time = static_cast<uint64_t>(time(nullptr));
    s->last_activity = s->login_time;
    s->operations_count = 0;
    g_fs->sessions.push_back(s);
    *session = s;

    g_fs->stats.active_sessions = static_cast<uint32_t>(g_fs->sessions.size());

    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int user_logout(void* session)
{
    if (!session) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    if (!g_fs) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_CONFIG);
    }

    SessionInfo* s = reinterpret_cast<SessionInfo*>(session);
    for (size_t i = 0; i < g_fs->sessions.size(); ++i) {
        if (g_fs->sessions[i] == s) {
            delete g_fs->sessions[i];
            g_fs->sessions.erase(g_fs->sessions.begin() + i);
            g_fs->stats.active_sessions = static_cast<uint32_t>(g_fs->sessions.size());
            return static_cast<int>(OFSErrorCodes::SUCCESS);
        }
    }

    return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
}

int user_create(void* admin_session, const char* username, const char* password, UserRole role)
{
    if (!admin_session || !username || !password) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    if (!g_fs) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_CONFIG);
    }
    if (!is_admin_session(admin_session)) {
        return static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED);
    }

    for (size_t i = 0; i < g_fs->users.size(); ++i) {
        if (std::strncmp(g_fs->users[i].username, username, sizeof(g_fs->users[i].username)) == 0) {
            return static_cast<int>(OFSErrorCodes::ERROR_FILE_EXISTS);
        }
    }

    UserInfo u(std::string(username), std::string(password), role, static_cast<uint64_t>(time(nullptr)));
    g_fs->users.push_back(u);
    g_fs->stats.total_users = static_cast<uint32_t>(g_fs->users.size());

    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int user_delete(void* admin_session, const char* username)
{
    if (!admin_session || !username) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    if (!g_fs) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_CONFIG);
    }
    if (!is_admin_session(admin_session)) {
        return static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED);
    }

    for (size_t i = 0; i < g_fs->users.size(); ++i) {
        if (std::strncmp(g_fs->users[i].username, username, sizeof(g_fs->users[i].username)) == 0) {
            g_fs->users.erase(g_fs->users.begin() + i);
            g_fs->stats.total_users = static_cast<uint32_t>(g_fs->users.size());
            return static_cast<int>(OFSErrorCodes::SUCCESS);
        }
    }

    return static_cast<int>(OFSErrorCodes::ERROR_NOT_FOUND);
}

int user_list(void* admin_session, UserInfo** users, int* count)
{
    if (!admin_session || !users || !count) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    if (!g_fs) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_CONFIG);
    }
    if (!is_admin_session(admin_session)) {
        return static_cast<int>(OFSErrorCodes::ERROR_PERMISSION_DENIED);
    }

    if (g_fs->users.empty()) {
        *users = nullptr;
        *count = 0;
        return static_cast<int>(OFSErrorCodes::SUCCESS);
    }

    *count = static_cast<int>(g_fs->users.size());
    UserInfo* arr = new UserInfo[*count];
    for (int i = 0; i < *count; ++i) {
        arr[i] = g_fs->users[i];
    }
    *users = arr;

    return static_cast<int>(OFSErrorCodes::SUCCESS);
}

int get_session_info(void* session, SessionInfo* info)
{
    if (!session || !info) {
        return static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION);
    }
    SessionInfo* s = reinterpret_cast<SessionInfo*>(session);
    *info = *s;
    return static_cast<int>(OFSErrorCodes::SUCCESS);
}
