import tkinter as tk
from tkinter import scrolledtext
import serial
import threading

PORT = 'COM10'
BAUD = 115200

class SerialPanel:
    def __init__(self, root):
        self.root = root
        self.root.title(f"Blue Box Live Serial Panel - {PORT}")
        self.root.geometry("700x500")

        # Set up a dark hacker-style theme
        self.root.configure(bg='#1e1e1e')

        # Read-only text area for incoming logs
        self.text_area = scrolledtext.ScrolledText(
            root, wrap=tk.WORD, state='disabled', 
            bg='#000000', fg='#00ff00', font=("Consolas", 11)
        )
        self.text_area.pack(padx=10, pady=10, fill=tk.BOTH, expand=True)

        # Input frame at the bottom
        self.entry_frame = tk.Frame(root, bg='#1e1e1e')
        self.entry_frame.pack(padx=10, pady=(0, 10), fill=tk.X)

        # Instruction label
        self.lbl = tk.Label(self.entry_frame, text="Command:", bg='#1e1e1e', fg='white', font=("Arial", 11, "bold"))
        self.lbl.pack(side=tk.LEFT, padx=(0, 5))

        # Command entry field
        self.entry = tk.Entry(self.entry_frame, font=("Consolas", 12), bg='#333333', fg='white', insertbackground='white')
        self.entry.pack(side=tk.LEFT, fill=tk.X, expand=True)
        self.entry.bind("<Return>", self.send_command)

        # Send button
        self.send_btn = tk.Button(self.entry_frame, text="Send", command=self.send_command, bg='#444444', fg='white', font=("Arial", 10, "bold"))
        self.send_btn.pack(side=tk.RIGHT, padx=(5, 0))

        # Quick preset buttons frame
        self.preset_frame = tk.Frame(root, bg='#1e1e1e')
        self.preset_frame.pack(padx=10, pady=(0, 10), fill=tk.X)
        
        tk.Button(self.preset_frame, text="Start Beacon", command=lambda: self.send_preset("start beacon"), bg='#0055a4', fg='white').pack(side=tk.LEFT, padx=(0, 5))
        tk.Button(self.preset_frame, text="Start WiFi Menu", command=lambda: self.send_preset("start wifi"), bg='#0055a4', fg='white').pack(side=tk.LEFT, padx=(0, 5))

        # D-Pad Frame
        self.dpad_frame = tk.Frame(root, bg='#1e1e1e')
        self.dpad_frame.pack(padx=10, pady=10)
        
        # Spacer for top row
        tk.Label(self.dpad_frame, width=8, bg='#1e1e1e').grid(row=0, column=0)
        
        btn_up = tk.Button(self.dpad_frame, text="UP", width=8, bg='#444444', fg='white', font=("Arial", 10, "bold"))
        btn_up.grid(row=0, column=1, pady=2)
        btn_up.bind("<ButtonPress-1>", lambda e: self.send_preset_hidden("btn up"))
        btn_up.bind("<ButtonRelease-1>", lambda e: self.send_preset_hidden("btn release up"))

        btn_left = tk.Button(self.dpad_frame, text="LEFT", width=8, bg='#444444', fg='white', font=("Arial", 10, "bold"))
        btn_left.grid(row=1, column=0, padx=2)
        btn_left.bind("<ButtonPress-1>", lambda e: self.send_preset_hidden("btn left"))
        btn_left.bind("<ButtonRelease-1>", lambda e: self.send_preset_hidden("btn release left"))

        btn_select = tk.Button(self.dpad_frame, text="SELECT", width=8, bg='#00aa00', fg='white', font=("Arial", 10, "bold"))
        btn_select.grid(row=1, column=1, padx=2)
        btn_select.bind("<ButtonPress-1>", lambda e: self.send_preset_hidden("btn select"))

        btn_right = tk.Button(self.dpad_frame, text="RIGHT", width=8, bg='#444444', fg='white', font=("Arial", 10, "bold"))
        btn_right.grid(row=1, column=2, padx=2)
        btn_right.bind("<ButtonPress-1>", lambda e: self.send_preset_hidden("btn right"))
        btn_right.bind("<ButtonRelease-1>", lambda e: self.send_preset_hidden("btn release right"))

        btn_down = tk.Button(self.dpad_frame, text="DOWN", width=8, bg='#444444', fg='white', font=("Arial", 10, "bold"))
        btn_down.grid(row=2, column=1, pady=2)
        btn_down.bind("<ButtonPress-1>", lambda e: self.send_preset_hidden("btn down"))
        btn_down.bind("<ButtonRelease-1>", lambda e: self.send_preset_hidden("btn release down"))

        btn_back = tk.Button(self.dpad_frame, text="BACK", width=8, bg='#aa0000', fg='white', font=("Arial", 10, "bold"))
        btn_back.grid(row=2, column=2, pady=2)
        btn_back.bind("<ButtonPress-1>", lambda e: self.send_preset_hidden("btn back"))

        # Serial Connection
        self.running = True
        try:
            self.ser = serial.Serial(PORT, BAUD, timeout=0.1)
            self.append_text(f"[*] Successfully connected to {PORT} at {BAUD} baud.\n")
            self.append_text("[*] Try typing 'start beacon' and hitting Enter!\n\n")
        except Exception as e:
            self.append_text(f"[!] Failed to connect to {PORT}: {e}\n")
            self.append_text("[!] Make sure your other Serial Monitor is completely closed!\n")
            self.ser = None

        if self.ser:
            self.read_thread = threading.Thread(target=self.read_serial, daemon=True)
            self.read_thread.start()

    def append_text(self, text):
        self.text_area.config(state='normal')
        self.text_area.insert(tk.END, text)
        self.text_area.see(tk.END)
        self.text_area.config(state='disabled')

    def send_preset(self, cmd):
        self.entry.delete(0, tk.END)
        self.entry.insert(0, cmd)
        self.send_command()

    def send_preset_hidden(self, cmd):
        if self.ser:
            try:
                self.ser.write((cmd + '\n').encode('utf-8'))
            except Exception as e:
                self.append_text(f"[!] Write error: {e}\n")

    def send_command(self, event=None):
        if self.ser:
            cmd = self.entry.get().strip()
            self.entry.delete(0, tk.END)
            if cmd:
                self.append_text(f">>> {cmd}\n")
                try:
                    self.ser.write((cmd + '\n').encode('utf-8'))
                except Exception as e:
                    self.append_text(f"[!] Write error: {e}\n")
        else:
            self.append_text("[!] Cannot send command. Not connected to serial port.\n")

    def read_serial(self):
        while self.running:
            try:
                if self.ser.in_waiting:
                    data = self.ser.read(self.ser.in_waiting).decode('utf-8', errors='replace')
                    if data:
                        # Schedule GUI update safely in the main thread
                        self.root.after(10, self.append_text, data)
            except Exception as e:
                self.root.after(10, self.append_text, f"\n[!] Serial read error: {e}\n")
                break

    def on_closing(self):
        self.running = False
        if self.ser:
            self.ser.close()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = SerialPanel(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()
