#pragma once

#include "util.hpp"

#include "Storage.hpp"

class Graph {
public:
    bool store(const std::string &path) {
        std::ofstream out(path);
        if (out.fail()) {
            std::cout << "Error: open " << path << " failed!\n";
            return false;
        }
        out << node_id_set << '\n' << edge_id_set;
        out.close();
        return true;
    }

    void clear() {
        node_id_set.clear();
        edge_id_set.clear();
    }

    bool load(const std::string &path) {
        std::ifstream in(path);
        if (in.fail()) {
            std::cout << "Error: open " << path << " failed!\n";
            return false;
        }
        in >> node_id_set >> edge_id_set;
        in.close();
        return true;
    }

    uint64_t size() {
        uint64_t res = sizeof(Graph);
        res += node_id_set.size() * sizeof(decltype(node_id_set)::node_type);
        res += node_id_set.size() * sizeof(decltype(edge_id_set)::node_type);
        return res;
    }
// private:
    std::set<int> node_id_set;
    std::set<int> edge_id_set;
};