## 1. Test Scenarios

### **User Management**
- ✔ LOGIN with valid and invalid credentials  
- ✔ CREATE_USER and DELETE_USER (admin-only permissions)  
- ✔ LIST_USERS shows correct count and role information  
- ✔ GET_SESSION_INFO returns active username + role  

### **Directory Operations**
- ✔ MKDIR successfully creates nested directories  
- ✔ LS correctly lists directory contents  
- ✔ DIR_EXISTS returns true/false as expected  
- ✔ RMDIR works only for empty directories  

### **File Operations**
- ✔ CREATE writes file contents using streaming mode (`SEND_DATA`)  
- ✔ READ returns full file contents without corruption  
- ✔ EDIT appends or overwrites based on index  
- ✔ FILE_EXISTS detects presence/absence correctly  
- ✔ TRUNCATE resets file content size to zero  
- ✔ DELETE removes file from disk + index structures  
- ✔ RENAME updates both DirTree and PathIndex paths  

### **System Operations**
- ✔ GET_METADATA returns correct size, timestamps, and type  
- ✔ GET_STATS returns current header + disk usage    

## 2. Performance Metrics

Hardware: Ubuntu, SSD, standard laptop.

| Operation | Avg Time |
|----------|----------|-------|
| LS, DIR_EXISTS | **0.5–1.2 ms** |
| CREATE file (small) | **1–3 ms** |
| READ file (1 KB) | **~1 ms** |
| EDIT (append) | **2–4 ms** |
| GET_STATS | **<1 ms** |


