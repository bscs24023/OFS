#ifndef OMNI_FS_HPP
#define OMNI_FS_HPP

#include "odf_types.hpp"
#include "user_manager_hash.hpp"
#include "dir_tree.hpp"
#include "path_index.hpp"
#include "free_bitmap.hpp"
#include <vector>
#include <string>

using namespace std;

struct OmniFS {
    string omni_path;
    string config_path;

    UserManagerHash users;
    DirTree dir_tree;
    PathIndex path_index;
    FreeBitmap bitmap;

    FSStats stats;

    vector<SessionInfo*> sessions;

    OmniFS()
        : omni_path()
        , config_path()
        , users(1024)
        , dir_tree()
        , path_index()
        , bitmap()
        , stats()
        , sessions()
    {}
};

extern OmniFS* g_omni_fs;

#endif
