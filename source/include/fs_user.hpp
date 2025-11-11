#ifndef FS_USER_HPP
#define FS_USER_HPP

#include "../include/omni_fs.hpp"
#include "../include/odf_types.hpp"
#include <vector>
#include <string>

using namespace std;

int user_login(void** session, const char* username, const char* password);

int user_logout(void* session);

int user_create(void* admin_session, const char* username, const char* password, UserRole role);
int user_delete(void* admin_session, const char* username);

int user_list(void* admin_session, UserInfo** users, int* count);

int get_session_info(void* session, SessionInfo* info);

#endif
