#include "command_registry.h"

// Define static members
CommandEntry CommandRegistry::commands[32];
int CommandRegistry::num_commands = 0;

// Helper: String Compare
static int strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void CommandRegistry::Register(const char* name, CommandHandler handler) {
    if (num_commands >= 32) return;
    
    // Check dupe
    for(int i=0; i<num_commands; i++) {
        if (strcmp(commands[i].name, name) == 0) {
            commands[i].handler = handler; // Overwrite
            return;
        }
    }
    
    commands[num_commands].name = name;
    commands[num_commands].handler = handler;
    num_commands++;
}

CommandHandler CommandRegistry::Get(const char* name) {
    for(int i=0; i<num_commands; i++) {
        if (strcmp(commands[i].name, name) == 0) {
            return commands[i].handler;
        }
    }
    return 0; // Null
}

const CommandEntry* CommandRegistry::GetList() {
    return commands;
}

int CommandRegistry::GetCount() {
    return num_commands;
}
