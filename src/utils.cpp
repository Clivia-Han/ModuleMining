#include <fstream>
#include<sstream>
#include <string.h>
#include <algorithm>
#include <unistd.h>
#include"utils.h"

/**
 * A function to copy map content into a vector
 */
void map_to_vec(map<string, CL_Partition *> &m, vector<CL_Partition *> &v) {
    for (map<string, CL_Partition *>::const_iterator it = m.begin(); it != m.end(); ++it) {
        v.push_back(it->second);
    }
}

string int_to_string(int a) {
    stringstream ss;
    ss << a;
    return ss.str();
}

string double_to_string(double a) {
    stringstream ss;
    ss << a;
    return ss.str();
}

int get_pos(char *str, char c) {
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] == c)
            return i;
    }
    return -1;
}

//get time in milli seconds
long long get_ms_of_day() {
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return (long long) tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int get_num_elems(vector<map<string, void *> *> *a) {
    int counter = 0;
    for (int i = 0; i < a->size(); i++) {
        counter += a->at(i)->size();
    }
    return counter;
}

void vect_map_destruct(vector<map<string, Pattern *> *> vm) {
    for (vector<map<string, Pattern *> *>::iterator iter1 = vm.begin(); iter1 != vm.end(); ++iter1) {
        for (map<string, Pattern *>::iterator iter2 = (*iter1)->begin(); iter2 != (*iter1)->end(); iter2++) {
            delete (iter2->second);

        }

        (*iter1)->clear();
        delete (*iter1);

    }
}

void vect_map_destruct(vector<CLMap *> vm) {
    for (vector<CLMap *>::iterator iter1 = vm.begin(); iter1 != vm.end(); ++iter1) {
        (*iter1)->delete_objects();
        (*iter1)->clear();
        delete (*iter1);
    }
}
