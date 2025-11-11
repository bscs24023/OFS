#include "../include/free_bitmap.hpp"
#include <cstring>
using namespace std;

FreeBitmap::FreeBitmap() : bitmap_(), blocks_(0) {}

FreeBitmap::FreeBitmap(unsigned int total_blocks) 
{
    init(total_blocks);
}

void FreeBitmap::init(unsigned int total_blocks) 
{
    blocks_ = total_blocks;
    unsigned int bytes = (total_blocks + 7) / 8;
    bitmap_.assign(bytes, 0);
}

void FreeBitmap::setBit(vector<unsigned char>& bm, unsigned int pos) 
{
    if (pos / 8 < bm.size())
    {
        bm[pos / 8] |= (1u << (pos % 8));
    }
}

void FreeBitmap::clearBit(vector<unsigned char>& bm, unsigned int pos) 
{
    if (pos / 8 < bm.size())
    {
        bm[pos / 8] &= static_cast<unsigned char>(~(1u << (pos % 8)));
    }
}

bool FreeBitmap::testBit(const vector<unsigned char>& bm, unsigned int pos) 
{
    if (pos / 8 >= bm.size()) 
    {
        return false;
    }
    return (bm[pos / 8] & (1u << (pos % 8))) != 0;
}

pair<OFSErrorCodes, vector<unsigned int>> FreeBitmap::allocateBlocks(unsigned int n) 
{
    if (n == 0) 
    {
        return {OFSErrorCodes::ERROR_INVALID_OPERATION, {}};
    }

    vector<unsigned int> allocated;
    allocated.reserve(n);

    for (unsigned int i = 0; i < blocks_ && allocated.size() < n; ++i) 
    {
        if (!testBit(bitmap_, i)) 
        {
            setBit(bitmap_, i);
            allocated.push_back(i);
        }
    }

    if (allocated.size() < n) 
    {
        for (unsigned int b : allocated)
        {
            clearBit(bitmap_, b);
        }

        return {OFSErrorCodes::ERROR_NO_SPACE, {}};
    }

    return {OFSErrorCodes::SUCCESS, allocated};
}

OFSErrorCodes FreeBitmap::freeBlocks(const vector<unsigned int>& blocks) 
{
    for (unsigned int b : blocks) 
    {
        if (b < blocks_) 
        {
            clearBit(bitmap_, b);
        }
    }
    return OFSErrorCodes::SUCCESS;
}

unsigned int FreeBitmap::freeCount() const 
{
    unsigned int count = 0;
    for (unsigned int i = 0; i < blocks_; ++i) 
    {
        if (!testBit(bitmap_, i))
        {
            ++count;
        }
    }
    return count;
}

unsigned int FreeBitmap::totalBlocks() const 
{
    return blocks_;
}
