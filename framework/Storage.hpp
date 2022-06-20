#pragma once

#include "util.hpp"
#include "Node.hpp"
#include "Edge.hpp"

class Storage{
public:
    std::vector<int> node_attributes(int node_id) {
        auto res = nodes_[node_id].attributes_;
        res.emplace_back(nodes_[node_id].type_);
        return res;
    }

    std::vector<int> edge_attributes(int edge_id) {
        auto res = edges_[edge_id].attributes_;
        res.emplace_back(edges_[edge_id].source_port_);
        res.emplace_back(edges_[edge_id].target_port_);
        res.emplace_back(nodes_[edges_[edge_id].source_].type_);
        res.emplace_back(nodes_[edges_[edge_id].target_].type_);
        return res;
    }

    bool store(const std::string &path) {
        std::ofstream out(path);
        if (out.fail()) {
            std::cout << "Error: open " << path << " failed!\n";
            return false;
        }
        out << nodes_ << '\n' << edges_;
        out.close();
        return true;
    }

    void clear() {
        nodes_.clear();
        edges_.clear();
    }

    bool load(const std::string &path) {
        std::ifstream in(path);
        if (in.fail()) {
            std::cout << "Error: open " << path << " failed!\n";
            return false;
        }
        in >> nodes_ >> edges_;
        in.close();
        return true;
    }

    uint64_t size() {
        uint64_t res = sizeof(Storage);
        for (const auto& e : nodes_) res += e.size();
        for (const auto& e : edges_) res += e.size();
        return res;
    }
// private:
    std::vector<Node> nodes_;
    std::vector<Edge> edges_;
};