/**
 * a helper structure for subgraph isomorphism function
 */
#ifndef CLMAP_H_
#define CLMAP_H_

#include <map>
#include<list>
#include <string.h>
#include "Pattern.h"

using namespace std;

class CLMap_Iterator {
public:
    string key;
    Pattern *pattern;
    map<string, Pattern *>::iterator map_p_iter;
    map<string, list<Pattern *> *>::iterator map_v_iter;
    list<Pattern *>::iterator vect_iter;

    CLMap_Iterator get_copy();
};

class CLMap {
private:
    map<string, Pattern *> patterns_sig;
    map<string, list<Pattern *> *> patterns_nosig;

    Pattern *exists(Pattern *, list<Pattern *> *);

    bool remove(Pattern *, list<Pattern *> *);

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


#endif /* CLMAP_H_ */
