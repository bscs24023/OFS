#ifndef PATH_INDEX_HPP
#define PATH_INDEX_HPP

#include "odf_types.hpp"
#include <string>
#include <vector>
#include <iostream>

using namespace std;

class PathIndex 
{
public:
    PathIndex();
    ~PathIndex();

    OFSErrorCodes insert(const string& path, const FileMetadata& meta);
    OFSErrorCodes remove(const string& path);
    FileMetadata* find(const string& path);
    const FileMetadata* find(const string& path) const;
    vector<string> listPaths() const;
    void clear();

private:
    vector<pair<string, FileMetadata>> index_;
};

#endif
