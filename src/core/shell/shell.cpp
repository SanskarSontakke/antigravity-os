#include "shell.h"
#include "../gui/TerminalWindow.h"
#include "../fs/ext4.h"
#include "../mm/kheap.h"
#include "../../drivers/rtc.h"
#include "../../utils/StringHelpers.h"

Shell::Shell(TerminalWindow* win) : editor(win) {
    this->window = win;
    this->cwd[0] = '/';
    this->cwd[1] = 0;

    this->env_count = 0;
    SetEnv("PS1", "\033[32muser@antigravity\033[0m:\033[34m{cwd}\033[0m$ ");
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

    CommandRegistry::Register("cp", CmdCp);
    CommandRegistry::Register("mv", CmdMv);
    CommandRegistry::Register("edit", CmdEdit);
    CommandRegistry::Register("nano", CmdNano);
    CommandRegistry::Register("export", CmdExport);
}

void Shell::Print(const char* str) {
    if(window) window->Print(str);
}

void Shell::SetCWD(const char* path) {
    int i=0;
    while(path[i] && i < 255) { cwd[i] = path[i]; i++; }
    cwd[i] = 0;
    SetEnv("PWD", cwd);
}

const char* Shell::GetCWD() {
    return cwd;
}

void Shell::SetEnv(const char* k, const char* v) {
    // Search
    for(int i=0; i<env_count; i++) {
        if (Utils::strcmp(envs[i].key, k) == 0) {
            Utils::strcpy(envs[i].value, v);
            return;
        }
    }
    if (env_count < 16) {
        Utils::strcpy(envs[env_count].key, k);
        Utils::strcpy(envs[env_count].value, v);
        env_count++;
    }
}

const char* Shell::GetEnv(const char* k) {
    for(int i=0; i<env_count; i++) {
        if (Utils::strcmp(envs[i].key, k) == 0) return envs[i].value;
    }
    return 0;
}

const char* Shell::GetAutocompleteSuggestion(const char* partial) {
    static char suggestion[64];
    suggestion[0] = 0;

    int p_len = Utils::strlen(partial);
    if (p_len == 0) return 0;

    // 1. Commands
    const CommandEntry* cmds = CommandRegistry::GetList();
    int cmd_count = CommandRegistry::GetCount();
    for(int i=0; i<cmd_count; i++) {
        if (Utils::strncmp(cmds[i].name, partial, p_len) == 0) {
            return cmds[i].name;
        }
    }

    // 2. Files
    FileList files = Ext4::GetFileList(GetCWD());
    for(int i=0; i<files.count; i++) {
        if (Utils::strncmp(files.entries[i].name, partial, p_len) == 0) {
            Utils::strcpy(suggestion, files.entries[i].name);
            Ext4::FreeFileList(files);
            return suggestion;
        }
    }
    Ext4::FreeFileList(files);

    return 0;
}

void Shell::Prompt() {
    const char* ps1 = GetEnv("PS1");
    if (!ps1) ps1 = "$ ";

    // Parse {cwd}
    char buf[128];
    int j=0;
    for(int i=0; ps1[i] && j < 127; i++) {
        if (ps1[i] == '{' && ps1[i+1] == 'c' && ps1[i+2] == 'w' && ps1[i+3] == 'd' && ps1[i+4] == '}') {
            const char* pwd = GetCWD();
            for(int k=0; pwd[k] && j < 127; k++) buf[j++] = pwd[k];
            i += 4;
        } else {
            buf[j++] = ps1[i];
        }
    }
    buf[j] = 0;

    Print(buf);
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
    shell->Print("  Filesystem: ls, cd, cat, cp, mv, mkdir, rm, touch, pwd\n");
    shell->Print("  Editor:     edit, nano\n");
    shell->Print("  System:     date, free, uname, uptime, export\n");
    shell->Print("  Terminal:   clear, history, echo, help\n");
}

void Shell::CmdCp(int argc, char** argv, Shell* shell) {
    if (argc < 3) { shell->Print("Usage: cp <src> <dest>\n"); return; }

    int max_size = 1024*20;
    char* buf = (char*)kmalloc(max_size);
    Ext4::ReadFile(argv[1], buf);
    int len = Utils::strlen(buf);

    if (len == 0 && buf[0] == 0) {
        // Empty or not found (Ext4::ReadFile stub returns empty string if not found)
        // If file really empty?
        // Check list?
        // For now assume valid if read.
    }

    Ext4::WriteFile(argv[2], buf, len);
    kfree(buf);
}

void Shell::CmdMv(int argc, char** argv, Shell* shell) {
    if (argc < 3) { shell->Print("Usage: mv <src> <dest>\n"); return; }
    CmdCp(argc, argv, shell);
    Ext4::Rm(argv[1]);
}

void Shell::CmdEdit(int argc, char** argv, Shell* shell) {
    if (argc < 2) { shell->Print("Usage: edit <file>\n"); return; }
    shell->editor.Start(argv[1]);
}

void Shell::CmdNano(int argc, char** argv, Shell* shell) {
    CmdEdit(argc, argv, shell);
}

void Shell::CmdExport(int argc, char** argv, Shell* shell) {
    if (argc < 2) {
        // List envs
        for(int i=0; i<shell->env_count; i++) {
            shell->Print(shell->envs[i].key);
            shell->Print("=");
            shell->Print(shell->envs[i].value);
            shell->Print("\n");
        }
        return;
    }

    // Format: KEY=VALUE
    const char* arg = argv[1];
    char key[32];
    char val[64];
    int k=0, v=0;
    bool in_val = false;

    for(int i=0; arg[i]; i++) {
        if (!in_val) {
            if (arg[i] == '=') {
                in_val = true;
                continue;
            }
            if (k < 31) key[k++] = arg[i];
        } else {
            if (v < 63) val[v++] = arg[i];
        }
    }
    key[k] = 0;
    val[v] = 0;

    if (k > 0) shell->SetEnv(key, val);
}
