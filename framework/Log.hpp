#pragma once

#include <utility>

#include "core.h"
#include "util.cpp"

enum {
    REPLACE = 0,
    SHINK
};
class Log {
public:
    uint64_t size() {
        uint64_t res = sizeof(Log);
        res += bef_.first.size() * sizeof(int);
        res += bef_.second.size() * sizeof(int);
        res += aft_.first.size() * sizeof(int);
        res += aft_.second.size() * sizeof(int);
        return res;
    }
// private:
    int type_;
    std::pair<std::vector<int>, std::vector<int>> bef_; // before nodes & edges
    std::pair<std::vector<int>, std::vector<int>> aft_; // after nodes & edges
};

std::ostream & operator << (std::ostream &out, Log &log) {
    out << log.type_ << ' ';
    out << log.bef_.first << ' ' << log.bef_.second << ' ';
    out << log.aft_.first << ' ' << log.aft_.second;
    return out;
}

std::istream & operator >> (std::istream &in, Log &log) {
    in >> log.type_;
    in >> log.bef_.first >> log.bef_.first;
    in >> log.aft_.first >> log.aft_.second;
    return in;
}

struct Logs {
    std::vector<Log> data_;

    bool load(const std::string& path) {
        std::ifstream in(path);
        if (in.fail()) {
            std::cout << "Error: open " << path << " failed!\n";
            return false;
        }
        in >> data_;
        in.close();
        return true;
    }

    void clear() {
        data_.clear();
    }

    bool store(const std::string& path) {
        std::ofstream out(path);
        if (out.fail()) {
            std::cout << "Error: open " << path << " failed!\n";
            return false;
        }
        out << data_;
        out.close();
        return true;
    }

    uint64_t size() {
        uint64_t res = 0;
        res += sizeof(Logs);
        for (auto &log : data_) {
            res += log.size();
        }
        return res;
    }
};