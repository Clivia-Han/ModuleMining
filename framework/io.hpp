#pragma once
#include "core.h"

struct BufferedReader {
    explicit BufferedReader(const char* const file_name) noexcept
            : fd(open(file_name, O_RDONLY)), iter(0), length(size(fd))
            , buffer((char*)mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0)) {
    }

    static inline int size(int fd) {
        FILE* file = fdopen(fd, "r");
        fseek(file, 0, SEEK_END);
        return (int)ftell(file);
    }

    bool has_next() {
        while (iter < length && (buffer[iter] == ' ' || buffer[iter] == '\n' || buffer[iter] == '\r')) {
            ++iter;
        }
        return iter != length;
    }

    uint32_t next_int() {
        uint32_t number = 0;
        char ch = get();
        while (!isdigit(ch)) ch = get();
        for (; isdigit(ch); ch = get())
            number = number * 10 + ch - '0';
        return number;
    }

    std::string next_str() {
        std::string str;
        char ch = get();
        while (ch == ' ' || ch == '\n' || ch == '\r') ch = get();
        for (; ch != EOF && ch != ' ' && ch != '\n' && ch != '\r'; ch = get()) {
            str += ch;
        }
        return str;
    }

    void close() {
        ::close(fd);
    }

    char get() {
        return iter == length ? EOF : buffer[iter++];
    }

    int fd, iter, length;
    char * buffer;
};