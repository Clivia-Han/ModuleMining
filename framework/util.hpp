#pragma once

#include "core.h"

class string_view {
public:
    string_view() = default;

    string_view(string_view &&) = default;
    string_view &operator=(string_view &&) = default;

    string_view(const string_view &) = default;
    string_view &operator=(const string_view &) = default;

    ~string_view() = default;

    string_view(std::string &s): string_view(s.data(), s.size()) {}

    size_t size() const { return size_; }
    const char *data() const { return data_; }

    const char *begin() { return data_; }
    const char *end() { return data_ + size_; }
    string_view substr(size_t pos, size_t n) { return {data_ + pos, n}; }

    operator std::string () const {
        return {data_, size_};
    }

    size_t find_first_of(char c, size_t pos) const {
        return std::find(data_ + pos, data_ + size_, c) - data_;
    }

    char at(size_t pos) const {
        return data_[pos];
    }

    uint32_t parse() {
        char *_;
        return std::strtoul(data_, &_, 10);
    }

    friend bool operator<(string_view lhs, const std::string &rhs) {
        return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    friend bool operator<(const std::string &lhs, string_view rhs) {
        return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }
    friend bool operator==(string_view lhs, const std::string &rhs) {
        return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }
    friend bool operator==(const std::string &lhs, string_view rhs) {
        return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }
    friend bool operator!=(string_view lhs, const std::string &rhs) {
        return lhs.size() != rhs.size() || !std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }
    friend bool operator!=(const std::string &lhs, string_view rhs) {
        return lhs.size() != rhs.size() || !std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }

private:
    string_view(const char *data, size_t size): data_{data}, size_{size} {}

    const char *data_;
    size_t size_;
};
std::vector<string_view> split(string_view s, char dim);

struct Iterator { 
    int iter; 
    friend int operator*(const Iterator &self) { return self.iter; }
    friend Iterator &operator++(Iterator &self) { return self.iter++, self; }
    friend bool operator != (const Iterator &lhs, const Iterator &rhs) { return lhs.iter != rhs.iter; }
};

struct Range { 
    Iterator begin() { return {0}; } 
    Iterator end() { return {end_}; } 
    int end_{}; 
};
Range range(int end);

long get_file_size(const std::string& path);

template<class T>
void unique(std::vector<T> &v);

int64_t time_cost(const std::chrono::system_clock::time_point &st, const std::chrono::system_clock::time_point &en);

int64_t time_cost_u(const std::chrono::system_clock::time_point &st, const std::chrono::system_clock::time_point &en);

int get_rmem(int p);

template<class T>
void unique(std::vector<T> &v) {
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
}

template<class T>
std::ostream & operator << (std::ostream &out, std::vector<T> &vec) {
    out << vec.size();
    for (int i = 0; i < vec.size(); ++i) {
        out << ' ' << vec[i];
    }
    return out;
}

template<class T>
std::istream & operator >> (std::istream &in, std::vector<T> &vec) {
    int sz;
    in >> sz;
    vec.resize(sz);
    for (auto &e : vec) in >> e;
    return in;
}

template<class T>
std::ostream & operator << (std::ostream &out, std::set<T> &st) {
    out << st.size();
    for (auto iter = st.begin(); iter != st.end(); ++iter) {
        out << ' ' << *iter;
    }
    return out;
}

template<class T>
std::istream & operator >> (std::istream &in, std::set<T> &st) {
    int sz;
    in >> sz;
    T e;
    for (int i = 0; i < sz; ++i) {
        in >> e;
        st.insert(e);
    }
    return in;
}

template<class Key, class Val>
std::ostream & operator << (std::ostream &out, std::map<Key, Val> &mp) {
    out << mp.size();
    for (auto iter = mp.begin(); iter != mp.end(); ++iter) {
        out << ' ' << iter->first << ' ' << iter->second;
    }
    return out;
}

template<class Key, class Val>
std::istream & operator >> (std::istream &in, std::map<Key, Val> &mp) {
    int sz;
    in >> sz;
    Key k;
    Val v;
    for (int i = 0; i < sz; ++i) {
        in >> k >> v;
        mp[k] = v;
    }
    return in;
}