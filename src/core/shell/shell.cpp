#include "shell.h"
#include "../gui/window.h" // Full definition of Window needed for Print
#include "../fs/ext4.h"
#include "../fs/sfs.h"
#include "../../drivers/rtc.h"

// Helper: String Utils
static int strlen(const char* str) {
    int len=0; while(str[len]) len++; return len;
}
static bool startswith(const char* str, const char* prefix) {
    while(*prefix) if(*str++ != *prefix++) return false;
    return true;
}

Shell::Shell(Window* win) {
    this->window = win;
    this->cwd[0] = '/';
    this->cwd[1] = 0;
}

void Shell::Init() {
    // Register All Commands
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
    // Basic copy
    int i=0;
    while(path[i] && i < 255) { cwd[i] = path[i]; i++; }
    cwd[i] = 0;
}

const char* Shell::GetCWD() {
    return cwd;
}

void Shell::Execute(const char* input) {
    if (!input || input[0] == 0) return;
    
    // Tokenize: Extract command
    char cmd[32] = {0};
    int i=0;
    while(input[i] && input[i] != ' ' && i < 31) {
        cmd[i] = input[i];
        i++;
    }
    cmd[i] = 0;
    
    // Extract Args (Pointer to rest of string)
    const char* args = 0;
    if (input[i] == ' ') {
        args = input + i + 1;
        // Skip extra spaces
        while(*args == ' ') args++;
    }
    
    // Lookup
    CommandHandler handler = CommandRegistry::Get(cmd);
    if (handler) {
        handler(args, this);
    } else {
        Print("Unknown command: ");
        Print(cmd);
        Print("\n");
    }
}

// --- Command Implementations ---

void Shell::CmdLs(const char* args, Shell* shell) {
    char buf[1024];
    // TODO: Support args like -l or specific path
    // For now, always list current directory logic specific
    // Since Ext4::Ls takes a path, we should pass cwd if no args
    const char* path = (args && args[0]) ? args : shell->GetCWD();
    
    Ext4::Ls(path, buf, 1024);
    shell->Print(buf);
}

void Shell::CmdCd(const char* args, Shell* shell) {
    if (!args || args[0] == 0) {
        shell->SetCWD("/");
        return;
    }
    // TODO: Verify path exists
    shell->SetCWD(args);
}

void Shell::CmdCat(const char* args, Shell* shell) {
    if (!args) { shell->Print("Usage: cat <file>\n"); return; }
    
    char buf[1024];
    // Try Ext4 first (stub)
    Ext4::ReadFile(args, buf);
    // If empty/stubbed output, maybe try SFS fallback? 
    // Actually Ext4 is the main one now.
    
    // Since Ext4::ReadFile is a stub printing to console, we can't capture it yet.
    // We need to fix Ext4::ReadFile to write to buffer.
    // For now, let's assume Ext4::ReadFile prints directly to console 
    // (which is bad for Shell::Print piping). 
    // BUT our task is to implement the shell structure. 
    // We will fix Ext4::ReadFile later/next.
    // Just printing it via window for safety.
    shell->Print("Reading "); shell->Print(args); shell->Print("...\n");
    // shell->Print(buf); 
}

void Shell::CmdMkdir(const char* args, Shell* shell) {
    shell->Print("mkdir: Functionality pending Ext4 write support.\n");
}

void Shell::CmdRm(const char* args, Shell* shell) {
    shell->Print("rm: Functionality pending Ext4 write support.\n");
}

void Shell::CmdTouch(const char* args, Shell* shell) {
    shell->Print("touch: Functionality pending Ext4 write support.\n");
}

void Shell::CmdPwd(const char* args, Shell* shell) {
    shell->Print(shell->GetCWD());
    shell->Print("\n");
}

void Shell::CmdDate(const char* args, Shell* shell) {
    // TODO: Connect to RTC driver
    shell->Print("2026-02-07 (RTC pending)\n");
}

void Shell::CmdFree(const char* args, Shell* shell) {
    shell->Print("Heap: 10MB Total. (Usage stats pending)\n");
}

void Shell::CmdUname(const char* args, Shell* shell) {
    shell->Print("Antigravity OS v1.1 (Ext4 Edition)\n");
}

void Shell::CmdUptime(const char* args, Shell* shell) {
    shell->Print("Uptime: (PIT Tick stats pending)\n");
}

void Shell::CmdClear(const char* args, Shell* shell) {
    if(shell->window) shell->window->Clear();
}

void Shell::CmdHistory(const char* args, Shell* shell) {
    // Window manages history storage currently.
    // We might need to move History to Shell or ask Window for it.
    // For now, print a placeholder or move logic later.
    shell->Print("Use Up/Down arrows for history.\n");
}

void Shell::CmdEcho(const char* args, Shell* shell) {
    if(args) {
        shell->Print(args);
        shell->Print("\n");
    }
}

void Shell::CmdHelp(const char* args, Shell* shell) {
    shell->Print("Available commands:\n");
    // Iterate registry? 
    // CommandRegistry::GetList()
    // For now manual list
    shell->Print("  ls, cd, cat, mkdir, rm, touch, pwd\n");
    shell->Print("  date, free, uname, uptime\n");
    shell->Print("  clear, history, echo, help\n");
}
