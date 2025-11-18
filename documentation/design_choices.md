User Indexing
Data structure used:
vector<UserInfo> user_list;

Reasoning:

The system is expected to have very few users (e.g., root + a few created users).

O(n) linear search is fast for n < 100.

Vectors serialize cleanly into the .uconf file.

Simple structure avoids complication

Operations using this structure:

user_login searches linearly for username.

user_delete erases by index.

user_list returns a copy of the vector.

Because the user count is relativly small, a vector is optimal, simple, and efficient.


Directory Tree Representation
Data structure used:
Custom Tree
struct Node {
    FileEntry entry;
    Node* parent;
    vector<Node*> children;
};

Reasoning:

Direct representation of hierarchical filesystems

Natural mapping of directory → children entries

Easy operations:

directory creation

listing children

recursive delete

moving through parents



Free Space Tracking Using a Bitmap

The free-space manager uses a bitmap:

0 = free

1 = allocated

Reasoning:

Used in real filesystems (EXT, NTFS)

Very memory-efficient

Fast to scan for free blocks

Easy to serialize into the .omni file

Can handle thousands of blocks with minimal overhead


Mapping File Paths to Disk Locations

OFS uses two  structures:

1) PathIndex

Stores: vector<pair<string, FileMetadata>>

FileMetadata includes:

owner

permissions

file size

block start index

block count

entry type

Mapping:
PathIndex[path] → FileMetadata

2) DirTree

Maintains hierarchy and ensures directories exist.

Combined:

User requests /docs/readme.txt

PathIndex finds its metadata

Metadata gives start block & size

Bitmap identifies block usage

Raw blocks yield file content

.omni File Structure

The OFS disk file is laid out as:
[ HEADER ]
[ BITMAP ]
[ USER TABLE ]
[ PATH INDEX ]
[ DIRECTORY TREE ]
[ RAW FILE DATA BLOCKS ]


REasongin for this layout: 

Easy to serialize all components

Allows deterministic loading on startup

Avoids fragmentation complications

Metadata and data remain cleanly separated

Header contains:

magic signature

filesystem version

number of blocks

sizes of metadata sections


Memory Management Strategies include: 

1. Clear ownership

Each component cleans up its own memory:

DirTree deletes nodes recursively

PathIndex clears vector

Session objects destroyed on logout

2. No smart pointers

Avoids circular references in the tree
Raw pointers give control.

3. File read buffers

Allocated only when needed:

file_read allocates buffer

caller frees using free_buffer()


