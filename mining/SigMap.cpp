#include "SigMap.h"
#include "Settings.h"
#include "core_file.h"


// a SigMap means the set of candidates with corresponding size
SigMapIter SigMapIter::get_copy() {
    SigMapIter iter;

    iter.pattern = pattern;
    iter.key = key;
    iter.map_p_iter = map_p_iter;
    iter.map_v_iter = map_v_iter;
    iter.vec_iter = vec_iter;

    return iter;
}

SigMap::SigMap() {
    size = 0;
}

/**
 * add the pattern to the list of patterns
 * When the signature creation takes forever, use the list and compare using subgraph isomorphism
 */
bool SigMap::add_pattern(Pattern *pattern) {
    //ignore this pattern if its size exceeds the given limit
    if ((Settings::max_edges_num > -1 && pattern->get_graph()->get_edges_num() > Settings::max_edges_num) ||
        (Settings::max_nodes_num > -1 && pattern->get_graph()->get_nodes_num() > Settings::max_nodes_num))
        return false;

    std::string sig = pattern->get_graph()->get_sig();
    if (sig.at(0) == 'X') {
        std::list<Pattern *> *v;
        auto it = patterns_nosig.find(sig);
        if (it != patterns_nosig.end()) {
            v = it->second;
            if (exists(pattern, v))
                return false;
        } else {
            v = new std::list<Pattern *>();
            patterns_nosig.insert(std::pair<std::string, std::list<Pattern *> *>(sig, v));
        }

        v->push_back(pattern);
    } else {
        if (patterns_sig.find(sig) != patterns_sig.end())
            return false;

        patterns_sig.insert(std::pair<std::string, Pattern *>(sig, pattern));
    }

    size++;
    return true;
}

void SigMap::remove_pattern(Pattern *pattern) {
    std::string sig = pattern->get_graph()->get_sig();
    if (sig.at(0) == 'X') {
        std::list<Pattern *> *v;
        auto it = patterns_nosig.find(sig);
        if (it != patterns_nosig.end()) {
            v = it->second;
            if (remove(pattern, v)) {
                size--;
                if (v->size() == 0) {
                    patterns_nosig.erase(sig);
                }
            }
        }
    } else {
        int a = patterns_sig.erase(sig);
        size -= a;
    }
}

/**
 * search for the given pattern, if not exist then the returned value = 0
 */
Pattern *SigMap::get_pattern(Pattern *pattern) {
    std::string sig = pattern->get_graph()->get_sig();

    if (sig.at(0) == 'X') {
        auto it = patterns_nosig.find(sig);
        if (it == patterns_nosig.end()) {
            return 0;
        }

        return exists(pattern, it->second);
    } else {
        auto it = patterns_sig.find(sig);
        if (it == patterns_sig.end()) {
            return 0;
        } else {
            return it->second;
        }
    }
}

/**
 * search for the pattern in the given list, if found return a pointer to its similar pattern
 */
Pattern *SigMap::exists(Pattern *pattern, std::list<Pattern *> *v) {
    if (Settings::debug_msg) {
        std::cout << "SigMap::exists starts..." << std::endl;
    }

    for (auto temp : *v) {
        if (the_same(pattern, temp)) {
            if (Settings::debug_msg) {
                std::cout << "SigMap::exists finished, pattern: " << temp->get_id() << " found." << std::endl;
            }

            return temp;
        }
    }

    if (Settings::debug_msg) {
        std::cout << "SigMap::exists finished, no pattern found." << std::endl;
    }
    return 0;
}

/*
 * remove a pattern if found, otherwise no effect and return false
 * return true only if the pattern is removed
 */
bool SigMap::remove(Pattern *pattern, std::list<Pattern *> *v) {
    for (auto iter = v->begin(); iter != v->end(); iter++) {
        Pattern *temp = *iter;
        if (the_same(pattern, temp)) {
            v->erase(iter);
            return true;
        }
    }
    return false;
}

//check if they are the same by performing subgraph isomorphism
bool SigMap::the_same(Pattern *p1, Pattern *p2) {
    return p1->get_graph()->same_with(p2->get_graph());
}

void SigMap::add_all(SigMap *sig_map) {
    for (auto & iter : sig_map->patterns_sig) {
        patterns_sig.insert(std::pair<std::string, Pattern *>(iter.first, iter.second));
        size++;
    }

    for (auto & iter : sig_map->patterns_nosig) {
        auto *v = new std::list<Pattern *>();
        v->insert(v->begin(), iter.second->begin(), iter.second->end());
        patterns_nosig.insert(std::pair<std::string, std::list<Pattern *> *>(iter.first, v));
        size += v->size();
    }
}

SigMapIter SigMap::get_first_element() {
    SigMapIter iter;

    if (!patterns_sig.empty()) {
        iter.key = patterns_sig.begin()->first;
        iter.pattern = patterns_sig.begin()->second;
        iter.map_p_iter = patterns_sig.begin();
        return iter;
    } else if (!patterns_nosig.empty() && !patterns_nosig.begin()->second->empty()) {
        iter.map_p_iter = patterns_sig.end();

        iter.key = patterns_nosig.begin()->first;
        iter.map_v_iter = patterns_nosig.begin();
        iter.pattern = *(patterns_nosig.begin()->second->begin());
        iter.vec_iter = patterns_nosig.begin()->second->begin();
        return iter;
    }

    iter.pattern = 0;
    if (size > 0) {
        std::cout << "Error56333: size=" << size << " though I cannot find a first pattern." << std::endl;
        exit(0);
    }

    return iter;
}

void SigMap::advance_iterator(SigMapIter &current_iter) {
    bool b = false;

    //iterating over the map of sigs
    if (current_iter.map_p_iter != patterns_sig.end()) {
        current_iter.map_p_iter++;
        if (current_iter.map_p_iter != patterns_sig.end()) {
            current_iter.key = current_iter.map_p_iter->first;
            current_iter.pattern = current_iter.map_p_iter->second;
            return;
        }

        current_iter.map_v_iter = patterns_nosig.begin();
        b = true;
    }

    if (current_iter.map_v_iter == patterns_nosig.end()) {
        current_iter.pattern = 0;
        return;
    }

    if (b) {
        current_iter.vec_iter = current_iter.map_v_iter->second->begin();
    } else {
        current_iter.vec_iter++;
    }

    if (current_iter.vec_iter == current_iter.map_v_iter->second->end()) {
        current_iter.map_v_iter++;
        if (current_iter.map_v_iter == patterns_nosig.end()) {
            current_iter.pattern = 0;
            return;
        }
        current_iter.vec_iter = current_iter.map_v_iter->second->begin();
    }

    current_iter.key = current_iter.map_v_iter->first;
    current_iter.pattern = *(current_iter.vec_iter);
}

void SigMap::print(int counter) {
    SigMapIter iter = this->get_first_element();

    while (true) {
        if (iter.pattern == 0)
            break;

        std::cout << counter << ":" << std::endl;
        std::cout << *(iter.pattern->get_graph()) << std::endl;
        std::cout << "-----------" << std::endl;
        this->advance_iterator(iter);
        counter++;
    }
}

void SigMap::clear() {
    patterns_sig.clear();
    for (auto & iter : patterns_nosig) {
        delete iter.second;
    }
    patterns_nosig.clear();
    //size = 0;
}

void SigMap::delete_objects() {
    SigMapIter iter = this->get_first_element();
    while (iter.pattern != 0) {
        Pattern *pattern = iter.pattern;
        delete pattern;
        this->advance_iterator(iter);
    }
}

void vect_map_destruct(std::vector<SigMap *> vm) {
    for (auto & iter1 : vm) {
        iter1->delete_objects();
        iter1->clear();
        delete iter1;
    }
}
