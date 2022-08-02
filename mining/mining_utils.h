#pragma once

#include<map>
#include<vector>
#include<string>
#include<sys/time.h>


std::string int_to_string(int a);

std::string double_to_string(double a);

int get_pos(char *str, char c);

long long get_msec();

int get_num_elems(std::vector<std::map<std::string, void *> *> *a);
