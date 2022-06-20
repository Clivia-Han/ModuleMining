#include "util.hpp"

std::vector<string_view> split(string_view s, char dim) {
    std::vector<string_view> views;
    for (size_t i = 0, j = 0; i < s.size(); i = ++j) {
        j = s.find_first_of(dim, j);
        if (i == j) continue;
        views.push_back(s.substr(i, j - i));
    }
    return views;
}

Range range(int end) { return {end}; }

long get_file_size(const std::string& path) {
    std::ifstream fin(path.c_str(), std::ios::in | std::ios::binary);
    fin.seekg(0, std::ios_base::end);
    std::streampos pos = fin.tellg();
    long sz = static_cast<long>(pos);
    fin.close();
    return sz;
}

int64_t time_cost(const std::chrono::system_clock::time_point &st, const std::chrono::system_clock::time_point &en) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(en - st).count();
}

int64_t time_cost_u(const std::chrono::system_clock::time_point &st, const std::chrono::system_clock::time_point &en) {
    return std::chrono::duration_cast<std::chrono::microseconds>(en - st).count();
}

int get_rmem(int p) {
    FILE *fd;
	int vmrss;
    char file[64] = {0};  
	char line_buff[256] = {0};
	char rdun1[32];
	char rdun2[32];
	
    sprintf(file, "/proc/%d/statm", p);
    fd = fopen (file, "r");
	char* ret = fgets(line_buff, sizeof(line_buff), fd);	
	sscanf(line_buff, "%s %d %s", rdun1, &vmrss, rdun2);
    fclose(fd);
	
    return vmrss;
}