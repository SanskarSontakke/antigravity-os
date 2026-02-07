#include "shell.h"
#include "../gui/TerminalWindow.h"
#include "../fs/ext4.h"
#include "../mm/kheap.h"
#include "../../drivers/rtc.h"

// Helper: String Utils
static int strlen(const char* str) {
    int len=0; while(str[len]) len++; return len;
}
static void strcpy(char* dest, const char* src) {
    while(*src) *dest++ = *src++;
    *dest = 0;
}
static int strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}
static void strcat(char* dest, const char* src) {
    while(*dest) dest++;
    while(*src) *dest++ = *src++;
    *dest = 0;
}

Shell::Shell(TerminalWindow* win) {
    this->window = win;
    this->cwd[0] = '/';
    this->cwd[1] = 0;
}

void Shell::Init() {
    CommandRegistry::Register("ls", CmdLs);
    CommandRegistry::Register("cd", CmdCd);
    CommandRegistry::Register("cat", CmdCat);
    CommandRegistry::Register("mkdir", CmdMkdir);
    CommandRegistry::Register("rm", CmdRm);
    CommandRegistry::Register("touch", CmdTouch);
    CommandRegistry::Register("pwd", CmdPwd);
    CommandRegistry::Register("date", CmdDate);
    CommandRegistry::Register("free", CmdFree);
    CommandRegistry::Register("uname", CmdUname);
    CommandRegistry::Register("uptime", CmdUptime);
    CommandRegistry::Register("clear", CmdClear);
    CommandRegistry::Register("history", CmdHistory);
    CommandRegistry::Register("echo", CmdEcho);
    CommandRegistry::Register("help", CmdHelp);
}

void Shell::Print(const char* str) {
    if(window) window->Print(str);
}

void Shell::SetCWD(const char* path) {
    // Simple path copy for now.
    // Ideally we should resolve ".." and "." here.
    int i=0;
    while(path[i] && i < 255) { cwd[i] = path[i]; i++; }
    cwd[i] = 0;
}

const char* Shell::GetCWD() {
    return cwd;
}

void Shell::Execute(const char* input) {
    if (!input || input[0] == 0) return;
    
    // Tokenizer
    char* argv[16];
    int argc = 0;
    
    char* buffer = (char*)kmalloc(strlen(input) + 1);
    strcpy(buffer, input);

    int len = strlen(buffer);
    int start = 0;
    bool in_quotes = false;
    bool in_token = false;

    for(int i=0; i<=len; i++) {
        char c = buffer[i];

        if (c == '\"') {
            if (in_quotes) {
                // End of quote
                buffer[i] = 0;
                if (argc < 16) {
                    argv[argc++] = &buffer[start];
                }
                in_token = false;
                in_quotes = false;
            } else {
                // Start of quote
                in_quotes = true;
                start = i + 1;
            }
            continue;
        }

        if (in_quotes) continue;

        if (c == ' ' || c == 0) {
            if (in_token) {
                buffer[i] = 0;
                if (argc < 16) argv[argc++] = &buffer[start];
                in_token = false;
            }
        } else {
            if (!in_token) {
                start = i;
                in_token = true;
            }
        }
    }
    
    if (argc > 0) {
        CommandHandler handler = CommandRegistry::Get(argv[0]);
        if (handler) {
            handler(argc, argv, this);
        } else {
            Print("Unknown command: ");
            Print(argv[0]);
            Print("\n");
        }
    }

    kfree(buffer);
}

// --- Command Implementations ---

void Shell::CmdLs(int argc, char** argv, Shell* shell) {
    char buf[1024];
    const char* path = shell->GetCWD();
    bool show_details = false;
    
    // Simple Argument Parser
    for(int i=1; i<argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            show_details = true;
        } else {
            path = argv[i];
        }
    }

    Ext4::Ls(path, buf, 1024, show_details);
    shell->Print(buf);
}

void Shell::CmdCd(int argc, char** argv, Shell* shell) {
    if (argc < 2) {
        shell->SetCWD("/"); // Go home
        return;
    }

    const char* path = argv[1];

    // Validation
    // Note: DirExists is currently a stub for the read-only FS.
    // In a full implementation, we would resolve the full path before checking.
    if (!Ext4::DirExists(path)) {
         shell->Print("cd: ");
         shell->Print(path);
         shell->Print(": Directory not found\n");
         return;
    }

    // Handle Special Cases
    if (strcmp(path, ".") == 0) return;
    if (strcmp(path, "/") == 0) {
        shell->SetCWD("/");
        return;
    }
    if (strcmp(path, "..") == 0) {
        // Go up one level
        // Find last slash
        char temp[256];
        strcpy(temp, shell->GetCWD());
        int len = strlen(temp);
        if (len > 1) { // Don't go up from root
            // Remove trailing slash if any
            if (temp[len-1] == '/') { temp[len-1] = 0; len--; }

            // Find last slash
            int last_slash = -1;
            for(int i=len-1; i>=0; i--) {
                if (temp[i] == '/') { last_slash = i; break; }
            }

            if (last_slash >= 0) {
                if (last_slash == 0) temp[1] = 0; // Root
                else temp[last_slash] = 0;
                shell->SetCWD(temp);
            }
        }
        return;
    }

    // Relative Path handling
    if (path[0] != '/') {
        char temp[256];
        strcpy(temp, shell->GetCWD());
        int len = strlen(temp);
        if (temp[len-1] != '/') {
            temp[len] = '/';
            temp[len+1] = 0;
        }
        strcat(temp, path);
        shell->SetCWD(temp);
    } else {
        shell->SetCWD(path);
    }
}

void Shell::CmdCat(int argc, char** argv, Shell* shell) {
    if (argc < 2) { shell->Print("Usage: cat <file>\n"); return; }
    
    char buf[1024];
    Ext4::ReadFile(argv[1], buf);
    shell->Print(buf);
    shell->Print("\n");
}

void Shell::CmdMkdir(int argc, char** argv, Shell* shell) {
    if (argc < 2) { shell->Print("Usage: mkdir <dir>\n"); return; }
    Ext4::MkDir(argv[1]);
}

void Shell::CmdRm(int argc, char** argv, Shell* shell) {
    if (argc < 2) { shell->Print("Usage: rm <file>\n"); return; }
    Ext4::Rm(argv[1]);
}

void Shell::CmdTouch(int argc, char** argv, Shell* shell) {
    if (argc < 2) { shell->Print("Usage: touch <file>\n"); return; }
    Ext4::Touch(argv[1]);
}

void Shell::CmdPwd(int argc, char** argv, Shell* shell) {
    shell->Print(shell->GetCWD());
    shell->Print("\n");
}

void Shell::CmdDate(int argc, char** argv, Shell* shell) {
    char buf[32];
    int h = RTC::GetHour();
    int m = RTC::GetMinute();
    int s = RTC::GetSecond();

    // Simple int to str
    // "HH:MM:SS"
    buf[0] = '0' + (h/10); buf[1] = '0' + (h%10); buf[2] = ':';
    buf[3] = '0' + (m/10); buf[4] = '0' + (m%10); buf[5] = ':';
    buf[6] = '0' + (s/10); buf[7] = '0' + (s%10); buf[8] = '\n'; buf[9] = 0;

    shell->Print("Current Time (RTC): ");
    shell->Print(buf);
}

void Shell::CmdFree(int argc, char** argv, Shell* shell) {
    shell->Print("Heap Status:\n");
    // Placeholder as KHeap doesn't expose stats yet
    shell->Print("  Total: 10MB\n");
    shell->Print("  Used:  (Tracking pending)\n");
}

void Shell::CmdUname(int argc, char** argv, Shell* shell) {
    shell->Print("Antigravity OS v1.1 - Custom x86 Kernel\n");
    shell->Print("Build: "); shell->Print(__DATE__); shell->Print(" "); shell->Print(__TIME__); shell->Print("\n");
}

void Shell::CmdUptime(int argc, char** argv, Shell* shell) {
    shell->Print("Uptime: (PIT Tick stats pending)\n");
}

void Shell::CmdClear(int argc, char** argv, Shell* shell) {
    if(shell->window) shell->window->Clear();
}

void Shell::CmdHistory(int argc, char** argv, Shell* shell) {
    if (!shell->window) return;

    for(int i=0; i < shell->window->history_count; i++) {
        char num[4];
        num[0] = '0' + i; num[1] = ':'; num[2] = ' '; num[3] = 0;
        shell->Print(num);
        shell->Print(shell->window->history[i]);
        shell->Print("\n");
    }
}

void Shell::CmdEcho(int argc, char** argv, Shell* shell) {
    for(int i=1; i<argc; i++) {
        shell->Print(argv[i]);
        if (i < argc-1) shell->Print(" ");
    }
    shell->Print("\n");
}

void Shell::CmdHelp(int argc, char** argv, Shell* shell) {
    shell->Print("Available commands:\n");
    shell->Print("  Filesystem: ls, cd, cat, mkdir, rm, touch, pwd\n");
    shell->Print("  System:     date, free, uname, uptime\n");
    shell->Print("  Terminal:   clear, history, echo, help\n");
}
