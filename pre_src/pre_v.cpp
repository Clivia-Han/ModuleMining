#include <bits/stdc++.h>
#include <omp.h>
#include "System.hpp"

// build <.topo> for hierarchy structure
bool creat_topo(const std::string& data, const std::string& out_folder) {
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
        if (cell_type == "-")
            continue;
        auto solve = [&](std::string name) {
            if (module_map.count(name))
                return;
            int sz = module_map.size();
            module_map[name] = sz;
            module_index.push_back(name);
            adj.emplace_back();
            in_degree.push_back(0);
        };
        solve(module_name);
        solve(cell_type);
        int to = module_map[module_name], from = module_map[cell_type];
        if (std::find(adj[from].begin(), adj[from].end(), to) ==
            adj[from].end()) {
            adj[from].push_back(to);
            in_degree[to]++;
        }
    }
    infile.close();
    std::queue<int> queue;
    for (int i : range(module_map.size())) {
        if (in_degree[i] == 0)
            queue.push(i);
    }
    std::vector<int> topo_order, back_in = in_degree;
    while (!queue.empty()) {
        int u = queue.front();
        topo_order.push_back(u);
        queue.pop();
        for (auto e : adj[u]) {
            back_in[e]--;
            if (back_in[e] == 0)
                queue.push(e);
        }
    }
    if (topo_order.size() != module_map.size()) {
        std::cerr << "can't find a topo oder!\n";
        return false;
    }
    std::ofstream outfile(out_folder + "/.topo");
    topo_order.erase(remove_if(topo_order.begin(), topo_order.end(),
                               [&](int x) { return in_degree[x] == 0; }),
                     topo_order.end());
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

bool split_file(const std::string& data, const std::string& out_folder) {
    std::ofstream outfile;
    std::string line;
    std::string module_name, cell_name, cell_type, cell_port, dir, signal,
        pre_module_name;
    std::ifstream infile("../data/" + data);
    if (infile.fail()) {
        std::cerr << "open data failed!\n";
        return false;
    }
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        iss >> module_name;
        if (pre_module_name.empty() || pre_module_name != module_name) {
            outfile.close();
            outfile.open(out_folder + "/" + module_name + ".table");
        }
        outfile << line << '\n';
        pre_module_name = module_name;
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
bool divide_table(const std::string& out_folder) {
    std::ifstream m_infile;
    m_infile.open(out_folder + "/.topo");
    std::vector<std::string> module_names;
    std::string _line;
    while (std::getline(m_infile, _line)) {
        module_names.push_back(_line);
    }
    m_infile.close();
#pragma omp parallel for
    for (int item = (int) module_names.size() - 1; item >= 0; --item) {
        auto module_name = module_names[item];
        std::ifstream infile;
        std::ofstream outfile;
        std::filesystem::path p1(out_folder + "/" + module_name + ".table");
        std::filesystem::path p2(out_folder + "/" + module_name + ".table.b");
        std::filesystem::rename(p1, p2);
        infile.open(p2);
        outfile.open(p1);
        outfile << module_name << '\n';
        std::string line, cell_name, cell_type, cell_port, dir, signal;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            iss >> module_name >> cell_name >> cell_type >> cell_port >> dir;
            std::getline(iss, signal);
            if (dir == "pi" || dir == "po") {
                // do nothing
            } else if (dir == "out" ||
                       (cell_port[0] >= 'X' && cell_port[0] <= 'Z') ||
                       cell_port[0] == 'Q') {
                dir = "o";
            } else {
                dir = "i";
            }
            signal.erase(remove_if(signal.begin(), signal.end(),
                                   [](char c) { return c == '{' || c == '}'; }),
                         signal.end());
            while (!signal.empty() &&
                   (signal.front() == ' ' || signal.front() == 9))
                signal.erase(signal.begin());
            while (!signal.empty() && signal.back() == ' ')
                signal.pop_back();
            signal = std::regex_replace(signal, std::regex("\\s\\["), "[");
            outfile << cell_name << ' ' << cell_type << ' ' << cell_port << ' '
                    << dir << ' ' << signal << '\n';
        }
        infile.close();
        outfile.close();
        std::filesystem::remove(p2);
    }
    return true;
}

// update primary input&output's signal to multi
bool update_pipo(const std::string verilog, const std::string& out_folder) {
    std::ifstream m_infile;
    m_infile.open(out_folder + "/.topo");
    std::vector<std::string> module_names;
    std::string _line;
    while (std::getline(m_infile, _line)) {
        module_names.push_back(_line);
    }
    m_infile.close();
    m_infile.open("../data/" + verilog);
    if (m_infile.fail()) {
        std::cerr << "open data failed!\n";
        return false;
    }
    std::string _type, m_name, s_name, s_range;
    std::map<std::string, std::map<std::string, std::pair<int, int>>> sig_range;
    while (std::getline(m_infile, _line)) {
        std::istringstream iss(_line);
        iss >> _type;
        if (_type == "module") {
            iss >> m_name;
            if (m_name.front() == '\\') {
                m_name.erase(m_name.begin());
            }
            auto raw_name = split(m_name, '(');
            m_name = raw_name.front();
        } else if (_type == "input" || _type == "output") {
            iss >> s_range;
            if (s_range[0] == '[') {
                iss >> s_name;
                s_name.pop_back();
                s_name = "\\" + s_name;
                string_view view(s_range);
                auto p2 = view.find_first_of(':', 0);
                auto p1 = view.find_first_of('[', 0);
                auto p3 = view.find_first_of(']', p2);
                int from = view.substr(p1 + 1, p2 - p1 - 1).parse();
                int to = view.substr(p2 + 1, p3 - p2 - 1).parse();
                sig_range[m_name][s_name] = {from, to};
            }
        }
    }
    // for (auto &e : sig_range) {
    //     std::cout << e.first << "233\n";
    //     for (auto &ee : e.second) {
    //         std::cout << ee.first << ' ' << ee.second.first << ' ' << ee.second.second << '\n';
    //     }
    // }
#pragma omp parallel for
    for (int item = (int) module_names.size() - 1; item >= 0; --item) {
        auto module_name = module_names[item];
        std::ifstream infile;
        std::ofstream outfile;
        std::filesystem::path p1(out_folder + "/" + module_name + ".table");
        std::filesystem::path p2(out_folder + "/" + module_name + ".table.b");
        std::filesystem::rename(p1, p2);
        infile.open(p2);
        outfile.open(p1);
        std::string line, cell_name, cell_type, cell_port, dir, signal;
        std::getline(infile, line);
        outfile << line << '\n';
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            iss >> cell_name >> cell_type >> cell_port >> dir;
            outfile << cell_name << ' ' << cell_type << ' ' << cell_port << ' ' << dir << ' ';
            iss.ignore();  // ignore one space for signal
            auto &sig_rg = sig_range[module_name];
            std::getline(iss, signal);
            auto raw_signal = split(signal, ' ');
            for (auto i : range(raw_signal.size())) {
                auto& view = raw_signal[i];
                if (sig_rg.count(view)) {
                    auto &[from, to] = sig_rg[view];
                    for (int j = from; j >= to; --j) {
                        if (j != from)
                            outfile << ' ';
                        outfile << std::string(view) << '[' << std::to_string(j)
                                << ']';
                    }
                } else  {
                    auto p2 = view.find_first_of(':', 0);
                    if (p2 == view.size()) {
                        outfile << std::string(view);
                    } else {
                        // split \xxx[X:Y] to {\xxx[X], \xxx[X-1], ... , \xxx[Y]}
                        auto p1 = view.find_first_of('[', 0),
                            p3 = view.find_first_of(']', p2);
                        int from = view.substr(p1 + 1, p2 - p1 - 1).parse();
                        int to = view.substr(p2 + 1, p3 - p2 - 1).parse();
                        auto pre = view.substr(0, p1);
                        for (int j = from; j >= to; --j) {
                            if (j != from)
                                outfile << ' ';
                            outfile << std::string(pre) << '[' << std::to_string(j)
                                    << ']';
                        }
                    }
                }
                outfile << " \n"[i + 1 == raw_signal.size()];
            }
        }
        std::filesystem::remove(p2);
        infile.close();
        outfile.close();
    }
    return true;
}

bool parse(const std::string& out_folder) {
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
#pragma omp parallel for
    for (int item = (int) module_names.size() - 1; item >= 0; --item) {
        const auto& module_name = module_names[item];
        syms[item].init(out_folder + "/" + module_name + ".table");
    }
    // replace
    for (auto& sym : syms) {
        std::cout << "  now module: " << sym.module_name_ << '\n';
        while (true) {
            bool flag = false;
            for (auto node_id : sym.now_graph_.node_id_set) {
                const auto node_type =
                    sym.attribute_trie_.find_idx(sym.node_ref(node_id).type_);
                auto which = [&](std::string match_str) {
                    for (auto i : range(module_names.size())) {
                        if (match_str == module_names[i]) {
                            return i;
                        }
                    }
                    return -1;
                }(node_type);
                if (which == -1)
                    continue;

                if (sym.replace(node_id, syms[which])) {
                    std::cout << "    replace node_id:" << node_id
                              << " with module:" << module_names[which]
                              << " success\n";
                } else {
                    std::cout << "    Error: replace node_id:" << node_id
                              << " with module:" << module_names[which]
                              << " failed!\n";
                    return false;
                }
                flag = true;
                break;
            }
            if (!flag)
                break;
        }
    }
#pragma omp parallel for
    for (int item = (int) module_names.size() - 1; item >= 0; --item) {
        const auto& module_name = module_names[item];
        std::string folder_path(out_folder + "/" + module_name);
        // create folder
        std::filesystem::path p(folder_path);
        if (std::filesystem::exists(p))
            std::filesystem::remove_all(p);
        std::filesystem::create_directories(p);
        // store data
        syms[item].store(folder_path);
    }
    // {
    //     // store final module
    //     const auto& module_name = module_names.back();
    //     std::string folder_path(out_folder + "/" + module_name);
    //     // create folder
    //     std::filesystem::path p(folder_path);
    //     if (std::filesystem::exists(p))
    //         std::filesystem::remove_all(p);
    //     std::filesystem::create_directories(p);
    //     // store data
    //     syms.back().store(folder_path);
    // }
    return true;
}

int main(int argc, char** argv) {
    std::string data = argc < 2 ? "aes_core.hierarchy.v.table" : argv[1];
    std::string verilog = argc < 3 ? "aes_core.hierarchy.v" : argv[2];
    std::cout << "parallel pre-run\n";
    std::string out_folder = "../pre_data/" + data;

    // create folder
    std::filesystem::path p(out_folder);
    if (std::filesystem::exists(p))
        std::filesystem::remove_all(p);
    std::filesystem::create_directories(p);

    if (creat_topo(data, out_folder)) {
        std::cout << "create topo success\n";
    } else {
        std::cerr << "Error: create topo failed!\n";
        exit(EXIT_FAILURE);
    }

    if (split_file(data, out_folder)) {
        std::cout << "split file success\n";
    } else {
        std::cerr << "Error: split file failed!\n";
        exit(EXIT_FAILURE);
    }

    if (divide_table(out_folder)) {
        std::cout << "divide table success\n";
    } else {
        std::cerr << "Error: divide table failed!\n";
        exit(EXIT_FAILURE);
    }

    if (update_pipo(verilog, out_folder)) {
        std::cout << "update primary input&output success\n";
    } else {
        std::cerr << "Error: update primary input&output failed!\n";
        exit(EXIT_FAILURE);
    }

    if (parse(out_folder)) {
        std::cout << "hierarchy replace success\n";
    } else {
        std::cerr << "Error: hierarchy replace failed!\n";
        exit(EXIT_FAILURE);
    }
}