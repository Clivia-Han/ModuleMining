/**
 * a helper structure for subgraph isomorphism function
 */
#pragma once

#include "Pattern.h"
#include "core_file.h"

class SigMapIter {
public:
    std::string key;
    Pattern *pattern;
    std::map<std::string, Pattern *>::iterator map_p_iter;
    std::map<std::string, std::list<Pattern *> *>::iterator map_v_iter;
    std::list<Pattern *>::iterator vec_iter;

    SigMapIter get_copy();
};

class SigMap {
private:
    std::map<std::string, Pattern *> patterns_sig;
    std::map<std::string, std::list<Pattern *> *> patterns_nosig;

    Pattern *exists(Pattern *, std::list<Pattern *> *);

    bool remove(Pattern *, std::list<Pattern *> *);

    bool the_same(Pattern *, Pattern *);

    int size;

public:
    SigMap();

    bool add_pattern(Pattern *pattern);

    void remove_pattern(Pattern *pattern);

    void add_all(SigMap *sig_map);

    Pattern *get_pattern(Pattern *pattern);

    int get_size() { return size; }

    void print(int counter = 1);

    SigMapIter get_first_element();

    void advance_iterator(SigMapIter &current_iter);

    void clear();

    void delete_objects();
};

void vect_map_destruct(std::vector<SigMap *> vm);