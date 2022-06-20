#include "System.hpp"
#include "util.hpp"

int main() {
    std::string data_path = "../data/aes_core.v.table";
    std::string folder_path = "../pre_data/aes_core.hierarchy.v.table/aes_core";
    auto seed = std::mt19937(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    auto rand_fun = [&](int a, int b) { return std::uniform_int_distribution<>(a, b - 1)(seed); };
    // time test
//    auto st = std::chrono::system_clock::now();
//    System sym;
//    sym.load(folder_path);
//    sym.store(folder_path);
//    auto en = std::chrono::system_clock::now();
//    std::cout << "time_cost: " << time_cost(st, en) << "(ms)\n";

    // mem
    System sym;
    auto mem1 = get_rmem(getpid());
    sym.load(folder_path);
    auto mem2 = get_rmem(getpid());
    std::cout << "build mem_size: ";
    std::cout << 1. * (mem2 - mem1) * 4 / 1024 << "(MB)\n";
//    std::cout << "normal mem_size: ";
//    std::cout << 1. * sym.size() / 1024 / 1024 << "(MB)\n";
    int64_t res = 0;
    for (int i = 0; i < sym.storage_.nodes_.size(); ++i) {
        auto item = sym.storage_.nodes_[i];
        res += sym.full_name_trie_.find_idx(item.full_name_).size() * sizeof(char);
        for (auto e : sym.node_attributes(i)) {
            res += sym.attribute_trie_.find_idx(e).size() * sizeof(char);
        }
    }
    for (int i = 0; i < sym.storage_.edges_.size(); ++i) {
        auto item = sym.storage_.edges_[i];
        res += sym.signal_trie_.find_idx(item.signal_name_).size() * sizeof(char);
        res += sym.full_name_trie_.find_idx(sym.node_ref(item.source_).full_name_).size() * sizeof(char);
        res += sym.full_name_trie_.find_idx(sym.node_ref(item.target_).full_name_).size() * sizeof(char);
        for (auto e : sym.edge_attributes(i)) {
            res += sym.attribute_trie_.find_idx(e).size() * sizeof(char);
        }
    }
    std::cout << "source data_size: ";
    std::cout << 1. * res / 1024 / 1024 << "(MB)\n";
    // file size
    std::cout << '\n';
    std::cout << "table file_size: ";
    std::cout << 1. * get_file_size(data_path) / 1024 / 1024 << "(MB)\n";
    long tf = 0;
    tf += get_file_size(folder_path + "/log.data");
    tf += get_file_size(folder_path + "/full_name.map");
    tf += get_file_size(folder_path + "/attribute.map");
    tf += get_file_size(folder_path + "/signal.map");
    tf += get_file_size(folder_path + "/storage.data");
    tf += get_file_size(folder_path + "/now.g");
    tf += get_file_size(folder_path + "/hid.g");
    std::cout << "disk file_size: ";
    std::cout << 1. * tf / 1024 / 1024 << "(MB)\n";

//    // query time
//    System sym;
//    sym.load(folder_path);
//    int fn_size = sym.full_name_trie.global_idx;
//    uint64_t avg_time = 0;
//    const int epech = 10;
//    for (int i = 0; i < epech; ++i) {
//        std::string fn = sym.full_name_trie.find_idx(rand_fun(0, fn_size));
//        auto st = std::chrono::system_clock::now();
//        sym.query_node(fn);
//        auto en = std::chrono::system_clock::now();
//        avg_time += time_cost_u(st, en);
//    }
//    std::cout << avg_time / epech << '\n';
//    int sg_size = sym.signal_trie.global_idx;
//    avg_time = 0;
//    for (int i = 0; i < epech; ++i) {
//        std::string sg = sym.signal_trie.find_idx(rand_fun(0, sg_size));
//        auto st = std::chrono::system_clock::now();
//        sym.query_edge(sg);
//        auto en = std::chrono::system_clock::now();
//        avg_time += time_cost_u(st, en);
//    }
//    std::cout << avg_time / epech << '\n';
//
//    // filter time
//    int attr_size = sym.attribute_trie.global_idx;
//    avg_time = 0;
//    for (int i = 0; i < epech; ++i) {
//        std::string attr = sym.attribute_trie.find_idx(rand_fun(0, attr_size));
//        auto st = std::chrono::system_clock::now();
//        sym.filter_node(attr);
//        auto en = std::chrono::system_clock::now();
//        avg_time += time_cost_u(st, en);
//    }
//    std::cout << avg_time / epech << '\n';
//    avg_time = 0;
//    for (int i = 0; i < epech; ++i) {
//        std::string attr = sym.attribute_trie.find_idx(rand_fun(0, attr_size));
//        auto st = std::chrono::system_clock::now();
//        sym.filter_edge(attr);
//        auto en = std::chrono::system_clock::now();
//        avg_time += time_cost_u(st, en);
//    }
//    std::cout << avg_time / epech << '\n';
}