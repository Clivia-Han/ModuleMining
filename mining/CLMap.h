/**
 * a helper structure for subgraph isomorphism function
 */
#pragma once

#include "Pattern.h"
#include "core_file.h"

class CLMap_Iterator {
public:
    std::string key;
    Pattern *pattern;
    std::map<std::string, Pattern *>::iterator map_p_iter;
    std::map<std::string, std::list<Pattern *> *>::iterator map_v_iter;
    std::list<Pattern *>::iterator vect_iter;

    CLMap_Iterator get_copy();
};

class CLMap {
private:
    std::map<std::string, Pattern *> patterns_sig;
    std::map<std::string, std::list<Pattern *> *> patterns_nosig;

    Pattern *exists(Pattern *, std::list<Pattern *> *);

    bool remove(Pattern *, std::list<Pattern *> *);

    bool the_same(Pattern *, Pattern *);

    int size;

public:
    CLMap();

    bool add_pattern(Pattern *pattern);

    void remove_pattern(Pattern *pattern);

    void add_all(CLMap *clmap);

    Pattern *get_pattern(Pattern *pattern);

    int get_size() { return size; }

    void print(int counter = 1);

    CLMap_Iterator get_first_element();

    void advance_iterator(CLMap_Iterator &current_iter);

    void clear();

    void delete_objects();
};
void vect_map_destruct(std::vector<CLMap *> vm);