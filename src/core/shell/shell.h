#ifndef SHELL_H
#define SHELL_H

#include "command_registry.h"

// Forward declaration
class TerminalWindow;

class Shell {
private:
    char cwd[256];
    TerminalWindow* window;
    
public:
    Shell(TerminalWindow* win);
    void Init();
    
    // Core
    void Execute(const char* input);
    
    // Utilities for Commands
    void Print(const char* str);
    void SetCWD(const char* path);
    const char* GetCWD();
    
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
};

#endif
