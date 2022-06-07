#ifndef UTILS_H_
#define UTILS_H_

#include<map>
#include<vector>
#include<string>
#include<sys/time.h>
#include "Pattern.h"
#include"CanonicalLabel.h"
#include "CLMap.h"

using namespace std;

void map_to_vec(map<string, CL_Partition *> &m, vector<CL_Partition *> &v);

string int_to_string(int a);

string double_to_string(double a);

int get_pos(char *str, char c);

long long get_ms_of_day();

int get_num_elems(vector<map<string, void *> *> *a);

void vect_map_destruct(vector<map<string, Pattern *> *> vm);

void vect_map_destruct(vector<CLMap *> vm);

#endif /* UTILS_H_ */
