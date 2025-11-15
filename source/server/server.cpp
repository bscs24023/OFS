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

using namespace std;

#define PORT 8080
#define MAX_CONN 10
#define BUFFER_SIZE 8192

static string trim(const string &s) {
    size_t a = 0;
    while (a < s.size() && isspace((unsigned char)s[a])) ++a;
    size_t b = s.size();
    while (b > a && isspace((unsigned char)s[b-1])) --b;
    return s.substr(a, b - a);
}

static void send_msg(int sock, const string& msg) {
    ssize_t n = send(sock, msg.c_str(), msg.size(), 0);
    (void)n;
}

static bool recv_line(int sock, string &out) {
    out.clear();
    char buf[BUFFER_SIZE];
    while (true) {
        ssize_t n = recv(sock, buf, sizeof(buf)-1, 0);
        if (n <= 0) { return false; }
        buf[n] = '\0';
        out.append(buf, n);
        size_t pos;
        if ((pos = out.find('\n')) != string::npos) {
            string line = out.substr(0, pos);
            string remain = (pos + 1 < out.size()) ? out.substr(pos + 1) : string();
            out = trim(line);
            (void)remain;
            return true;
        }
        if (out.size() > 256*1024) { 
            out = trim(out);
            return true;
        }
    }
    return false;
}

static vector<string> split_tokens(const string &line) 
{

    vector<string> toks;
    istringstream ss(line);
    string token;
    while (ss >> token) {
        toks.push_back(token);
    }
    return toks;
}

static void* session_from_string(const string &s) {
    if (s.empty() || s == "null" || s == "0") return nullptr;
    try {
        unsigned long long v = stoull(s);
        return reinterpret_cast<void*>(static_cast<uintptr_t>(v));
    } catch (...) {
        return nullptr;
    }
}

static string ok() { return string("OK\n"); }
static string err_msg(const char* m) { return string("ERR ") + m + "\n"; }
static string err_msg(const string& m) { return string("ERR ") + m + "\n"; }

static void handle_client(int client_sock) {
    string line;
    send_msg(client_sock, "Welcome to OFS server\n");

    void* current_session = nullptr;

    while (true) {
        bool ok_read = recv_line(client_sock, line);
        if (!ok_read) break;
        line = trim(line);
        if (line.empty()) continue;

        vector<string> toks = split_tokens(line);
        if (toks.empty()) { send_msg(client_sock, err_msg("INVALID_COMMAND")); continue; }

        string cmd = toks[0];
        for (auto &c: cmd) c = toupper((unsigned char)c);

        if (cmd == "LOGIN") {
            if (toks.size() < 3) { send_msg(client_sock, err_msg("USAGE: LOGIN <user> <pass>")); continue; }
            string user = toks[1];
            string pass = toks[2];
            void* sess = nullptr;
            int rc = user_login(&sess, user.c_str(), pass.c_str());
            if (rc != 0) {
                const char* em = get_error_message(rc);
                send_msg(client_sock, err_msg(em ? em : "LOGIN_FAILED"));
                continue;
            }
            current_session = sess;
            uintptr_t p = reinterpret_cast<uintptr_t>(sess);
            send_msg(client_sock, string("OK SESSION ") + to_string(p) + "\n");
            continue;
        }

        if (cmd == "LOGOUT") {
            string sessstr = (toks.size() >= 2 ? toks[1] : string());
            void* s = session_from_string(sessstr);
            int rc = user_logout(s);
            if (current_session == s) current_session = nullptr;
            if (rc != 0) {
                const char* em = get_error_message(rc);
                send_msg(client_sock, err_msg(em ? em : "LOGOUT_FAILED"));
            } else send_msg(client_sock, ok());
            break;
        }

        if (cmd == "MKDIR") {
            if (toks.size() < 3) { send_msg(client_sock, err_msg("USAGE: MKDIR <session> <path>")); continue; }
            void* s = session_from_string(toks[1]);
            string path = toks[2];
            int rc = dir_create(s, path.c_str());
            if (rc != 0) { send_msg(client_sock, err_msg(get_error_message(rc))); } else send_msg(client_sock, ok());
            continue;
        }

        if (cmd == "RMDIR") {
            if (toks.size() < 3) { send_msg(client_sock, err_msg("USAGE: RMDIR <session> <path>")); continue; }
            void* s = session_from_string(toks[1]);
            string path = toks[2];
            int rc = dir_delete(s, path.c_str());
            if (rc != 0) { send_msg(client_sock, err_msg(get_error_message(rc))); } else send_msg(client_sock, ok());
            continue;
        }

        if (cmd == "LS") {
            if (toks.size() < 3) { send_msg(client_sock, err_msg("USAGE: LS <session> <path>")); continue; }
            void* s = session_from_string(toks[1]);
            string path = toks[2];
            FileEntry* entries = nullptr;
            int count = 0;
            int rc = dir_list(s, path.c_str(), &entries, &count);
            if (rc != 0) { send_msg(client_sock, err_msg(get_error_message(rc))); } else {
                send_msg(client_sock, string("OK ") + to_string(count) + "\n");
                for (int i = 0; i < count; ++i) {
                    string t = string(entries[i].name) + " " + to_string((int)entries[i].type) + "\n";
                    send_msg(client_sock, t);
                }
                delete[] entries;
            }
            continue;
        }

        if (cmd == "CREATE") {
            if (toks.size() < 3) { send_msg(client_sock, err_msg("USAGE: CREATE <session> <path>")); continue; }
            void* s = session_from_string(toks[1]);
            string path = toks[2];
            send_msg(client_sock, "SEND_DATA <<<EOF>>> on its own line to finish\n");
            string data;
            while (true) {
                string dataline;
                bool ok = recv_line(client_sock, dataline);
                if (!ok) { break; }
                if (dataline == "<<<EOF>>>") break;
                data += dataline;
                data.push_back('\n');
            }
            int rc = file_create(s, path.c_str(), data.c_str(), data.size());
            if (rc != 0) send_msg(client_sock, err_msg(get_error_message(rc))); else send_msg(client_sock, ok());
            continue;
        }

        if (cmd == "READ") {
            if (toks.size() < 3) { send_msg(client_sock, err_msg("USAGE: READ <session> <path>")); continue; }
            void* s = session_from_string(toks[1]);
            string path = toks[2];
            char* buffer = nullptr;
            size_t size = 0;
            int rc = file_read(s, path.c_str(), &buffer, &size);
            if (rc != 0) { send_msg(client_sock, err_msg(get_error_message(rc))); } else {
                send_msg(client_sock, string("OK ") + to_string(size) + "\n");
                if (buffer && size > 0) {
                    send_msg(client_sock, string(buffer, size) + "\n");
                }
                free_buffer(buffer);
            }
            continue;
        }

        if (cmd == "DELETE") {
            if (toks.size() < 3) { send_msg(client_sock, err_msg("USAGE: DELETE <session> <path>")); continue; }
            void* s = session_from_string(toks[1]);
            string path = toks[2];
            int rc = file_delete(s, path.c_str());
            if (rc != 0) send_msg(client_sock, err_msg(get_error_message(rc))); else send_msg(client_sock, ok());
            continue;
        }

        if (cmd == "STATS") {
            if (toks.size() < 2) { send_msg(client_sock, err_msg("USAGE: STATS <session>")); continue; }
            void* s = session_from_string(toks[1]);
            FSStats st;
            int rc = get_stats(s, &st);
            if (rc != 0) send_msg(client_sock, err_msg(get_error_message(rc)));
            else {
                string out = "OK FILES=" + to_string(st.total_files)
                    + " USED=" + to_string(st.used_space)
                    + " FREE=" + to_string(st.free_space) + "\n";
                send_msg(client_sock, out);
            }
            continue;
        }

        if (cmd == "CREATE_USER") {
            if (toks.size() < 5) { send_msg(client_sock, err_msg("USAGE: CREATE_USER <admin_session> <username> <password> <role>")); continue; }
            void* s = session_from_string(toks[1]);
            string username = toks[2];
            string password = toks[3];
            int role = stoi(toks[4]);
            int rc = user_create(s, username.c_str(), password.c_str(), static_cast<UserRole>(role));
            if (rc != 0) send_msg(client_sock, err_msg(get_error_message(rc))); else send_msg(client_sock, ok());
            continue;
        }

        send_msg(client_sock, err_msg("UNKNOWN_COMMAND"));
    }

    if (current_session) {
        user_logout(current_session);
        current_session = nullptr;
    }
    close(client_sock);
}

int main() {
    void* fsinst = nullptr;
    if (fs_init(&fsinst, "file.omni", "default.uconf.txt") != 0) {
        fs_format("file.omni", "default.uconf.txt");
        if (fs_init(&fsinst, "file.omni", "default.uconf.txt") != 0) {
            cerr << "FS init failed\n";
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
    if (listen(server_fd, MAX_CONN) < 0) { perror("listen"); return 1; }

    cout << "OFS server listening on port " << PORT << endl;

    while (true) {
        int client = accept(server_fd, nullptr, nullptr);
        if (client < 0) { perror("accept"); continue; }
        thread(handle_client, client).detach();
    }

    fs_shutdown(fsinst);
    return 0;
}
