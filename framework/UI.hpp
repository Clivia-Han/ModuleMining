#pragma once

#include "Miner.h"
#include "Settings.h"
#include "System.hpp"
#include "core_file.h"
#include "mining_utils.h"

int Settings::support = 100;
int Settings::max_edges_num = -1;
int Settings::max_nodes_num = -1;
bool Settings::debug_msg = false;
int Settings::given_seed_node_id = -1;  //user-given seed node id
bool Settings::throw_nodes_after_iterations = false;
long Settings::postpone_nodes_after_iterations = 10000000;
std::string Settings::store_path = "../output_data";
std::string Settings::file_name = "../pre_data/aes_core.v.table";

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
        std::cout << std::setw(28) << std::setiosflags(std::ios::left)
                  << "-m <store_path> \n"
                  << "   <file_name> \n"
                  << "   <support> \n"
                  << "   <max_edges_num> \n"
                  << "   <max_nodes_num> \n"
                  << "   <given_seed_node_id>\n"
                  << "   <throw_nodes_after_iterations>\n"
                  << "   <postpone_nodes_after_iterations>\n"
                  << "                                :mining frequent module";
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
            std::string comm_line;
            std::vector<string_view> args;
            std::cin >> cmd;

            Miner *miner = new Miner();
            long long elapsed = 0;
            long long start_time;
            long long end_time;

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
                case "-m"_hash:
                    std::getline(std::cin, comm_line);
                    args = split(string_view(comm_line), ' ');
                    for (int i = 0; i < args.size(); i++) {
                        if (i == 0) {
                            Settings::store_path = args[i];
                        } else if (i == 1) {
                            Settings::file_name = args[i];
                        } else if (i == 2) {
                            Settings::support = stoi(std::string(args[i]));
                        } else if (i == 3) {
                            Settings::max_edges_num = stoi(std::string(args[i]));
                        } else if (i == 4) {
                            Settings::max_nodes_num = stoi(std::string(args[i]));
                        } else if (i == 5) {
                            Settings::given_seed_node_id = stoi(std::string(args[i]));
                        } else if (i == 6) {
                            Settings::throw_nodes_after_iterations = std::string(args[i]) == "true";;
                        } else if (i == 7) {
                            Settings::postpone_nodes_after_iterations = stoi(std::string(args[i]));
                        }
                    }
                    sym.init_unpre(Settings::file_name);
                    sym.store(Settings::store_path);
                    std::cout << "Settings::store_path " << Settings::store_path << std::endl;
                    std::cout << "Settings::file_name " << Settings::file_name << std::endl;
                    std::cout << "Settings::support " << Settings::support << std::endl;
                    std::cout << "Settings::max_edges_num " << Settings::max_edges_num << std::endl;
                    std::cout << "Settings::max_nodes_num " << Settings::max_nodes_num << std::endl;
                    std::cout << "Settings::given_seed_node_id " << Settings::given_seed_node_id << std::endl;
                    std::cout << "Settings::throw_nodes_after_iterations " << Settings::throw_nodes_after_iterations << std::endl;std::cout << "Settings::throw_nodes_after_iterations " << Settings::throw_nodes_after_iterations << std::endl;
                    std::cout << "Settings::postpone_nodes_after_iterations " << Settings::postpone_nodes_after_iterations << std::endl;
                    std::cout << "now start mining process" << std::endl;
                    start_time = get_msec();
                    miner->start_mining_module(sym, Settings::support, Settings::given_seed_node_id, Settings::store_path);
                    end_time = get_msec();
                    elapsed = start_time - end_time;
                    std::cout << "Mining took " << (elapsed / 1000) << " sec and " << (elapsed % 1000) << " ms" << std::endl;
                    std::cout << "Finished!";
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
                    break;
                case "-f"_hash:
                    std::cin >> path;
                    break;
                case "-u"_hash:
                    std::cin >> path;
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
