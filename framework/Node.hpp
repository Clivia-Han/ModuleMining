#pragma once
#include <utility>

#include "core.h"
#include "util.hpp"

// cell
class Node {
public:
    [[nodiscard]] uint64_t size() const {
        uint64_t res = 0;
        res += sizeof(Node);
        res += sizeof(int) * attributes_.size();
        return res;
    }

    friend std::ostream & operator << (std::ostream &out, Node &node) {
        out << node.full_name_ << ' ' << node.type_ << ' ';
        out << node.attributes_;
        return out;
    }

    friend std::istream & operator >> (std::istream &in, Node &node) {
        in >> node.full_name_ >> node.type_;
        in >> node.attributes_;
        return in;
    }

// private:
    int full_name_;
    int type_;
    std::vector<int> attributes_;
};