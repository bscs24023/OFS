#ifndef FREE_BITMAP_HPP
#define FREE_BITMAP_HPP

#include "odf_types.hpp"
#include <vector>
#include <utility>

using namespace std;

class FreeBitmap 
{
public:
    FreeBitmap();
    explicit FreeBitmap(unsigned int total_blocks);

    void init(unsigned int total_blocks);

    pair<OFSErrorCodes, vector<unsigned int>> allocateBlocks(unsigned int n);

    OFSErrorCodes freeBlocks(const vector<unsigned int>& blocks);

    unsigned int freeCount() const;

    unsigned int totalBlocks() const;

private:
    vector<unsigned char> bitmap_;
    unsigned int blocks_; 

    static void setBit(vector<unsigned char>& bm, unsigned int pos);
    static void clearBit(vector<unsigned char>& bm, unsigned int pos);
    static bool testBit(const vector<unsigned char>& bm, unsigned int pos);
};

#endif
