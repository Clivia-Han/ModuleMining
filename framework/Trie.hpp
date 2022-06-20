#pragma once

#include "core.h"

struct TrieNode {
    int idx;
    int fa_ptr;
    std::vector<std::pair<char, int>> childs;

    TrieNode(): idx(-1), fa_ptr(-1) {}
};

struct Trie {
    int global_idx = 0;
    std::vector<TrieNode> nodes;
    std::vector<int> idx_pos;

    Trie() {
        nodes.emplace_back();
    }

    bool store(const std::string &path) {
        std::ofstream outfile(path);
        if (outfile.fail()) {
            std::cerr << "Error: open " << path << " failed!\n";
            return false;
        }
        auto mapping = std::move(all_mapping());
        outfile << mapping;
        outfile.close();
        return true;
    }

    void clear() {
        global_idx = 0;
        nodes.clear();
        nodes.emplace_back();
        idx_pos.clear();
    }

    bool load(const std::string &path) {
        std::ifstream infile(path);
        if (infile.fail()) {
            std::cerr << "Error: open " << path << " failed!\n";
            return false;
        }
        std::map<int, std::string> mapping;
        infile >> mapping;
        for (auto &e : mapping) {
            auto [idx, _] = insert(e.second);
            assert(idx == e.first);
        }
        infile.close();
        return true;
    }

    std::pair<int, bool> insert(const std::string& str) {
        int ptr = 0; // root
        for (auto e : str) {
            bool flag = false;
            for (auto &item : nodes[ptr].childs) {
                if (item.first == e) {
                    ptr = item.second;
                    flag = true;
                    break;
                }
            }
            if (flag) continue;
            int nxt_ptr = (int)nodes.size();
            nodes[ptr].childs.emplace_back(e, nxt_ptr);
            nodes.emplace_back();
            nodes[nxt_ptr].fa_ptr = ptr;
            ptr = nxt_ptr;
        }
        if (nodes[ptr].idx != -1) {
            return std::make_pair(nodes[ptr].idx, true);
        } else {
            idx_pos.push_back(ptr);
            nodes[ptr].idx = global_idx++;
            return std::make_pair(nodes[ptr].idx, false);
        }
    }

    int find_str(const std::string& str) {
        int ptr = 0; // root
        for (auto e : str) {
            bool flag = false;
            for (auto &item : nodes[ptr].childs) {
                if (item.first == e) {
                    ptr = item.second;
                    flag = true;
                    break;
                }
            }
            if (!flag) return -1;
        }
        return nodes[ptr].idx;
    }

    std::string find_idx(int idx) {
        if (idx >= global_idx) return "-1";
        int fa_ptr, ptr = idx_pos[idx];
        std::string res;
        while (ptr != 0) {
            fa_ptr = nodes[ptr].fa_ptr;
            for (auto &child : nodes[fa_ptr].childs) {
                if (child.second == ptr) {
                    res += child.first;
                    break;
                }
            }
            ptr = fa_ptr;
        }
        std::reverse(res.begin(), res.end());
        return res;
    }

    [[nodiscard]] int node_size() const {
        return (int)nodes.size();
    }

    uint64_t size() {
        uint64_t res = sizeof(Trie);
        for (const auto& node : nodes) {
            res += sizeof(node);
            res += node.childs.size() * sizeof(std::pair<char, int>);
        }
        return res;
    }

    void print(std::ostream &out, bool sorted = true) {
        std::string stack;
        std::vector<std::pair<int, std::string>> res;
        std::function<void(int)> dfs = [&](int ptr) {
            if (nodes[ptr].idx != -1) {
                res.emplace_back(nodes[ptr].idx, stack);
            }
            for (auto &pci : nodes[ptr].childs) {
                stack.push_back(pci.first);
                dfs(pci.second);
                stack.pop_back();
            }
        };
        dfs(0);
        if (sorted) std::sort(res.begin(), res.end());
        for (auto &e : res) {
            out << e.first << ' ' << e.second << '\n';
        }
    }

    std::map<int, std::string> all_mapping() {
        std::string stack;
        std::map<int, std::string> res;
        std::function<void(int)> dfs = [&](int ptr) {
            if (nodes[ptr].idx != -1) {
                res[nodes[ptr].idx] = stack;
            }
            for (auto &pci : nodes[ptr].childs) {
                stack.push_back(pci.first);
                dfs(pci.second);
                stack.pop_back();
            }
        };
        dfs(0);
        return res;
    }
};
