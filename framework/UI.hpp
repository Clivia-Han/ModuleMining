#pragma once

#include "System.hpp"

typedef uint64_t hash_t;
constexpr hash_t prime = 0x100000001B3ull;
constexpr hash_t basis = 0xCBF29CE484222325ull;
hash_t hash_(char const *str) {
    hash_t ret(basis);
    while (*str) {
        ret ^= *str;
        ret *= prime;
        str++;
    }
    return ret;
}
constexpr hash_t hash_compile_time(char const *str, hash_t last_value = basis) {
    return *str ? hash_compile_time(str + 1, (*str ^ last_value) * prime) : last_value;
}
constexpr hash_t operator ""_hash(char const* p, size_t) {
    return hash_compile_time(p);
}

struct UI {
    System sym;

    void print_cmd_lsit() {
        std::cout << "cmd list: \n";
        std::cout << std::setw(28) << std::setiosflags(std::ios::left)
                << "--help" << ": print cmd list\n";
        std::cout << std::setw(28) << std::setiosflags(std::ios::left)
                << "-l <folder_path>" << ": load data\n";
        std::cout << std::setw(28) << std::setiosflags(std::ios::left)
                << "-x <file_path>" << ": save for networkx\n";
        std::cout << std::setw(28) << std::setiosflags(std::ios::left)
                << "-s <folder_path>" << ": store data\n";
        std::cout << std::setw(28) << std::setiosflags(std::ios::left)
                << "-q <folder_path>" << ": quit and store data\n";
        // TODO:
        std::cout << std::setw(28) << std::setiosflags(std::ios::left)
                << "-f <string>" << ": filter attribute\n";
        std::cout << std::setw(28) << std::setiosflags(std::ios::left)
                << "-u <string>" << ": query attribute\n";
        std::cout << std::setw(28) << std::setiosflags(std::ios::left)
                << "-r <file_path1> <file_path2>" << ": replace\n";
        std::cout << std::setw(28) << std::setiosflags(std::ios::left)
                  << "-rollback" << ": roll back one replace operation through log\n";
        std::cout << '\n';
    }

    void welcome() {
        print_cmd_lsit();
        bool quit_tag = false;
        while (true) {
            std::cout << "please input your cmd:";
            std::string cmd, path, path2;
            std::cin >> cmd;
            switch (hash_(cmd.c_str())) {
                case "--help"_hash:
                    print_cmd_lsit();
                    break;
                case "-s"_hash:
                    std::cin >> path;
                    if (sym.store(path)) {
                        std::cout << "store success\n";
                    } else {
                        std::cout << "Error: store failed\n";
                    }
                    break;
                case "-l"_hash:
                    std::cin >> path;
                    if (sym.load(path)) {
                        std::cout << "load success\n";
                    } else {
                        std::cout << "Error: load failed\n";
                    }
                    break;
                case "-x"_hash:
                    std::cin >> path;
                    if (sym.write_networkX(path)) {
                        std::cout << "save for networkx success\n";
                    } else {
                        std::cout << "Error: save for networkx failed\n";
                    }
                    break;
                case "-q"_hash:
                    std::cin >> path;
                    if (sym.store(path)) {
                        std::cout << "quit success\n";
                        quit_tag = true;
                    } else {
                        std::cout << "Error: quit failed\n";
                    }
                    break;
                case "-r"_hash:
                    std::cin >> path >> path2;
                    // TODO:
                    break;
                case "-f"_hash:
                    std::cin >> path;
                    // TODO:
                    break;
                case "-u"_hash:
                    std::cin >> path;
                    // TODO:
                    break;
                case "--rollback"_hash:
                    break;
                default:
                    std::cout << "Warning: wrong cmd!\n";
            }
            if (quit_tag) {
                break;
            }
            std::cout << '\n';
        }
    }
};

