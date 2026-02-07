#ifndef STRING_HELPERS_H
#define STRING_HELPERS_H

#include "../core/mm/kheap.h"

namespace Utils {
    static int strlen(const char* str) {
        int len=0; while(str[len]) len++; return len;
    }

    static void strcpy(char* dest, const char* src) {
        while(*src) *dest++ = *src++;
        *dest = 0;
    }

    static void strncpy(char* dest, const char* src, int n) {
        int i=0;
        while(src[i] && i < n) {
            dest[i] = src[i];
            i++;
        }
        dest[i] = 0;
    }

    static int strcmp(const char* s1, const char* s2) {
        while(*s1 && (*s1 == *s2)) { s1++; s2++; }
        return *(const unsigned char*)s1 - *(const unsigned char*)s2;
    }

    static int strncmp(const char* s1, const char* s2, int n) {
        while(n > 0 && *s1 && (*s1 == *s2)) {
            s1++; s2++;
            n--;
        }
        if (n == 0) return 0;
        return *(const unsigned char*)s1 - *(const unsigned char*)s2;
    }

    static char* strdup(const char* s) {
        int len = strlen(s);
        char* d = (char*)kmalloc(len + 1);
        strcpy(d, s);
        return d;
    }

    static void strcat(char* dest, const char* src) {
        while(*dest) dest++;
        while(*src) *dest++ = *src++;
        *dest = 0;
    }

    // Helper to extract filename from path
    static const char* basename(const char* path) {
        int len = strlen(path);
        if (len == 0) return path;

        // Find last slash
        for(int i=len-1; i>=0; i--) {
            if (path[i] == '/') return path + i + 1;
        }
        return path;
    }
}
#endif
