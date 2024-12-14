import sys
import tkinter as tk
from tkinter import Canvas, Scrollbar, Menu
from PIL import Image, ImageTk
import select
import os

buffer = ""
file_content_buffer = None
receiving_file = False

images_cache = {}
current_context_menu = None

def execute_command(command):
    if command:
        sys.stdout.write(command + '\n')
        sys.stdout.flush()

def display_files_and_dirs(canvas, files, dirs):
    canvas.delete("all")
    x, y = 10, 10
    spacing_x = 120
    spacing_y = 150
    max_width = 500

    def load_image(path):
        if path not in images_cache:
            image = Image.open(path).resize((100, 100))
            images_cache[path] = ImageTk.PhotoImage(image)
        return images_cache[path]

    def create_button(name, icon_path, command, x, y):
        icon_image = load_image(icon_path)
        button = tk.Button(canvas, image=icon_image, command=command, borderwidth=0)
        button.image = icon_image
        canvas.create_window(x + 50, y + 50, window=button)
        canvas.create_text(x + 50, y + 130, text=name, anchor="center", font=("Arial", 10))

        if name != ".." and icon_path == "Directory.png":
            button.bind("<Button-3>", lambda event, n=name: show_directory_context_menu(event, n))
        elif icon_path == "File.png":
            button.bind("<Button-3>", lambda event, n=name: show_file_context_menu(event, n))

    dirs.insert(0, "..")

    for name in dirs:
        command = lambda n=name: execute_command(f"cd {n}")
        create_button(name, "Directory.png", command, x, y)
        x += spacing_x
        if x + spacing_x > max_width:
            x = 10
            y += spacing_y

    for name in files:
        command = lambda n=name: execute_command(f"cat {n}")
        create_button(name, "File.png", command, x, y)
        x += spacing_x
        if x + spacing_x > max_width:
            x = 10
            y += spacing_y

    canvas.configure(scrollregion=(0, 0, max_width, y + spacing_y))

def update_current_directory(data):
    output_display.configure(state=tk.NORMAL)
    output_display.delete(1.0, tk.END)
    output_display.insert(tk.END, data)
    output_display.configure(state=tk.DISABLED)

def hide_context_menu(event=None):
    global current_context_menu
    if current_context_menu:
        current_context_menu.unpost()
        current_context_menu = None

def display_file_content(content):
    popup = tk.Toplevel(root)
    popup.title("파일 내용")
    popup.geometry("600x400")
    text_widget = tk.Text(popup, wrap=tk.WORD)
    text_widget.pack(expand=True, fill=tk.BOTH)
    text_widget.insert("1.0", content)
    text_widget.configure(state=tk.DISABLED)

def display_file_info(file_info):
    popup = tk.Toplevel(root)
    popup.title("파일 속성")
    popup.geometry("400x300")
    text_widget = tk.Text(popup, wrap=tk.WORD, state=tk.NORMAL, height=10)
    text_widget.pack(expand=True, fill=tk.BOTH, padx=10, pady=10)

    for key, value in file_info.items():
        text_widget.insert(tk.END, f"{key}: {value}\n")

    def change_permissions():
        change_popup = tk.Toplevel(root)
        change_popup.title("파일 권한 변경")
        change_popup.geometry("300x150")

        label = tk.Label(change_popup, text="새 권한을 입력하세요 (e.g., 755 or u+r):")
        label.pack(pady=5)
        entry = tk.Entry(change_popup, width=30)
        entry.pack(pady=5)

        def submit_permission():
            permission = entry.get().strip()
            if permission:
                execute_command(f"chmod {permission} {file_info['파일 이름']}")
            change_popup.destroy()
            popup.destroy()
            execute_command(f"watchFile {file_info['파일 이름']}")

        submit_button = tk.Button(change_popup, text="확인", command=submit_permission)
        submit_button.pack(pady=10)

    change_button = tk.Button(popup, text="파일 권한 바꾸기", command=change_permissions)
    change_button.pack(pady=10)

    text_widget.configure(state=tk.DISABLED)

def parse_file_info(message):
    try:
        parts = message.replace("[FileInfo] ", "").split(" ", 3)
        if len(parts) != 4:
            raise ValueError("Invalid FileInfo format.")
        file_info = {
            "파일 이름": parts[0],
            "파일 권한": parts[1],
            "크기": f"{parts[2]}",
            "파일 만든 시간": parts[3],
        }
        return file_info
    except Exception as e:
        print(f"Error parsing FileInfo: {e}", file=sys.stderr)
        return None

def receive_data():
    global buffer, file_content_buffer, receiving_file
    try:
        if select.select([sys.stdin], [], [], 0)[0]:
            chunk = sys.stdin.read()
            buffer += chunk

            while "\n" in buffer:
                message, buffer = buffer.split("\n", 1)
                message = message.strip()

                if receiving_file:
                    if message == "[END_OF_FILE]":
                        receiving_file = False
                        if file_content_buffer is not None:
                            display_file_content(file_content_buffer)
                            file_content_buffer = None
                    else:
                        if file_content_buffer is None:
                            file_content_buffer = ""
                        file_content_buffer += message + "\n"

                elif message == "[START_OF_FILE]":
                    receiving_file = True
                    file_content_buffer = ""

                elif message.startswith("[FileInfo]"):
                    file_info = parse_file_info(message)
                    if file_info:
                        display_file_info(file_info)

                elif message.startswith("[Files]:"):
                    files_part = ""
                    directories_part = ""
                    if "[Directories]:" in message:
                        parts = message.split("[Directories]:")
                        files_part = parts[0].replace("[Files]:", "").strip()
                        directories_part = parts[1].strip()
                    files = files_part.split() if files_part else []
                    dirs = directories_part.split() if directories_part else []
                    display_files_and_dirs(file_canvas, files, dirs)

                elif message.startswith("/"):
                    update_current_directory(message)

                else:
                    output_display.configure(state=tk.NORMAL)
                    output_display.insert(tk.END, message + "\n")
                    output_display.configure(state=tk.DISABLED)

    except Exception as e:
        print(f"Error receiving data: {e}", file=sys.stderr)

    root.after(100, receive_data)

def show_create_directory_popup():
    hide_context_menu()
    popup = tk.Toplevel(root)
    popup.title("디렉터리 생성")
    popup.geometry("300x100")
    label = tk.Label(popup, text="새 디렉터리 이름을 입력하세요:")
    label.pack(pady=5)
    entry = tk.Entry(popup, width=30)
    entry.pack(pady=5)

    def submit():
        directory_name = entry.get().strip()
        if directory_name:
            execute_command(f"mkdir {directory_name}")
        popup.destroy()

    submit_button = tk.Button(popup, text="확인", command=submit)
    submit_button.pack(pady=5)

def show_directory_context_menu(event, directory_name):
    global current_context_menu
    hide_context_menu()
    context_menu = Menu(root, tearoff=0)
    context_menu.add_command(label="디렉터리 열기", command=lambda: execute_command(f"cd {directory_name}"))
    context_menu.add_command(label="디렉터리 삭제", command=lambda: execute_command(f"rmdir {directory_name}"))

    def show_rename_popup():
        popup = tk.Toplevel(root)
        popup.title("이름 변경")
        popup.geometry("300x100")
        label = tk.Label(popup, text="새 이름을 입력하세요:")
        label.pack(pady=5)
        entry = tk.Entry(popup, width=30)
        entry.pack(pady=5)

        def submit():
            new_name = entry.get().strip()
            if new_name:
                execute_command(f"rename {directory_name} {new_name}")
            popup.destroy()

        submit_button = tk.Button(popup, text="확인", command=submit)
        submit_button.pack(pady=5)

    def show_move_directory_popup():
        popup = tk.Toplevel(root)
        popup.title("디렉터리 이동")
        popup.geometry("300x150")
        label = tk.Label(popup, text="이동할 경로를 입력하세요:")
        label.pack(pady=5)
        entry = tk.Entry(popup, width=30)
        entry.pack(pady=5)

        def submit():
            target_directory = entry.get().strip()
            if target_directory:
                execute_command(f"moveFile {directory_name} {target_directory}")
            popup.destroy()

        submit_button = tk.Button(popup, text="확인", command=submit)
        submit_button.pack(side=tk.LEFT, padx=20, pady=10)

        cancel_button = tk.Button(popup, text="취소", command=popup.destroy)
        cancel_button.pack(side=tk.RIGHT, padx=20, pady=10)

    context_menu.add_command(label="디렉터리 이름 바꾸기", command=show_rename_popup)
    context_menu.add_command(label="디렉터리 이동", command=show_move_directory_popup)
    context_menu.add_command(label="디렉터리 속성", command=lambda: execute_command(f"watchFile {directory_name}"))
    context_menu.post(event.x_root, event.y_root)
    current_context_menu = context_menu


def show_file_context_menu(event, file_name):
    global current_context_menu
    hide_context_menu()
    context_menu = Menu(root, tearoff=0)
    context_menu.add_command(label="파일 열기", command=lambda: execute_command(f"cat {file_name}"))
    context_menu.add_command(label="파일 삭제", command=lambda: execute_command(f"rm {file_name}"))

    def show_rename_popup():
        popup = tk.Toplevel(root)
        popup.title("이름 변경")
        popup.geometry("300x100")
        label = tk.Label(popup, text="새 이름을 입력하세요:")
        label.pack(pady=5)
        entry = tk.Entry(popup, width=30)
        entry.pack(pady=5)

        def submit():
            new_name = entry.get().strip()
            if new_name:
                execute_command(f"rename {file_name} {new_name}")
            popup.destroy()

        submit_button = tk.Button(popup, text="확인", command=submit)
        submit_button.pack(pady=5)

    def show_copy_popup():
        popup = tk.Toplevel(root)
        popup.title("파일 복사")
        popup.geometry("300x100")
        label = tk.Label(popup, text="새 파일 이름을 입력하세요:")
        label.pack(pady=5)
        entry = tk.Entry(popup, width=30)
        entry.pack(pady=5)

        def submit():
            new_name = entry.get().strip()
            if new_name:
                execute_command(f"cp {file_name} {new_name}")
            popup.destroy()

        submit_button = tk.Button(popup, text="확인", command=submit)
        submit_button.pack(pady=5)

    def show_move_file_popup():
        popup = tk.Toplevel(root)
        popup.title("파일 이동")
        popup.geometry("300x150")
        label = tk.Label(popup, text="이동할 경로를 입력하세요:")
        label.pack(pady=5)
        entry = tk.Entry(popup, width=30)
        entry.pack(pady=5)

        def submit():
            target_directory = entry.get().strip()
            if target_directory:
                execute_command(f"moveFile {file_name} {target_directory}")
            popup.destroy()

        submit_button = tk.Button(popup, text="확인", command=submit)
        submit_button.pack(side=tk.LEFT, padx=20, pady=10)

        cancel_button = tk.Button(popup, text="취소", command=popup.destroy)
        cancel_button.pack(side=tk.RIGHT, padx=20, pady=10)

    context_menu.add_command(label="파일 이름 바꾸기", command=show_rename_popup)
    context_menu.add_command(label="파일 복사하기", command=show_copy_popup)
    context_menu.add_command(label="파일 이동", command=show_move_file_popup)
    context_menu.add_command(label="파일 속성", command=lambda: execute_command(f"watchFile {file_name}"))
    context_menu.post(event.x_root, event.y_root)
    current_context_menu = context_menu


def on_right_click(event):
    global current_context_menu
    hide_context_menu()
    context_menu = Menu(root, tearoff=0)
    context_menu.add_command(label="디렉터리 생성", command=show_create_directory_popup)
    context_menu.post(event.x_root, event.y_root)
    current_context_menu = context_menu

def create_gui():
    global root
    root = tk.Tk()
    root.title("File Explorer")

    global output_display
    output_display = tk.Text(root, height=1, width=60, state=tk.DISABLED)
    output_display.pack(pady=10)

    global file_canvas
    frame = tk.Frame(root)
    frame.pack(fill=tk.BOTH, expand=True)

    v_scroll = Scrollbar(frame, orient=tk.VERTICAL)
    v_scroll.pack(side=tk.RIGHT, fill=tk.Y)

    file_canvas = Canvas(frame, bg="white", height=300, width=600, yscrollcommand=v_scroll.set)
    file_canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
    v_scroll.config(command=file_canvas.yview)

    file_canvas.bind("<Button-3>", on_right_click)
    file_canvas.bind("<Button-1>", hide_context_menu)

    root.geometry("600x400")
    root.resizable(False, False)

    receive_data()

    root.mainloop()

if __name__ == "__main__":
    fd = sys.stdin.fileno()
    os.set_blocking(fd, False)
    create_gui()
