#pragma once

#include "core.h"
#include "util.hpp"

#include "Node.hpp"

// signal
class Edge {
public:
    [[nodiscard]] uint64_t size() const {
        uint64_t res = 0;
        res += sizeof(Edge);
        res += sizeof(int) * attributes_.size();
        return res;
    }

    friend std::ostream & operator << (std::ostream &out, Edge &edge) {
        out << edge.signal_name_ << ' ';
        out << edge.source_ << ' ' << edge.source_port_ << ' ';
        out << edge.target_ << ' ' << edge.target_port_ << ' ';
        out << edge.attributes_;
        return out;
    }

    friend std::istream & operator >> (std::istream &in, Edge &edge) {
        in >> edge.signal_name_;
        in >> edge.source_ >> edge.source_port_;
        in >> edge.target_ >> edge.target_port_;
        in >> edge.attributes_;
        return in;
    }

// private:
    int signal_name_;
    int source_, source_port_; // node id
    int target_, target_port_;
    std::vector<int> attributes_; // include cell port
};