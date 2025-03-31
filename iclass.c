#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define MAX_LINES 1000
#define MAX_COLS 256

char buffer[MAX_LINES][MAX_COLS];
int row = 0, col = 0;
int mode = 0;  // 0=Command, 1=Insert
char filename[256] = "";
int language = 0;  // 0=Null, 1=C, 2=Python, 3=Assembly
int dirty = 0;     // Track unsaved changes
int top_line = 0;  // Top visible line

void init_buffer() {
    for (int i = 0; i < MAX_LINES; i++) {
        memset(buffer[i], 0, MAX_COLS);
    }
}

void check_language() {
    if (strlen(filename) >= 3 && strcmp(filename + strlen(filename) - 3, ".py") == 0) {
        language = 2;
    } else if (strlen(filename) >= 2 && strcmp(filename + strlen(filename) - 2, ".c") == 0) {
        language = 1;
    } else if (strlen(filename) >= 4 && (strcmp(filename + strlen(filename) - 4, ".asm") == 0 || 
                                         strcmp(filename + strlen(filename) - 4, ".s") == 0)) {
        language = 3;
    } else {
        language = 0;
    }
}

void show_help() {
    clear();
    attron(COLOR_PAIR(2));
    mvprintw(0, 0, "Iclass v1.0.1 - Help");
    attroff(COLOR_PAIR(2));
    
    mvprintw(2, 0, "Commands:");
    mvprintw(3, 4, ":w      - Save file");
    mvprintw(4, 4, ":w <file> - Save as");
    mvprintw(5, 4, ":q      - Quit");
    mvprintw(6, 4, ":q!     - Force quit without saving");
    mvprintw(7, 4, ":h      - Show this help");
    mvprintw(8, 4, ":c      - Set language to C");
    mvprintw(9, 4, ":p      - Set language to Python");
    mvprintw(10, 4, ":a      - Set language to Assembly");
    mvprintw(11, 4, ":n      - Set language to Plain Text");
    mvprintw(12, 4, "Author : Bardia Naziri");
    
    mvprintw(13, 0, "Navigation:");
    mvprintw(14, 4, "Arrow Keys - Move cursor");
    mvprintw(15, 4, "i         - Enter insert mode");
    mvprintw(16, 4, "ESC       - Return to command mode");
    
    mvprintw(18, 0, "Press any key to continue...");
    refresh();
    getch();
}

void highlight_code(int line) {
    char *c_keywords[] = {"int", "char", "float", "double", "if", "else", "while", 
                         "for", "return", "void", "main", "#include", "define"};
    char *py_keywords[] = {"def", "if", "else", "elif", "while", "for", "return", 
                          "class", "import", "from", "as", "try", "except", "with"};
    char *asm_keywords[] = {"mov", "add", "sub", "mul", "div", "jmp", "cmp", "je", 
                           "jne", "call", "ret", "push", "pop", "int", "section", "global"};
    char *asm_registers[] = {"eax", "ebx", "ecx", "edx", "esi", "edi", "esp", "ebp", 
                            "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rsp", "rbp"};

    int num_c = sizeof(c_keywords)/sizeof(c_keywords[0]);
    int num_py = sizeof(py_keywords)/sizeof(py_keywords[0]);
    int num_asm = sizeof(asm_keywords)/sizeof(asm_keywords[0]);
    int num_reg = sizeof(asm_registers)/sizeof(asm_registers[0]);

    char *ptr = buffer[line];
    int screen_line = line - top_line;
    while (*ptr) {
        // Skip whitespace
        while (*ptr && isspace(*ptr)) {
            mvprintw(screen_line, 5 + (ptr - buffer[line]), "%c", *ptr);
            ptr++;
        }
        if (!*ptr) break;

        // Highlight keywords
        int found = 0;
        if (language == 1) { // C
            for (int i = 0; i < num_c; i++) {
                int len = strlen(c_keywords[i]);
                if (strncmp(ptr, c_keywords[i], len) == 0 && !isalnum(ptr[len])) {
                    attron(COLOR_PAIR(4));
                    mvprintw(screen_line, 5 + (ptr - buffer[line]), "%.*s", len, ptr);
                    attroff(COLOR_PAIR(4));
                    ptr += len;
                    found = 1;
                    break;
                }
            }
        } else if (language == 2) { // Python
            for (int i = 0; i < num_py; i++) {
                int len = strlen(py_keywords[i]);
                if (strncmp(ptr, py_keywords[i], len) == 0 && !isalnum(ptr[len])) {
                    attron(COLOR_PAIR(4));
                    mvprintw(screen_line, 5 + (ptr - buffer[line]), "%.*s", len, ptr);
                    attroff(COLOR_PAIR(4));
                    ptr += len;
                    found = 1;
                    break;
                }
            }
        } else if (language == 3) { // Assembly
            for (int i = 0; i < num_asm; i++) {
                int len = strlen(asm_keywords[i]);
                if (strncmp(ptr, asm_keywords[i], len) == 0 && !isalnum(ptr[len])) {
                    attron(COLOR_PAIR(4));
                    mvprintw(screen_line, 5 + (ptr - buffer[line]), "%.*s", len, ptr);
                    attroff(COLOR_PAIR(4));
                    ptr += len;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                for (int i = 0; i < num_reg; i++) {
                    int len = strlen(asm_registers[i]);
                    if (strncmp(ptr, asm_registers[i], len) == 0 && !isalnum(ptr[len])) {
                        attron(COLOR_PAIR(6));
                        mvprintw(screen_line, 5 + (ptr - buffer[line]), "%.*s", len, ptr);
                        attroff(COLOR_PAIR(6));
                        ptr += len;
                        found = 1;
                        break;
                    }
                }
            }
        }

        if (found) continue;

        // Highlight strings
        if (*ptr == '"' || *ptr == '\'') {
            char delim = *ptr;
            attron(COLOR_PAIR(6));
            mvprintw(screen_line, 5 + (ptr - buffer[line]), "%c", *ptr);
            ptr++;
            while (*ptr && *ptr != delim) {
                mvprintw(screen_line, 5 + (ptr - buffer[line]), "%c", *ptr);
                ptr++;
            }
            if (*ptr == delim) {
                mvprintw(screen_line, 5 + (ptr - buffer[line]), "%c", *ptr);
                ptr++;
            }
            attroff(COLOR_PAIR(6));
            continue;
        }

        // Highlight numbers
        if (isdigit(*ptr)) {
            attron(COLOR_PAIR(7));
            while (*ptr && (isdigit(*ptr) || *ptr == '.' || *ptr == 'x' || 
                          (*ptr >= 'a' && *ptr <= 'f') || (*ptr >= 'A' && *ptr <= 'F'))) {
                mvprintw(screen_line, 5 + (ptr - buffer[line]), "%c", *ptr);
                ptr++;
            }
            attroff(COLOR_PAIR(7));
            continue;
        }

        // Highlight comments
        if (*ptr == ';') { // Assembly
            attron(COLOR_PAIR(8));
            while (*ptr) {
                mvprintw(screen_line, 5 + (ptr - buffer[line]), "%c", *ptr);
                ptr++;
            }
            attroff(COLOR_PAIR(8));
            continue;
        } else if (*ptr == '/' && *(ptr + 1) == '/') { // C/Python
            attron(COLOR_PAIR(8));
            while (*ptr) {
                mvprintw(screen_line, 5 + (ptr - buffer[line]), "%c", *ptr);
                ptr++;
            }
            attroff(COLOR_PAIR(8));
            continue;
        } else if (*ptr == '#' && language == 2) { // Python
            attron(COLOR_PAIR(8));
            while (*ptr) {
                mvprintw(screen_line, 5 + (ptr - buffer[line]), "%c", *ptr);
                ptr++;
            }
            attroff(COLOR_PAIR(8));
            continue;
        }

        // Highlight symbols
        if (strchr("{}()[];:=+-*/%&|^~!<>", *ptr)) {
            attron(COLOR_PAIR(5));
            mvprintw(screen_line, 5 + (ptr - buffer[line]), "%c", *ptr);
            attroff(COLOR_PAIR(5));
            ptr++;
        } else {
            mvprintw(screen_line, 5 + (ptr - buffer[line]), "%c", *ptr);
            ptr++;
        }
    }
}

void draw_screen() {
    clear();
    int screen_height = LINES - 3;
    
    // Adjust top_line if needed
    if (row < top_line) {
        top_line = row;
    } else if (row >= top_line + screen_height) {
        top_line = row - screen_height + 1;
    }

    // Draw visible lines
    for (int i = 0; i < screen_height && (i + top_line) < MAX_LINES; i++) {
        int line_num = i + top_line;
        attron(COLOR_PAIR(3));
        mvprintw(i, 0, "%4d ", line_num + 1);
        attroff(COLOR_PAIR(3));

        if (buffer[line_num][0] || line_num == row) {
            if (language != 0) {
                highlight_code(line_num);
            } else {
                mvprintw(i, 5, "%s", buffer[line_num]);
            }
        }
    }

    // Status bar
    attron(COLOR_PAIR(2));
    mvprintw(LINES - 2, 0, " %s | %s | Line %d, Col %d | %s", 
             mode ? "INSERT" : "COMMAND",
             language == 1 ? "C" : language == 2 ? "Python" : language == 3 ? "Assembly" : "Text",
             row + 1, col + 1,
             filename[0] ? filename : "[No Name]");
    
    if (dirty) {
        attron(COLOR_PAIR(4));
        mvprintw(LINES - 2, COLS - 10, "[Modified]");
        attroff(COLOR_PAIR(4));
    }
    
    mvprintw(LINES - 1, 0, " :w=Save :q=Quit :h=Help");
    attroff(COLOR_PAIR(2));

    // Position cursor
    move(row - top_line, col + 5);
    refresh();
}

int save_file() {
    if (!filename[0]) {
        echo();
        mvprintw(LINES - 1, 0, "Enter filename: ");
        getstr(filename);
        noecho();
        check_language();
    }

    FILE *file = fopen(filename, "w");
    if (!file) {
        mvprintw(LINES - 1, 0, "Error: Could not save file!");
        return 0;
    }

    for (int i = 0; i < MAX_LINES; i++) {
        if (buffer[i][0] || i == MAX_LINES - 1) {
            fprintf(file, "%s\n", buffer[i]);
        }
    }
    fclose(file);
    dirty = 0;
    return 1;
}

void handle_command_mode(int ch) {
    switch (ch) {
        case 'i': 
            mode = 1; 
            break;
            
        case KEY_LEFT: 
            if (col > 0) col--; 
            break;
            
        case KEY_RIGHT:
            if (col < MAX_COLS - 1 && buffer[row][col] != '\0') col++;
            break;
            
        case KEY_UP:
            if (row > 0) {
                row--;
                if (col > strlen(buffer[row])) {
                    col = strlen(buffer[row]);
                }
            }
            break;
            
        case KEY_DOWN:
            if (row < MAX_LINES - 1 && buffer[row + 1][0] != '\0') {
                row++;
                if (col > strlen(buffer[row])) {
                    col = strlen(buffer[row]);
                }
            }
            break;
            
        case KEY_HOME:
            col = 0;
            break;
            
        case KEY_END:
            col = strlen(buffer[row]);
            break;
            
        case ':': {
            echo();
            mvprintw(LINES - 1, 0, ":");
            char cmd[MAX_COLS];
            getstr(cmd);
            noecho();

            if (strncmp(cmd, "w", 1) == 0) {
                if (save_file()) {
                    mvprintw(LINES - 1, 0, "File saved successfully.");
                }
            } 
            else if (strncmp(cmd, "q", 1) == 0) {
                if (dirty && cmd[1] != '!') {
                    mvprintw(LINES - 1, 0, "Unsaved changes! Use :q! to force quit.");
                } else {
                    endwin();
                    exit(0);
                }
            }
            else if (strncmp(cmd, "h", 1) == 0) {
                show_help();
            }
            else if (strncmp(cmd, "c", 1) == 0) {
                language = 1;
            }
            else if (strncmp(cmd, "p", 1) == 0) {
                language = 2;
            }
            else if (strncmp(cmd, "a", 1) == 0) {
                language = 3;
            }
            else if (strncmp(cmd, "n", 1) == 0) {
                language = 0;
            }
            else {
                mvprintw(LINES - 1, 0, "Unknown command: %s", cmd);
            }
            break;
        }
    }
}

void handle_insert_mode(int ch) {
    if (ch == 27) { // ESC
        mode = 0;
        return;
    }

    // Handle special keys
    switch (ch) {
        case 127: // Backspace
        case KEY_BACKSPACE:
            if (col > 0) {
                memmove(&buffer[row][col-1], &buffer[row][col], strlen(&buffer[row][col]) + 1);
                col--;
                dirty = 1;
            } else if (row > 0) {
                int new_col = strlen(buffer[row-1]);
                strcat(buffer[row-1], buffer[row]);
                for (int i = row; i < MAX_LINES - 1; i++) {
                    strcpy(buffer[i], buffer[i+1]);
                }
                row--;
                col = new_col;
                dirty = 1;
            }
            return;
            
        case KEY_DC: // Delete
            if (buffer[row][col]) {
                memmove(&buffer[row][col], &buffer[row][col+1], strlen(&buffer[row][col+1]) + 1);
                dirty = 1;
            }
            return;
            
        case '\n': // Enter
            if (row < MAX_LINES - 1) {
                // Move lines down
                for (int i = MAX_LINES - 1; i > row + 1; i--) {
                    strcpy(buffer[i], buffer[i-1]);
                }
                
                // Split current line
                strcpy(buffer[row+1], &buffer[row][col]);
                buffer[row][col] = '\0';
                
                // Handle indentation
                if (language == 2) { // Python
                    int indent = 0;
                    while (buffer[row][indent] == ' ') indent++;
                    if (buffer[row][strlen(buffer[row])-1] == ':') indent += 4;
                    for (int i = 0; i < indent; i++) {
                        buffer[row+1][i] = ' ';
                    }
                    buffer[row+1][indent] = '\0';
                    col = indent;
                } 
                else if (language == 1) { // C
                    int indent = 0;
                    while (buffer[row][indent] == ' ') indent++;
                    if (strchr(buffer[row], '{')) indent += 4;
                    else if (strchr(buffer[row], '}') && indent >= 4) indent -= 4;
                    for (int i = 0; i < indent; i++) {
                        buffer[row+1][i] = ' ';
                    }
                    buffer[row+1][indent] = '\0';
                    col = indent;
                }
                
                row++;
                dirty = 1;
            }
            return;
    }

    // Handle regular characters
    if (isprint(ch)) {
        if (col < MAX_COLS - 2) {
            if (buffer[row][col]) {
                memmove(&buffer[row][col+1], &buffer[row][col], strlen(&buffer[row][col]) + 1);
            }
            buffer[row][col] = ch;
            col++;
            dirty = 1;
        }
    }
}

int main(int argc, char *argv[]) {
    // Initialize ncurses
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    start_color();
    
    // Color pairs
    init_pair(1, COLOR_WHITE, COLOR_BLACK);  // Default text
    init_pair(2, COLOR_WHITE, COLOR_BLUE);   // Status bar
    init_pair(3, COLOR_YELLOW, COLOR_BLACK); // Line numbers
    init_pair(4, COLOR_GREEN, COLOR_BLACK);  // Keywords
    init_pair(5, COLOR_CYAN, COLOR_BLACK);   // Symbols
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);// Strings/Registers
    init_pair(7, COLOR_YELLOW, COLOR_BLACK); // Numbers
    init_pair(8, COLOR_BLUE, COLOR_BLACK);   // Comments

    init_buffer();
    
    // Load file if specified
    if (argc > 1) {
        strncpy(filename, argv[1], 255);
        FILE *file = fopen(filename, "r");
        if (file) {
            int i = 0;
            while (i < MAX_LINES && fgets(buffer[i], MAX_COLS, file)) {
                buffer[i][strcspn(buffer[i], "\n")] = '\0';
                i++;
            }
            fclose(file);
            check_language();
        } else {
            mvprintw(0, 0, "Could not open file: %s", filename);
            refresh();
            sleep(1);
        }
    }

    // Main loop
    int ch;
    while (1) {
        draw_screen();
        ch = getch();
        
        if (mode == 0) {
            handle_command_mode(ch);
        } else {
            handle_insert_mode(ch);
        }
    }

    endwin();
    return 0;
}