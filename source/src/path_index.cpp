#include "../include/path_index.hpp"

using namespace std;

PathIndex::PathIndex() 
{
    index_.reserve(1024);
}

PathIndex::~PathIndex() = default;

OFSErrorCodes PathIndex::insert(const string& path, const FileMetadata& meta) 
{
    for (auto& p : index_) 
    {
        if (p.first == path) 
        {
            return OFSErrorCodes::ERROR_FILE_EXISTS;
        }
    }
    index_.push_back({path, meta});
    return OFSErrorCodes::SUCCESS;
}

OFSErrorCodes PathIndex::remove(const string& path) 
{
    for (auto it = index_.begin(); it != index_.end(); ++it) 
    {
        if (it->first == path) 
        {
            index_.erase(it);
            return OFSErrorCodes::SUCCESS;
        }
    }
    return OFSErrorCodes::ERROR_NOT_FOUND;
}

FileMetadata* PathIndex::find(const string& path) 
{
    for (auto& p : index_) 
    {
        if (p.first == path) 
        {
            return &p.second;
        }
    }
    return nullptr;
}

const FileMetadata* PathIndex::find(const string& path) const 
{
    for (const auto& p : index_) 
    {
        if (p.first == path) 
        {
            return &p.second;
        }
    }
    return nullptr;
}

vector<string> PathIndex::listPaths() const 
{
    vector<string> out;
    out.reserve(index_.size());
    for (const auto& p : index_) 
    {
        out.push_back(p.first);
    }
    return out;
}

void PathIndex::clear() 
{
    index_.clear();
}
