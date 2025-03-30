import curses
import os
import re
from curses import textpad

class IclassEditor:
    def __init__(self, stdscr, filename=None):
        self.stdscr = stdscr
        self.buffer = [""] * 1000  # 1000 lines max
        self.row = 0
        self.col = 0
        self.mode = 0  # 0=Command, 1=Insert
        self.filename = filename if filename else ""
        self.language = 0  # 0=Null, 1=C, 2=Python, 3=Assembly
        self.dirty = False
        self.top_line = 0
        self.command = ""
        self.message = ""
        
        # Initialize colors
        curses.start_color()
        curses.init_pair(1, curses.COLOR_WHITE, curses.COLOR_BLACK)  # Default text
        curses.init_pair(2, curses.COLOR_WHITE, curses.COLOR_BLUE)   # Status bar
        curses.init_pair(3, curses.COLOR_YELLOW, curses.COLOR_BLACK) # Line numbers
        curses.init_pair(4, curses.COLOR_GREEN, curses.COLOR_BLACK)  # Keywords
        curses.init_pair(5, curses.COLOR_CYAN, curses.COLOR_BLACK)   # Symbols
        curses.init_pair(6, curses.COLOR_MAGENTA, curses.COLOR_BLACK) # Strings/Registers
        curses.init_pair(7, curses.COLOR_YELLOW, curses.COLOR_BLACK) # Numbers
        curses.init_pair(8, curses.COLOR_BLUE, curses.COLOR_BLACK)   # Comments
        
        # Load file if specified
        if filename and os.path.exists(filename):
            with open(filename, 'r') as file:
                for i, line in enumerate(file):
                    if i >= len(self.buffer):
                        break
                    self.buffer[i] = line.rstrip('\n')
            self.check_language()
    
    def check_language(self):
        if self.filename.endswith('.py'):
            self.language = 2
        elif self.filename.endswith('.c'):
            self.language = 1
        elif self.filename.endswith('.asm') or self.filename.endswith('.s'):
            self.language = 3
        else:
            self.language = 0
    
    def show_help(self):
        self.stdscr.clear()
        
        help_text = [
            "iClass v1.0.1 -- help\n",
            "",
            "Commands:",
            ":w      - Save file",
            ":w <file> - Save as",
            ":q      - Quit",
            ":q!     - Force quit without saving",
            ":h      - Show this help",
            ":c      - Set language to C",
            ":p      - Set language to Python",
            ":a      - Set language to Assembly",
            ":n      - Set language to Plain Text",
            "",
            "Navigation:",
            "Arrow Keys - Move cursor",
            "i         - Enter insert mode",
            "ESC       - Return to command mode",
            "",
            "Written By : Bardia Naziri",
            "",
            "Python version -- Use for HexTech Shell",
            "",
            "Press any key to continue...",
        ]
        
        for i, line in enumerate(help_text, 2):
            self.stdscr.addstr(i, 0, line)
        
        self.stdscr.refresh()
        self.stdscr.getch()
    
    def highlight_code(self, line, y_pos):
        c_keywords = ["int", "char", "float", "double", "if", "else", "while", 
                     "for", "return", "void", "main", "#include", "define"]
        py_keywords = ["def", "if", "else", "elif", "while", "for", "return", 
                      "class", "import", "from", "as", "try", "except", "with"]
        asm_keywords = ["mov", "add", "sub", "mul", "div", "jmp", "cmp", "je", 
                       "jne", "call", "ret", "push", "pop", "int", "section", "global"]
        asm_registers = ["eax", "ebx", "ecx", "edx", "esi", "edi", "esp", "ebp", 
                        "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rsp", "rbp"]
        
        text = self.buffer[line]
        x_pos = 5
        i = 0
        n = len(text)
        
        while i < n:
            # Skip whitespace
            if text[i].isspace():
                self.stdscr.addch(y_pos, x_pos, text[i])
                x_pos += 1
                i += 1
                continue
            
            # Highlight keywords
            found = False
            if self.language == 1:  # C
                for keyword in c_keywords:
                    if text.startswith(keyword, i) and (i + len(keyword) >= n or not text[i + len(keyword)].isalnum()):
                        self.stdscr.attron(curses.color_pair(4))
                        self.stdscr.addstr(y_pos, x_pos, keyword)
                        self.stdscr.attroff(curses.color_pair(4))
                        x_pos += len(keyword)
                        i += len(keyword)
                        found = True
                        break
            elif self.language == 2:  # Python
                for keyword in py_keywords:
                    if text.startswith(keyword, i) and (i + len(keyword) >= n or not text[i + len(keyword)].isalnum()):
                        self.stdscr.attron(curses.color_pair(4))
                        self.stdscr.addstr(y_pos, x_pos, keyword)
                        self.stdscr.attroff(curses.color_pair(4))
                        x_pos += len(keyword)
                        i += len(keyword)
                        found = True
                        break
            elif self.language == 3:  # Assembly
                for keyword in asm_keywords:
                    if text.startswith(keyword, i) and (i + len(keyword) >= n or not text[i + len(keyword)].isalnum()):
                        self.stdscr.attron(curses.color_pair(4))
                        self.stdscr.addstr(y_pos, x_pos, keyword)
                        self.stdscr.attroff(curses.color_pair(4))
                        x_pos += len(keyword)
                        i += len(keyword)
                        found = True
                        break
                if not found:
                    for reg in asm_registers:
                        if text.startswith(reg, i) and (i + len(reg) >= n or not text[i + len(reg)].isalnum()):
                            self.stdscr.attron(curses.color_pair(6))
                            self.stdscr.addstr(y_pos, x_pos, reg)
                            self.stdscr.attroff(curses.color_pair(6))
                            x_pos += len(reg)
                            i += len(reg)
                            found = True
                            break
            
            if found:
                continue
            
            # Highlight strings
            if text[i] in ('"', "'"):
                delim = text[i]
                self.stdscr.attron(curses.color_pair(6))
                self.stdscr.addch(y_pos, x_pos, text[i])
                x_pos += 1
                i += 1
                while i < n and text[i] != delim:
                    self.stdscr.addch(y_pos, x_pos, text[i])
                    x_pos += 1
                    i += 1
                if i < n and text[i] == delim:
                    self.stdscr.addch(y_pos, x_pos, text[i])
                    x_pos += 1
                    i += 1
                self.stdscr.attroff(curses.color_pair(6))
                continue
            
            # Highlight numbers
            if text[i].isdigit():
                self.stdscr.attron(curses.color_pair(7))
                while i < n and (text[i].isdigit() or text[i] == '.' or text[i] == 'x' or
                                (text[i].lower() >= 'a' and text[i].lower() <= 'f')):
                    self.stdscr.addch(y_pos, x_pos, text[i])
                    x_pos += 1
                    i += 1
                self.stdscr.attroff(curses.color_pair(7))
                continue
            
            # Highlight comments
            if text[i] == ';':  # Assembly
                self.stdscr.attron(curses.color_pair(8))
                while i < n:
                    self.stdscr.addch(y_pos, x_pos, text[i])
                    x_pos += 1
                    i += 1
                self.stdscr.attroff(curses.color_pair(8))
                continue
            elif i + 1 < n and text[i] == '/' and text[i+1] == '/':  # C/Python
                self.stdscr.attron(curses.color_pair(8))
                while i < n:
                    self.stdscr.addch(y_pos, x_pos, text[i])
                    x_pos += 1
                    i += 1
                self.stdscr.attroff(curses.color_pair(8))
                continue
            elif text[i] == '#' and self.language == 2:  # Python
                self.stdscr.attron(curses.color_pair(8))
                while i < n:
                    self.stdscr.addch(y_pos, x_pos, text[i])
                    x_pos += 1
                    i += 1
                self.stdscr.attroff(curses.color_pair(8))
                continue
            
            # Highlight symbols
            if text[i] in "{}()[];:=+-*/%&|^~!<>":
                self.stdscr.attron(curses.color_pair(5))
                self.stdscr.addch(y_pos, x_pos, text[i])
                self.stdscr.attroff(curses.color_pair(5))
                x_pos += 1
                i += 1
            else:
                self.stdscr.addch(y_pos, x_pos, text[i])
                x_pos += 1
                i += 1
    
    def draw_screen(self):
        self.stdscr.clear()
        height, width = self.stdscr.getmaxyx()
        screen_height = height - 3
        
        # Adjust top_line if needed
        if self.row < self.top_line:
            self.top_line = self.row
        elif self.row >= self.top_line + screen_height:
            self.top_line = self.row - screen_height + 1
        
        # Draw visible lines
        for i in range(screen_height):
            line_num = i + self.top_line
            if line_num >= len(self.buffer):
                break
            
            # Line numbers
            self.stdscr.attron(curses.color_pair(3))
            self.stdscr.addstr(i, 0, f"{line_num + 1:4d} ")
            self.stdscr.attroff(curses.color_pair(3))
            
            # Line content
            if self.buffer[line_num] or line_num == self.row:
                if self.language != 0:
                    self.highlight_code(line_num, i)
                else:
                    self.stdscr.addstr(i, 5, self.buffer[line_num])
        
        # Status bar
        mode_str = "INSERT" if self.mode else "COMMAND"
        lang_str = {
            0: "Text",
            1: "C",
            2: "Python",
            3: "Assembly"
        }.get(self.language, "Text")
        filename_str = self.filename if self.filename else "[No Name]"
        
        self.stdscr.attron(curses.color_pair(2))
        status_line = f" {mode_str} | {lang_str} | Line {self.row + 1}, Col {self.col + 1} | {filename_str}"
        self.stdscr.addstr(height - 2, 0, status_line.ljust(width - 1))
        
        if self.dirty:
            self.stdscr.attron(curses.color_pair(4))
            self.stdscr.addstr(height - 2, width - 10, "[Modified]")
            self.stdscr.attroff(curses.color_pair(4))
        
        # Command line
        if self.command:
            self.stdscr.addstr(height - 1, 0, f":{self.command}")
        elif self.message:
            self.stdscr.addstr(height - 1, 0, self.message)
            self.message = ""
        else:
            self.stdscr.addstr(height - 1, 0, " :w=Save :q=Quit :h=Help")
        
        self.stdscr.attroff(curses.color_pair(2))
        
        # Position cursor
        try:
            self.stdscr.move(self.row - self.top_line, self.col + 5)
        except curses.error:
            pass
        
        self.stdscr.refresh()
    
    def save_file(self, save_as=False):
        height, width = self.stdscr.getmaxyx()
        
        if not self.filename or save_as:
            self.message = "Enter filename: "
            self.draw_screen()
            curses.echo()
            filename = self.stdscr.getstr(height - 1, len(self.message), 255).decode('utf-8')
            curses.noecho()
            if filename:
                self.filename = filename
                self.check_language()
            else:
                return False
        
        try:
            with open(self.filename, 'w') as file:
                for line in self.buffer:
                    if line is None:
                        break
                    file.write(line + '\n')
            self.dirty = False
            self.message = "File saved successfully."
            return True
        except IOError:
            self.message = "Error: Could not save file!"
            return False
    
    def handle_command_mode(self, ch):
        if ch == ord('i'):
            self.mode = 1
            return
        
        if ch == curses.KEY_LEFT:
            if self.col > 0:
                self.col -= 1
            return
        
        if ch == curses.KEY_RIGHT:
            if self.col < len(self.buffer[self.row]) and self.col < 255:
                self.col += 1
            return
        
        if ch == curses.KEY_UP:
            if self.row > 0:
                self.row -= 1
                if self.col > len(self.buffer[self.row]):
                    self.col = len(self.buffer[self.row])
            return
        
        if ch == curses.KEY_DOWN:
            if self.row < len(self.buffer) - 1 and self.buffer[self.row + 1] is not None:
                self.row += 1
                if self.col > len(self.buffer[self.row]):
                    self.col = len(self.buffer[self.row])
            return
        
        if ch == curses.KEY_HOME:
            self.col = 0
            return
        
        if ch == curses.KEY_END:
            self.col = len(self.buffer[self.row])
            return
        
        if ch == ord(':'):
            self.command = ""
            self.draw_screen()
            curses.echo()
            self.command = self.stdscr.getstr(self.stdscr.getmaxyx()[0] - 1, 1, 255).decode('utf-8')
            curses.noecho()
            
            if self.command.startswith('w'):
                if len(self.command) > 2:
                    self.filename = self.command[2:].strip()
                    self.check_language()
                self.save_file()
            elif self.command.startswith('q'):
                if self.dirty and not self.command.startswith('q!'):
                    self.message = "Unsaved changes! Use :q! to force quit."
                else:
                    raise SystemExit(0)
            elif self.command.startswith('h'):
                self.show_help()
            elif self.command.startswith('c'):
                self.language = 1
            elif self.command.startswith('p'):
                self.language = 2
            elif self.command.startswith('a'):
                self.language = 3
            elif self.command.startswith('n'):
                self.language = 0
            else:
                self.message = f"Unknown command: {self.command}"
            
            self.command = ""
    
    def handle_insert_mode(self, ch):
        if ch == 27:  # ESC
            self.mode = 0
            return
        
        current_line = self.buffer[self.row]
        
        # Handle special keys
        if ch == curses.KEY_BACKSPACE or ch == 127:
            if self.col > 0:
                self.buffer[self.row] = current_line[:self.col - 1] + current_line[self.col:]
                self.col -= 1
                self.dirty = True
            elif self.row > 0:
                prev_line_len = len(self.buffer[self.row - 1])
                self.buffer[self.row - 1] += current_line
                self.buffer.pop(self.row)
                self.buffer.append("")
                self.row -= 1
                self.col = prev_line_len
                self.dirty = True
            return
        
        if ch == curses.KEY_DC:  # Delete
            if self.col < len(current_line):
                self.buffer[self.row] = current_line[:self.col] + current_line[self.col + 1:]
                self.dirty = True
            return
        
        if ch == ord('\n') or ch == curses.KEY_ENTER:  # Enter
            # Split current line
            new_line = current_line[self.col:]
            self.buffer[self.row] = current_line[:self.col]
            
            # Insert new line
            self.buffer.insert(self.row + 1, "")
            if len(self.buffer) > 1000:
                self.buffer.pop()
            
            # Handle indentation
            indent = 0
            if self.language == 2:  # Python
                # Count leading spaces on current line
                indent = len(re.match(r'^\s*', self.buffer[self.row]).group())
                # Increase indent if line ends with colon
                if self.buffer[self.row].rstrip().endswith(':'):
                    indent += 4
                new_line = ' ' * indent + new_line.lstrip()
            elif self.language == 1:  # C
                indent = len(re.match(r'^\s*', self.buffer[self.row]).group())
                if '{' in self.buffer[self.row]:
                    indent += 4
                elif '}' in self.buffer[self.row] and indent >= 4:
                    indent -= 4
                new_line = ' ' * indent + new_line.lstrip()
            
            self.buffer[self.row + 1] = new_line
            self.row += 1
            self.col = indent
            self.dirty = True
            return
        
        # Handle regular characters
        if 32 <= ch <= 126:  # Printable ASCII
            if len(current_line) < 255:
                self.buffer[self.row] = current_line[:self.col] + chr(ch) + current_line[self.col:]
                self.col += 1
                self.dirty = True
    
    def run(self):
        while True:
            self.draw_screen()
            try:
                ch = self.stdscr.getch()
                if self.mode == 0:
                    self.handle_command_mode(ch)
                else:
                    self.handle_insert_mode(ch)
            except curses.error:
                pass

def main(stdscr, filename=None):
    editor = IclassEditor(stdscr, filename)
    editor.run()

if __name__ == "__main__":
    import sys
    filename = sys.argv[1] if len(sys.argv) > 1 else None
    curses.wrapper(main, filename)