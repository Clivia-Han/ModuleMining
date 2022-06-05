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

void MapToVec(map<string, CL_Partition* >& , vector<CL_Partition* >& );
string intToString(int a);
string doubleToString(double a);
int getPos(char* str, char c);
long long getmsofday();
char* getCmdOption(char ** begin, char ** end, const std::string & option);
bool cmdOptionExists(char** begin, char** end, const std::string& option);
int getNumElems(vector<map<string, void*>* >* );
void vect_map_destruct(vector<map<string, Pattern*>* > vm);
void vect_map_destruct(vector<CLMap* > vm);
long getTotalSystemMemory();
void process_mem_usage(double& vm_usage, double& resident_set);

#endif /* UTILS_H_ */
