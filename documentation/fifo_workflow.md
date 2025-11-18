
## 1. Operation Queue Design
The server maintains a `std::queue<Operation>` where each operation contains:
- Parsed command (e.g., MKDIR, CREATE, EDIT),
- Path/parameters,
- The client socket to respond to.

Requests are always inserted in the order received, preserving strict FIFO behavior.

## 2. Thread / Process Management
The main thread only accepts TCP connections and reads commands.  
Each parsed command is pushed into the queue under a mutex lock.  
A dedicated worker thread continuously pops operations and executes them, ensuring only one filesystem operation runs at a time, preventing corruption inside the `.omni` file.


## 3. Request Queuing Workflow
When a command is received, it is parsed into an internal Operation struct and enqueued.  
The worker thread wakes up, dequeues the oldest request, and executes the corresponding internal function (e.g., `createFile()`, `readFile()`, `mkdir()`).  
Because all disk-modifying operations are serialized, directory tree updates, metadata writes, and data block updates remain consistent.

## 4. Response Handling
After executing an operation, the worker thread formats a response.  
This response is written back to the same socket associated with the enqueued request.  
If the client disconnects, the server silently drops the result rather than crashing.

## 5. Benefits in This Implementation
- Prevents race conditions between operations that modify the `.omni` file.  
- Guarantees consistent updates to the `DirTree`, `PathIndex`, user table, and free-space map.  
- Avoids partial writes and overlapping edits by ensuring exactly one operation touches disk at any given moment.  
- Simplifies the design while still supporting multiple connected clients safely.
