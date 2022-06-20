#include <bits/stdc++.h>
#include "System.hpp"

// build <.topo> for hierarchy structure
bool creat_topo(const std::string &data, const std::string &out_folder) {
    std::ifstream infile("../data/" + data);
    if (infile.fail()) {
        std::cerr << "open data failed!\n";
        return false;
    }
    std::string line;
    std::string module_name, cell_name, cell_type;
    std::map<std::string, int> module_map;
    std::vector<std::string> module_index;
    std::vector<std::vector<int>> adj;
    std::vector<int> in_degree;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        iss >> module_name >> cell_name >> cell_type;
        if (cell_type == "-") continue;
        auto solve = [&](std::string name) {
            if (module_map.count(name)) return;
            int sz = module_map.size();
            module_map[name] = sz;
            module_index.push_back(name);
            adj.emplace_back();
            in_degree.push_back(0);
        };
        solve(module_name);
        solve(cell_type);
        int to = module_map[module_name], from = module_map[cell_type];
        if (std::find(adj[from].begin(), adj[from].end(), to) == adj[from].end()) {
            adj[from].push_back(to);
            in_degree[to]++;
        }
    }
    infile.close();
    std::queue<int> queue;
    for (int i : range(module_map.size())) {
        if (in_degree[i] == 0) queue.push(i);
    }
    std::vector<int> topo_order, back_in = in_degree;
    while (!queue.empty()) {
        int u = queue.front();
        topo_order.push_back(u);
        queue.pop();
        for (auto e : adj[u]) {
            back_in[e]--;
            if (back_in[e] == 0) queue.push(e);
        }
    }
    if (topo_order.size() != module_map.size()) {
        std::cerr << "can't find a topo oder!\n";
        return false;
    }
    std::ofstream outfile(out_folder + "/.topo");
    topo_order.erase(remove_if(topo_order.begin(), topo_order.end(), [&](int x) {return in_degree[x] == 0;}), topo_order.end());
    // revrse topo order
    // for (int i = topo_order.size() - 1; i >= 0; --i) {
    //     outfile << module_index[topo_order[i]] << "\n";
    // }
    for (auto e : topo_order) {
        outfile << module_index[e] << '\n';
    }
    outfile.close();
    return true;
}

// divide one table file to multi file
// update structure, include:
// 1. remove module_name from every line
// 2. add module_name in the first line
// 3. update dir
// 4. update signal: (1)remove tab && { && }; (2) split \xxx[X:Y]
bool divide_table(const std::string &data, const std::string &out_folder) {
    std::ofstream outfile;
    std::string line;
    std::string module_name, cell_name, cell_type, cell_port, dir, signal, pre_module_name;
    std::ifstream infile("../data/" + data);
    if (infile.fail()) {
        std::cerr << "open data failed!\n";
        return false;
    }
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        iss >> module_name >> cell_name >> cell_type >> cell_port >> dir;
        std::getline(iss, signal);
        if (pre_module_name.empty() || pre_module_name != module_name) {
            outfile.close();
            outfile.open(out_folder + "/" + module_name + ".table");
            outfile << module_name << '\n';
        }
        if (dir == "pi" || dir == "po") {
            // do nothing
        } else if (dir == "out" || (cell_port[0] >= 'X' && cell_port[0] <= 'Z') || cell_port[0] == 'Q') {
            dir = "o";
        } else {
            dir = "i";
        }
        signal.erase(remove_if(signal.begin(), signal.end(), [](char c) {return c == '{' || c == '}';}), signal.end());
        while (!signal.empty() && (signal.front() == ' ' || signal.front() == 9)) signal.erase(signal.begin());
        while (!signal.empty() && signal.back() == ' ') signal.pop_back();
        signal = std::regex_replace(signal, std::regex("\\s\\["), "[");
        auto raw_signal = split(signal, ' ');
        // std::vector<std::string> signals{raw_signal.begin(), raw_signal.end()};
        outfile << cell_name << ' ' << cell_type << ' ' << cell_port << ' ' << dir << ' ';
        for (auto i : range(raw_signal.size())) {
            auto &view = raw_signal[i];
            auto p2 = view.find_first_of(':', 0);
            if (p2 == view.size()) {
                outfile << std::string(view);
            } else {
                // split \xxx[X:Y] to {\xxx[Y], \xxx[Y+1], ... , \xxx[X]}
                auto p1 = view.find_first_of('[', 0), p3 = view.find_first_of(']', p2);
                int to = view.substr(p1 + 1, p2 - p1 - 1).parse();
                int from = view.substr(p2 + 1, p3 - p2 - 1).parse();
                auto pre = view.substr(0, p1);
                for (int j = from; j <= to; ++j) {
                    if (j != from) outfile << ' ';
                    outfile << std::string(pre) << '[' << std::to_string(j) << ']';
                }
            }
            outfile << " \n"[i + 1 == raw_signal.size()];
        }
        pre_module_name = module_name;
    }
    outfile.close();
    return true;
}

// update primary input&output's signal to multi 
bool update_pipo(const std::string &out_folder) {
    std::ifstream infile;
    std::ofstream outfile;
    infile.open(out_folder + "/.topo");
    std::vector<std::string> module_names;
    std::string line, cell_name, cell_type, cell_port, dir, signal;
    while (std::getline(infile, line)) {
        module_names.push_back(line);
    }
    infile.close();
    for (auto &module_name : module_names) {
        std::filesystem::path p1(out_folder + "/" + module_name + ".table");
        std::filesystem::path p2(out_folder + "/" + module_name + ".table.b");
        std::filesystem::rename(p1, p2);
        infile.open(p2);
        std::getline(infile, line);
        std::vector<std::pair<std::string, std::vector<std::string>>> pios;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            iss >> cell_name >> cell_type >> cell_port >> dir;
            iss.ignore(); // ignore one space for signal
            std::getline(iss, signal);
            if (cell_type == "-") {
                pios.emplace_back(signal, std::vector<std::string>());
                continue;
            }
            auto raw_signal = split(signal, ' ');
            for (auto &sig_view : raw_signal) {
                for (auto &[str, vec] : pios) {
                    if (str == sig_view) {
                        vec.push_back(sig_view);
                    } else if (str.size() < sig_view.size() && sig_view.at(str.size()) == '[' 
                            && str == sig_view.substr(0, str.size())) {
                        vec.push_back(sig_view);
                    }
                }
            }
        }
        for (auto &[_, vec] : pios) {
            unique(vec);
            if (vec.size() == 1) continue;
            std::sort(vec.begin(), vec.end(), [](auto &lhs, auto &rhs) {
                string_view lv(lhs), rv(rhs);
                auto s = lv.find_first_of('[', 0);
                auto t1 = lv.find_first_of(']', s), t2 = rv.find_first_of(']', s);
                return lv.substr(s + 1, t1 - s - 1).parse() < rv.substr(s + 1, t2 - s - 1).parse();
            });
        }
        infile.close();
        infile.open(p2);
        outfile.open(p1);
        getline(infile, line);
        outfile << line << '\n';
        int ptr = 0;
        while (getline(infile, line)) {
            std::istringstream iss(line);
            iss >> cell_name >> cell_type;
            if (cell_type != "-") {
                outfile << line << '\n';
                continue;
            }
            iss >> cell_port >> dir;
            outfile << cell_name << ' ' << cell_type << ' ' << cell_port << ' ' << dir;
            for (auto i : range(pios[ptr].second.size())) {
                outfile << ' ' << pios[ptr].second[i];
            }
            outfile << '\n';
            ptr++;
        }
        std::filesystem::remove(p2);
        infile.close();
        outfile.close();
    }
    return true;
}

bool parse(const std::string &out_folder, bool parallel = true) {
    std::cout << "start hierarchy replace...\n";
    std::ifstream infile;
    infile.open(out_folder + "/.topo");
    std::vector<std::string> module_names;
    std::string line;
    while (std::getline(infile, line)) {
        module_names.push_back(line);
    }
    infile.close();
    std::vector<System> syms(module_names.size());
    // init
    if (parallel) {
        int THREAD_SIZE = std::min<int>(module_names.size(), std::thread::hardware_concurrency());
        std::vector<std::thread> threads(THREAD_SIZE);
        std::atomic_int p(0);
        auto get_job = [&]() {
            return std::atomic_fetch_add_explicit(&p, 1, std::memory_order_relaxed);
        };
        auto run = [&]() {
            for (int x; (x = get_job()) < module_names.size();) {
                syms[x].init(out_folder + "/" + module_names[x] + ".table");
            }
        };
        for (auto i : range(THREAD_SIZE))
            threads[i] = std::thread(run);
        for (auto &thread : threads)
            thread.join();
    } else {
        for (auto i : range(module_names.size())) {
            const auto &module_name = module_names[i];
            syms[i].init(out_folder + "/" + module_name + ".table");
        }
    }
    // replace
    for (auto &sym : syms) {
        std::cout << "  now module: " << sym.module_name_ << '\n';
        while (true) {
            bool flag = false;
            for (auto node_id : sym.now_graph_.node_id_set) {
                const auto node_type = sym.attribute_trie_.find_idx(sym.node_ref(node_id).type_);
                auto which = [&](std::string match_str) {
                for (auto i : range(module_names.size())) {
                    if (match_str == module_names[i]) {
                        return i;
                    }
                }
                return -1;
                }(node_type);
                if (which == -1) continue;
                
                if (sym.replace(node_id, syms[which])) {
                    std::cout << "    replace node_id:" << node_id << " with module:" << module_names[which] << " success\n";
                } else {
                    std::cout << "    Error: replace node_id:" << node_id << " with module:" << module_names[which] << " failed!\n";
                    return false;
                }
                flag = true;
                break;
            }
            if (!flag) break;
        }
    }
    for (auto i : range(module_names.size())) {
        const auto &module_name = module_names[i];
        std::string folder_path(out_folder + "/" + module_name);
        // create folder
        std::filesystem::path p(folder_path);
        if (std::filesystem::exists(p)) std::filesystem::remove_all(p);
        std::filesystem::create_directories(p);
        // store data
        syms[i].store(folder_path);
    }
    return true;
}

int main(int argc, char **argv) {
    std::string data = argc < 2 ? "aes_core.hierarchy.v.table" : argv[1];
    bool parallel = argc < 3 ? false : true;
    if (parallel) {
        std::cout << "parallel pre-run\n";
    }
    std::string out_folder = "../pre_data/" + data;

    // create folder
    std::filesystem::path p(out_folder);
    if (std::filesystem::exists(p)) std::filesystem::remove_all(p);
    std::filesystem::create_directories(p);
    
    if (creat_topo(data, out_folder)) {
        std::cout << "create topo success\n";
    } else {
        std::cerr << "Error: create topo failed!\n";
        exit(EXIT_FAILURE);
    }

    if (divide_table(data, out_folder)) {
        std::cout << "divide table success\n";
    } else {
        std::cerr << "Error: divide table failed!\n";
        exit(EXIT_FAILURE);
    }

    if (update_pipo(out_folder)) {
        std::cout << "update primary input&output success\n";
    } else {
        std::cerr << "Error: update primary input&output failed!\n";
        exit(EXIT_FAILURE);
    }

    if (parse(out_folder, parallel)) {
        std::cout << "hierarchy replace success\n";
    } else {
        std::cerr << "Error: hierarchy replace failed!\n";
        exit(EXIT_FAILURE);
    }
}