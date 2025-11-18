
#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#include "../include/fs_core.hpp"
#include "../include/fs_user.hpp"
#include "../include/fs_dir.hpp"
#include "../include/fs_file.hpp"
#include "../include/fs_info.hpp"
#include "../include/odf_types.hpp"

using namespace std;

#define PORT 8080
#define BACKLOG 10
#define BUF_SIZE 8192

static string trim(const string &s)
{
    size_t a = 0;
    while (a < s.size() && isspace((unsigned char)s[a])) ++a;
    size_t b = s.size();
    while (b > a && isspace((unsigned char)s[b-1])) --b;
    return s.substr(a, b - a);
}

static void send_msg(int sock, const string &msg)
{
    ssize_t n = send(sock, msg.c_str(), msg.size(), 0);
    (void)n;
}

static bool recv_line(int sock, string &out)
{
    out.clear();
    char buf[BUF_SIZE];
    string acc;
    while (true)
    {
        ssize_t n = recv(sock, buf, sizeof(buf) - 1, 0);
        if (n <= 0)
        {
            return false;
        }
        buf[n] = '\0';
        acc.append(buf, n);
        size_t pos = acc.find('\n');
        if (pos != string::npos)
        {
            string line = acc.substr(0, pos);
            out = trim(line);
            return true;
        }
        if (acc.size() > 256 * 1024)
        {
            out = trim(acc);
            return true;
        }
    }
    return false;
}

static vector<string> tokenize(const string &line)
{
    vector<string> toks;
    istringstream ss(line);
    string t;
    while (ss >> t)
    {
        toks.push_back(t);
    }
    return toks;
}

static string rc_to_msg(int code)
{
    const char* m = get_error_message(code);
    if (m) return string(m);
    return string("ERROR_") + to_string(code);
}

static void handle_client(int client_sock)
{
    send_msg(client_sock, "Welcome to OFS server\n");
    void* session = nullptr;

    string line;
    while (true)
    {
        bool ok = recv_line(client_sock, line);
        if (!ok) break;
        if (line.empty()) continue;

        vector<string> args = tokenize(line);
        if (args.empty()) { send_msg(client_sock, "ERR INVALID_COMMAND\n"); continue; }

        string cmd = args[0];
        for (char &c : cmd) c = toupper((unsigned char)c);

        if (cmd == "LOGIN")
        {
            if (args.size() < 3) { send_msg(client_sock, "ERR USAGE: LOGIN <user> <pass>\n"); continue; }
            string user = args[1];
            string pass = args[2];
            void* new_s = nullptr;
            int rc = user_login(&new_s, user.c_str(), pass.c_str());
            if (rc != 0) { send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n"); continue; }
            session = new_s;
            send_msg(client_sock, "OK SESSION\n");
            continue;
        }

        if (cmd == "LOGOUT")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); break; }
            int rc = user_logout(session);
            session = nullptr;
            if (rc != 0) send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n");
            else send_msg(client_sock, "OK\n");
            break;
        }

        if (cmd == "CREATE_USER")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); continue; }
            if (args.size() < 4) { send_msg(client_sock, "ERR USAGE: CREATE_USER <username> <password> <role>\n"); continue; }
            string uname = args[1];
            string pwd = args[2];
            int role = stoi(args[3]);
            int rc = user_create(session, uname.c_str(), pwd.c_str(), static_cast<UserRole>(role));
            if (rc != 0) send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n"); else send_msg(client_sock, "OK\n");
            continue;
        }

        if (cmd == "DELETE_USER")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); continue; }
            if (args.size() < 2) { send_msg(client_sock, "ERR USAGE: DELETE_USER <username>\n"); continue; }
            int rc = user_delete(session, args[1].c_str());
            if (rc != 0) send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n"); else send_msg(client_sock, "OK\n");
            continue;
        }

        if (cmd == "LIST_USERS")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); continue; }
            UserInfo* list = nullptr;
            int count = 0;
            int rc = user_list(session, &list, &count);
            if (rc != 0) { send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n"); continue; }
            send_msg(client_sock, string("OK ") + to_string(count) + "\n");
            for (int i = 0; i < count; ++i)
            {
                send_msg(client_sock, string(list[i].username) + " role=" + to_string(static_cast<int>(list[i].role)) + "\n");
            }
            delete[] list;
            continue;
        }

        if (cmd == "GET_SESSION_INFO")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); continue; }
            SessionInfo info;
            int rc = get_session_info(session, &info);
            if (rc != 0) { send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n"); continue; }
            send_msg(client_sock, string("OK user=") + info.user.username + " role=" + to_string(static_cast<int>(info.user.role)) + "\n");
            continue;
        }

        if (cmd == "DIR_CREATE" || cmd == "MKDIR")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); continue; }
            if (args.size() < 2) { send_msg(client_sock, "ERR USAGE: DIR_CREATE <path>\n"); continue; }
            string path = args[1];
            int rc = dir_create(session, path.c_str());
            if (rc != 0) send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n"); else send_msg(client_sock, "OK\n");
            continue;
        }

        if (cmd == "DIR_LIST" || cmd == "LS")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); continue; }
            if (args.size() < 2) { send_msg(client_sock, "ERR USAGE: DIR_LIST <path>\n"); continue; }
            string path = args[1];
            FileEntry* entries = nullptr;
            int cnt = 0;
            int rc = dir_list(session, path.c_str(), &entries, &cnt);
            if (rc != 0) { send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n"); continue; }
            send_msg(client_sock, string("OK ") + to_string(cnt) + "\n");
            for (int i = 0; i < cnt; ++i)
            {
                send_msg(client_sock, string(entries[i].name) + " " + to_string((int)entries[i].type) + "\n");
            }
            delete[] entries;
            continue;
        }

        if (cmd == "DIR_DELETE" || cmd == "RMDIR")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); continue; }
            if (args.size() < 2) { send_msg(client_sock, "ERR USAGE: DIR_DELETE <path>\n"); continue; }
            int rc = dir_delete(session, args[1].c_str());
            if (rc != 0) send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n"); else send_msg(client_sock, "OK\n");
            continue;
        }

        if (cmd == "DIR_EXISTS" || cmd == "EXISTS_DIR")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); continue; }
            if (args.size() < 2) { send_msg(client_sock, "ERR USAGE: DIR_EXISTS <path>\n"); continue; }
            int rc = dir_exists(session, args[1].c_str());
            if (rc != 0) send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n"); else send_msg(client_sock, "OK\n");
            continue;
        }

        if (cmd == "FILE_CREATE" || cmd == "CREATE")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); continue; }
            if (args.size() < 2) { send_msg(client_sock, "ERR USAGE: CREATE <path>\n"); continue; }
            string path = args[1];
            send_msg(client_sock, "SEND_DATA <<<EOF>>> on its own line to finish\n");
            string data;
            while (true)
            {
                string l;
                if (!recv_line(client_sock, l)) { break; }
                if (l == "<<<EOF>>>") break;
                data += l;
                data.push_back('\n');
            }
            int rc = file_create(session, path.c_str(), data.c_str(), data.size());
            if (rc != 0) send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n"); else send_msg(client_sock, "OK\n");
            continue;
        }

        if (cmd == "FILE_READ" || cmd == "READ")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); continue; }
            if (args.size() < 2) { send_msg(client_sock, "ERR USAGE: READ <path>\n"); continue; }
            string path = args[1];
            char* buf = nullptr;
            size_t size = 0;
            int rc = file_read(session, path.c_str(), &buf, &size);
            if (rc != 0) { send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n"); continue; }
            send_msg(client_sock, string("OK ") + to_string(size) + "\n");
            if (buf && size > 0) send_msg(client_sock, string(buf, size) + "\n");
            free_buffer(buf);
            continue;
        }

        if (cmd == "FILE_EDIT" || cmd == "EDIT")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); continue; }
            if (args.size() < 3) { send_msg(client_sock, "ERR USAGE: EDIT <path> <index>\n"); continue; }
            string path = args[1];
            unsigned int index = static_cast<unsigned int>(stoul(args[2]));
            send_msg(client_sock, "SEND_DATA <<<EOF>>> on its own line to finish\n");
            string data;
            while (true)
            {
                string l;
                if (!recv_line(client_sock, l)) { break; }
                if (l == "<<<EOF>>>") break;
                data += l;
                data.push_back('\n');
            }
            int rc = file_edit(session, path.c_str(), data.c_str(), data.size(), index);
            if (rc != 0) send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n"); else send_msg(client_sock, "OK\n");
            continue;
        }

        if (cmd == "FILE_DELETE" || cmd == "DELETE")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); continue; }
            if (args.size() < 2) { send_msg(client_sock, "ERR USAGE: DELETE <path>\n"); continue; }
            int rc = file_delete(session, args[1].c_str());
            if (rc != 0) send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n"); else send_msg(client_sock, "OK\n");
            continue;
        }

        if (cmd == "FILE_TRUNCATE" || cmd == "TRUNCATE")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); continue; }
            if (args.size() < 2) { send_msg(client_sock, "ERR USAGE: TRUNCATE <path>\n"); continue; }
            int rc = file_truncate(session, args[1].c_str());
            if (rc != 0) send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n"); else send_msg(client_sock, "OK\n");
            continue;
        }

        if (cmd == "FILE_EXISTS" || cmd == "EXISTS_FILE")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); continue; }
            if (args.size() < 2) { send_msg(client_sock, "ERR USAGE: FILE_EXISTS <path>\n"); continue; }
            int rc = file_exists(session, args[1].c_str());
            if (rc != 0) send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n"); else send_msg(client_sock, "OK\n");
            continue;
        }

        if (cmd == "FILE_RENAME" || cmd == "RENAME")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); continue; }
            if (args.size() < 3) { send_msg(client_sock, "ERR USAGE: RENAME <old> <new>\n"); continue; }
            int rc = file_rename(session, args[1].c_str(), args[2].c_str());
            if (rc != 0) send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n"); else send_msg(client_sock, "OK\n");
            continue;
        }

        if (cmd == "GET_METADATA")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); continue; }
            if (args.size() < 2) { send_msg(client_sock, "ERR USAGE: GET_METADATA <path>\n"); continue; }
            FileMetadata m;
            int rc = get_metadata(session, args[1].c_str(), &m);
            if (rc != 0) { send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n"); continue; }
            send_msg(client_sock, string("OK name=") + m.entry.name + " size=" + to_string(m.entry.size) + " owner=" + m.entry.owner + "\n");
            continue;
        }

        if (cmd == "SET_PERMISSIONS")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); continue; }
            if (args.size() < 3) { send_msg(client_sock, "ERR USAGE: SET_PERMISSIONS <path> <perm>\n"); continue; }
            uint32_t perms = static_cast<uint32_t>(stoul(args[2]));
            int rc = set_permissions(session, args[1].c_str(), perms);
            if (rc != 0) send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n"); else send_msg(client_sock, "OK\n");
            continue;
        }

        if (cmd == "GET_STATS")
        {
            if (!session) { send_msg(client_sock, "ERR NOT_LOGGED_IN\n"); continue; }
            FSStats st;
            int rc = get_stats(session, &st);
            if (rc != 0) send_msg(client_sock, string("ERR ") + rc_to_msg(rc) + "\n");
            else send_msg(client_sock, string("OK files=") + to_string(st.total_files) + " used=" + to_string(st.used_space) + " free=" + to_string(st.free_space) + "\n");
            continue;
        }

        send_msg(client_sock, "ERR UNKNOWN_COMMAND\n");
    }

    if (session)
    {
        user_logout(session);
        session = nullptr;
    }
    close(client_sock);
}

int main()
{
    void* fsinst = nullptr;
    int rc = fs_init(&fsinst, "file.omni", "default.uconf.txt");
    if (rc != 0)
    {
        fs_format("file.omni", "default.uconf.txt");
        rc = fs_init(&fsinst, "file.omni", "default.uconf.txt");
        if (rc != 0)
        {
            cerr << "FS initialization failed: " << rc << "\n";
            return 1;
        }
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }
    if (listen(server_fd, BACKLOG) < 0) { perror("listen"); return 1; }

    cout << "OFS server listening on port " << PORT << endl;

    while (true)
    {
        int client = accept(server_fd, nullptr, nullptr);
        if (client < 0) { perror("accept"); continue; }
        thread t(handle_client, client);
        t.detach();
    }

    fs_shutdown(fsinst);
    return 0;
}
