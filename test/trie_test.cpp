#include "Trie.hpp"

int main() {
    Trie trie;
    trie.insert("r4_/2");
    trie.insert("r4_/1");
    trie.insert("r4_/3");
    std::cout << trie.find_str("r4_/1") << '\n';
    std::cout << trie.find_str("r233") << '\n';
    std::cout << trie.find_idx(0) << '\n';
    trie.print(std::cout, false);
}