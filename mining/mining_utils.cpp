#include "core_file.h"
#include "mining_utils.h"

std::string int_to_string(int a) {
    std::stringstream ss;
    ss << a;
    return ss.str();
}

std::string double_to_string(double a) {
    std::stringstream ss;
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

long long get_msec() {
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return (long long) tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int get_num_elems(std::vector<std::map<std::string, void *> *> *a) {
    int counter = 0;
    for (int i = 0; i < a->size(); i++) {
        counter += a->at(i)->size();
    }
    return counter;
}
