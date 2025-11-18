import socket
import tkinter as tk
from tkinter import ttk, simpledialog, scrolledtext


SERVER_HOST = "127.0.0.1"
SERVER_PORT = 8080


class OFSClient:
    def __init__(self):
        self.sock = None

    def connect(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((SERVER_HOST, SERVER_PORT))
        welcome = self.sock.recv(4096).decode()
        return welcome.strip()

    def send(self, cmd):
        if not self.sock:
            return "ERROR: Not connected"

        try:
            self.sock.sendall((cmd + "\n").encode())
            return self.sock.recv(8192).decode().strip()
        except Exception as e:
            return f"ERROR: {e}"

    def send_stream(self, cmd, content):
        if not self.sock:
            return "ERROR: Not connected"

        self.sock.sendall((cmd + "\n").encode())
        reply = self.sock.recv(4096).decode()
        if not reply.startswith("SEND_DATA"):
            return reply.strip()

        for line in content.split("\n"):
            self.sock.sendall((line + "\n").encode())

        self.sock.sendall("<<<EOF>>>\n".encode())
        return self.sock.recv(4096).decode().strip()

    def close(self):
        if self.sock:
            self.sock.close()



class OFSGUI:
    def __init__(self, root):
        self.client = OFSClient()
        self.root = root
        self.root.title("OFS GUI Console")
        self.root.geometry("1000x650")
        self.root.configure(bg="#1e1e1e")

        style = ttk.Style()
        style.theme_use("clam")

        style.configure("TButton", font=("Segoe UI", 11), padding=6)
        style.configure("TLabel", background="#1e1e1e", foreground="white")

        self.build_ui()
        try:
            welcome = self.client.connect()
            self.log(welcome)
        except:
            self.log("ERROR: Could not connect to server.")

    def build_ui(self):

        sidebar_container = tk.Frame(self.root, bg="#2b2b2b")
        sidebar_container.pack(side="left", fill="y")

        canvas = tk.Canvas(sidebar_container, bg="#2b2b2b", highlightthickness=0)
        canvas.pack(side="left", fill="y", expand=True)

        scrollbar = ttk.Scrollbar(sidebar_container, orient="vertical", command=canvas.yview)
        scrollbar.pack(side="right", fill="y")

        canvas.configure(yscrollcommand=scrollbar.set)

        self.sidebar = tk.Frame(canvas, bg="#2b2b2b")
        canvas.create_window((0, 0), window=self.sidebar, anchor="nw")

        def update_scroll_region(event):
            canvas.configure(scrollregion=canvas.bbox("all"))

        self.sidebar.bind("<Configure>", update_scroll_region)

        self.output = scrolledtext.ScrolledText(
            self.root, bg="#111", fg="#0f0", font=("Consolas", 12)
        )
        self.output.pack(fill="both", expand=True)

        self.add_section("User Commands", [
            ("Login", self.ui_login),
            ("Logout", lambda: self.send_cmd("LOGOUT")),
            ("Create User", self.ui_create_user),
            ("Delete User", self.ui_delete_user),
            ("List Users", lambda: self.send_cmd("LIST_USERS")),
            ("Session Info", lambda: self.send_cmd("GET_SESSION_INFO")),
        ])

        self.add_section("Directory", [
            ("MKDIR", self.ui_mkdir),
            ("LS", self.ui_ls),
            ("RMDIR", self.ui_rmdir),
            ("DIR_EXISTS", self.ui_dir_exists),
        ])

        self.add_section("Files", [
            ("Create File", self.ui_create_file),
            ("Read File", self.ui_read_file),
            ("Edit File", self.ui_edit_file),
            ("Delete File", self.ui_delete_file),
            ("Rename File", self.ui_rename_file),
            ("File Exists", self.ui_file_exists),
            ("Truncate", self.ui_truncate_file),
            ("Get Metadata", self.ui_metadata),
        ])

        self.add_section("System", [
            ("Get Stats", lambda: self.send_cmd("GET_STATS")),
            ("Exit Client", self.exit_client),
        ])

    def add_section(self, title, buttons):
        lbl = tk.Label(self.sidebar, text=title, bg="#2b2b2b",
                       fg="cyan", font=("Segoe UI", 12, "bold"))
        lbl.pack(pady=(15, 5))

        for text, cmd in buttons:
            btn = ttk.Button(self.sidebar, text=text, command=cmd)
            btn.pack(fill="x", padx=10, pady=2)

    def log(self, msg):
        self.output.insert("end", msg + "\n")
        self.output.see("end")

    def send_cmd(self, cmd):
        resp = self.client.send(cmd)
        self.log(f"> {cmd}\n{resp}")


    def ui_login(self):
        u = simpledialog.askstring("Login", "Username")
        p = simpledialog.askstring("Login", "Password", show="*")
        if u and p:
            self.send_cmd(f"LOGIN {u} {p}")

    def ui_create_user(self):
        u = simpledialog.askstring("Create User", "Username")
        p = simpledialog.askstring("Create User", "Password")
        r = simpledialog.askstring("Create User", "Role (0=USER,1=ADMIN)")
        if u and p and r:
            self.send_cmd(f"CREATE_USER {u} {p} {r}")

    def ui_delete_user(self):
        u = simpledialog.askstring("Delete User", "Username")
        if u:
            self.send_cmd(f"DELETE_USER {u}")

    def ui_mkdir(self):
        path = simpledialog.askstring("MKDIR", "Directory path")
        if path:
            self.send_cmd(f"MKDIR {path}")

    def ui_ls(self):
        path = simpledialog.askstring("LS", "Directory path")
        if path:
            self.send_cmd(f"LS {path}")

    def ui_rmdir(self):
        path = simpledialog.askstring("RMDIR", "Directory path")
        if path:
            self.send_cmd(f"RMDIR {path}")

    def ui_dir_exists(self):
        path = simpledialog.askstring("DIR_EXISTS", "Directory path")
        if path:
            self.send_cmd(f"DIR_EXISTS {path}")


    def ui_create_file(self):
        path = simpledialog.askstring("Create File", "File path")
        content = simpledialog.askstring("File Content", "Enter content")
        if path and content is not None:
            resp = self.client.send_stream(f"CREATE {path}", content)
            self.log(resp)

    def ui_read_file(self):
        path = simpledialog.askstring("Read File", "File path")
        if path:
            self.send_cmd(f"READ {path}")

    def ui_edit_file(self):
        path = simpledialog.askstring("Edit File", "File path")
        index = simpledialog.askstring("Edit File", "Start index")
        content = simpledialog.askstring("Edit File", "New content")
        if path and index and content:
            resp = self.client.send_stream(f"EDIT {path} {index}", content)
            self.log(resp)

    def ui_delete_file(self):
        path = simpledialog.askstring("Delete File", "File path")
        if path:
            self.send_cmd(f"DELETE {path}")

    def ui_rename_file(self):
        old = simpledialog.askstring("Rename", "Old path")
        new = simpledialog.askstring("Rename", "New path")
        if old and new:
            self.send_cmd(f"RENAME {old} {new}")

    def ui_file_exists(self):
        path = simpledialog.askstring("FILE_EXISTS", "File path")
        if path:
            self.send_cmd(f"FILE_EXISTS {path}")

    def ui_truncate_file(self):
        path = simpledialog.askstring("Truncate", "File path")
        if path:
            self.send_cmd(f"TRUNCATE {path}")

    def ui_metadata(self):
        path = simpledialog.askstring("Metadata", "File path")
        if path:
            self.send_cmd(f"GET_METADATA {path}")

    def exit_client(self):
        self.client.close()
        self.root.quit()



if __name__ == "__main__":
    root = tk.Tk()
    OFSGUI(root)
    root.mainloop()
