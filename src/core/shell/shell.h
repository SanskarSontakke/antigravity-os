#ifndef SHELL_H
#define SHELL_H

#include "command_registry.h"
#include "Editor.h"

// Forward declaration
class TerminalWindow;

struct EnvVar {
    char key[32];
    char value[64];
};

class Shell {
private:
    char cwd[256];
    
public:
    TerminalWindow* window;
    Editor editor;

    EnvVar envs[16];
    int env_count;

    Shell(TerminalWindow* win);
    void Init();
    
    // Core
    void Execute(const char* input);
    
    // Utilities for Commands
    void Print(const char* str);
    void SetCWD(const char* path);
    const char* GetCWD();
    
    // Environment
    void SetEnv(const char* key, const char* value);
    const char* GetEnv(const char* key);
    void Prompt(); // Overrides default prompt

    // Autocomplete
    const char* GetAutocompleteSuggestion(const char* partial_str);

    // Command Implementations (using new signature)
    static void CmdLs(int argc, char** argv, Shell* shell);
    static void CmdCd(int argc, char** argv, Shell* shell);
    static void CmdCat(int argc, char** argv, Shell* shell);
    static void CmdMkdir(int argc, char** argv, Shell* shell);
    static void CmdRm(int argc, char** argv, Shell* shell);
    static void CmdTouch(int argc, char** argv, Shell* shell);
    static void CmdPwd(int argc, char** argv, Shell* shell);
    static void CmdDate(int argc, char** argv, Shell* shell);
    static void CmdFree(int argc, char** argv, Shell* shell);
    static void CmdUname(int argc, char** argv, Shell* shell);
    static void CmdUptime(int argc, char** argv, Shell* shell);
    static void CmdClear(int argc, char** argv, Shell* shell);
    static void CmdHistory(int argc, char** argv, Shell* shell);
    static void CmdEcho(int argc, char** argv, Shell* shell);
    static void CmdHelp(int argc, char** argv, Shell* shell);

    // New Commands
    static void CmdCp(int argc, char** argv, Shell* shell);
    static void CmdMv(int argc, char** argv, Shell* shell);
    static void CmdEdit(int argc, char** argv, Shell* shell);
    static void CmdNano(int argc, char** argv, Shell* shell);
    static void CmdExport(int argc, char** argv, Shell* shell);
};

#endif
