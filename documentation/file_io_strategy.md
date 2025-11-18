
## 1. Reading and Writing the `.omni` File
The `.omni` file is accessed using low-level C I/O (`fseek`, `fread`, `fwrite`) to maintain full control over byte-level layout.  
Each region (header, users, bitmap, directory tree, data blocks) has fixed offsets, allowing predictable seeking.  
Reads and writes always target exact positions calculated from the filesystem metadata which ultimately prevents fragmentation.  

## 2. Serialization / Deserialization

### OMNIHeader
The header is a simple POD structure, so it is written and read in a single block operation.  
No dynamic memory or variable-length fields ensures perfect binary compatibility across sessions.  
When loading the filesystem, the header is read first to reconstruct size, block count, and indexing offsets.  

### UserInfo
UserInfo objects are stored consecutively in a fixed-size region, enabling O(1) access by index.  
Serialization consists of writing the entire struct as binary since no pointers are involved.  
On startup, all UserInfo entries are loaded into RAM to allow fast user lookup and authentication.  
Changes (create/delete user) immediately overwrite the modified UserInfo range in the .omni file.

### FileEntry
FileEntry is also POD-based and written in a fixed metadata segment to avoid variable offsets.  
Each entry corresponds to a path in the directory tree, and its location is derived by index.  
On load, all FileEntry values populate the DirTree structure, which rebuilds the directory hierarchy in memory.  


## 3. Buffering Strategies
Small metadata structures use minimal stack buffers for efficient parsing without large overhead.  
File content is loaded in full into a heap buffer when accessed, this gives simple and predictable behavior.  
Writes to large files are done in chunks to avoid copying excessively large buffers in memory.  
Only necessary blocks are read during file operations, keeping memory usage manageable.

## 4. Handling File Growth
When a file grows, additional blocks are allocated by scanning the free-space bitmap for the next available slot.  
If a file expands beyond its previous block count, data is appended to newly allocated blocks without moving old data.  
If the .omni file itself needs to grow, it is extended at the end with zero-initialized blocks.  
Shrinking a file releases extra blocks back to the bitmap, updating the used and free counters accordingly.

## 5. Free Space Management
A bitmap array tracks free and used blocks using 1 bit per block, providing compact and fast allocation.  
Block allocation just finds the first 0-bit by scanning the bitmap and sets it to 1.  
Deallocation resets the corresponding bit, making the block reusable for future file writes.  
The bitmap itself is stored at a fixed region in the .omni file and loaded entirely into memory at startup.

## 6. Data Integrity Approach
Critical metadata (header, bitmap, FileEntry table) is written synchronously and flushed immediately to avoid corruption.  
File content writes are also flushed block-by-block which ensures partial writes cannot damage unrelated data.  
The system never writes outside predefined offsets, preventing accidental overwrites of metadata or neighboring blocks.  
On startup, the entire filesystem is validated to confirm that header, bitmap, and directory structures are consistent.

## 7. Memory Residency vs Disk Access
The header, user table, free-space bitmap, and directory tree permanently reside in RAM after initialization.  
File content is only read from disk when the user accesses a file, reducing memory footprint.  
Editing a file reloads only that file’s blocks, not the entire data region.
