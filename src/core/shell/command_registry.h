#ifndef COMMAND_REGISTRY_H
#define COMMAND_REGISTRY_H

class Shell;

// Command Handler Function Pointer Type
typedef void (*CommandHandler)(int argc, char** argv, Shell* shell);

struct CommandEntry {
    const char* name;
    CommandHandler handler;
};

class CommandRegistry {
private:
    static CommandEntry commands[32];
    static int num_commands;

public:
    static void Register(const char* name, CommandHandler handler);
    static CommandHandler Get(const char* name);
    static const CommandEntry* GetList();
    static int GetCount();
};

#endif
