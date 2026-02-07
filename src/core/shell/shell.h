#ifndef SHELL_H
#define SHELL_H

#include "command_registry.h"

// Forward declaration
class Window;

class Shell {
private:
    char cwd[256];
    Window* window;
    
public:
    Shell(Window* win);
    void Init();
    
    // Core
    void Execute(const char* input);
    
    // Utilities for Commands
    void Print(const char* str);
    void SetCWD(const char* path);
    const char* GetCWD();
    
    // Command Implementations (Static or Member?)
    // Making them static fits the Registry, but they need access to Shell instance.
    // The Registry typedef is: void (*CommandHandler)(const char* args, Shell* shell);
    // So we can make them static helper functions in shell.cpp or static members here.
    
    static void CmdLs(const char* args, Shell* shell);
    static void CmdCd(const char* args, Shell* shell);
    static void CmdCat(const char* args, Shell* shell);
    static void CmdMkdir(const char* args, Shell* shell);
    static void CmdRm(const char* args, Shell* shell);
    static void CmdTouch(const char* args, Shell* shell);
    static void CmdPwd(const char* args, Shell* shell);
    static void CmdDate(const char* args, Shell* shell);
    static void CmdFree(const char* args, Shell* shell);
    static void CmdUname(const char* args, Shell* shell);
    static void CmdUptime(const char* args, Shell* shell);
    static void CmdClear(const char* args, Shell* shell);
    static void CmdHistory(const char* args, Shell* shell);
    static void CmdEcho(const char* args, Shell* shell);
    static void CmdHelp(const char* args, Shell* shell);
};

#endif
