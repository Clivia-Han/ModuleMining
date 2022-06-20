#pragma once

#include "util.hpp"

#include "Log.hpp"
#include "Trie.hpp"
#include "Graph.hpp"

struct System {
public:
    bool init_unpre(const std::string &connect_table_file) {
        clear();

        auto &graph = now_graph_;
        auto &nodes = storage_.nodes_;
        auto &edges = storage_.edges_;
        std::ifstream infile(connect_table_file);
        if (infile.fail()) {
            std::cerr << "open " << connect_table_file << " failed!\n";
            return false;
        }

        std::vector<int> fn_2_id;
        struct ipt { int id, port; };
        std::vector<std::pair<std::vector<ipt>, std::vector<ipt>>> sig_cat;
        auto vir_type = attribute_trie_.insert("virtual").first;
        auto super_s = full_name_trie_.insert("super_source").first;
        nodes.push_back({super_s, vir_type, {}});
        fn_2_id.push_back(super_s);
        graph.node_id_set.insert(super_s);
        auto super_t = full_name_trie_.insert("super_target").first;
        nodes.push_back({super_t, vir_type, {}});
        fn_2_id.push_back(super_t);
        graph.node_id_set.insert(super_t);

        std::string line;
        std::string cell_name, cell_type, cell_port, dir, signal;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            iss >> module_name_ >> cell_name >> cell_type >> cell_port >> dir;
            std::getline(iss, signal);
            // erase space
            signal.erase(std::remove_if(signal.begin(), signal.end(), isspace), signal.end());
            if (cell_type == "-") {
                continue;
            }
            // build node_id_set
            auto [fn_id, fn_existed] = full_name_trie_.insert(cell_name);
            int ct_id = attribute_trie_.insert(cell_type).first;
            if (!fn_existed) {
                int node_id = (int) nodes.size();
                nodes.push_back({fn_id, ct_id, {}});
                fn_2_id.push_back(node_id);
                graph.node_id_set.insert(node_id);
            }
            int node_id = fn_2_id[fn_id];
            int cp_id = attribute_trie_.insert(cell_port).first;
            auto [sig_id,sig_existed] = signal_trie_.insert(signal);
            if (!sig_existed) sig_cat.emplace_back();
            // judge direction by dir or cell_port
            if (dir == "in") {
                sig_cat[sig_id].second.push_back({node_id, cp_id});
            } else if (dir == "out") {
                sig_cat[sig_id].first.push_back({node_id, cp_id});
            }else if (cell_port[0] == 'Q' || cell_port[0] >= 'X' && cell_port[0] <= 'Z') {
                sig_cat[sig_id].first.push_back({node_id, cp_id});
            } else if (cell_port[0] >= 'X' && cell_port[0] <= 'Z') {
                sig_cat[sig_id].second.push_back({node_id, cp_id});
            }
        }
        // build edge_id_set
        for (auto sig_id : range(sig_cat.size())) {
            const auto &source_vec = sig_cat[sig_id].first;
            const auto &target_vec = sig_cat[sig_id].second;
            if (source_vec.empty()) {
                for (const auto &target : target_vec) {
                    int edge_id = (int) edges.size();
                    edges.push_back({sig_id, super_s, vir_type, target.id, target.port, {}});
                    graph.edge_id_set.insert(edge_id);
                }
                continue;
            }
            if (target_vec.empty()) {
                for (const auto &source : source_vec) {
                    int edge_id = (int) edges.size();
                    edges.push_back({sig_id, source.id, source.port, super_t, vir_type, {}});
                    graph.edge_id_set.insert(edge_id);
                }
                continue;
            }
            if (source_vec.size() > 1) {
                std::cerr << "Warning: signal with multi output: " << signal_trie_.find_idx(sig_id) << "!\n";
            }
            for (const auto &source: source_vec) {
                for (const auto &target: target_vec) {
                    int edge_id = (int) edges.size();
                    edges.push_back({sig_id, source.id, source.port, target.id, target.port, {}});
                    graph.edge_id_set.insert(edge_id);
                }
            }
        }
        return true;
    }

    bool init(const std::string &pre_table) {
        clear();

        auto &graph = now_graph_;
        auto &nodes = storage_.nodes_;
        auto &edges = storage_.edges_;
        std::ifstream infile(pre_table);
        if (infile.fail()) {
            std::cerr << "Error: open " << pre_table << " failed!\n";
            return false;
        }

        std::vector<int> fn_2_id;
        struct ipt { int id, port; };
        std::vector<std::pair<std::vector<ipt>, std::vector<ipt>>> sig_cat;

        std::string line;
        std::string cell_name, cell_type, cell_port, dir, signal;

        getline(infile, line);
        module_name_ = line;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            iss >> cell_name >> cell_type >> cell_port >> dir;
            iss.ignore();
            std::getline(iss, signal);
            // build node_id_set
            auto [fn_id, fn_existed] = full_name_trie_.insert(cell_name);
            int ct_id = attribute_trie_.insert(cell_type).first;
            if (!fn_existed) {
                int node_id = (int) nodes.size();
                nodes.push_back({fn_id, ct_id, {}});
                fn_2_id.push_back(node_id);
                graph.node_id_set.insert(node_id);
            }
            int node_id = fn_2_id[fn_id];
            int cp_id = attribute_trie_.insert(cell_port).first;
            // build sig_cat
            auto raw_signal = split(signal, ' ');
            if (dir == "i" || dir == "po") {
                for (auto &view : raw_signal) {
                    auto [sig_id,sig_existed] = signal_trie_.insert(view);
                    if (!sig_existed) sig_cat.emplace_back();
                    sig_cat[sig_id].second.push_back({node_id, cp_id});
                }
            } else if (dir == "o" || dir == "pi") {
                for (auto &view : raw_signal) {
                    auto [sig_id,sig_existed] = signal_trie_.insert(view);
                    if (!sig_existed) sig_cat.emplace_back();
                    sig_cat[sig_id].first.push_back({node_id, cp_id});
                }
            } else {
                abort();
            }
            // build np_signal_order_ for hierarchy concat
            // hint: we regard cell_type with prefix "sky" as base cell
            if (!(cell_type.size() >= 3 && cell_type.substr(0, 3) == "sky")) {
                int ptr = 0;
                for (auto &view : raw_signal) {
                    auto sig_id = signal_trie_.insert(view).first;
                    np_signal_order_[node_id][cp_id][sig_id] = ptr++;
                }
            }
        }
        // build edge_id_set
        for (auto sig_id : range(sig_cat.size())) {
            const auto &source_vec = sig_cat[sig_id].first;
            const auto &target_vec = sig_cat[sig_id].second;
            if (source_vec.size() > 1) {
                std::cerr << "Warning: signal with multi output: " << signal_trie_.find_idx(sig_id) << "!\n";
            }
            // wire
            for (const auto &source: source_vec) {
                for (const auto &target: target_vec) {
                    int edge_id = (int) edges.size();
                    edges.push_back({sig_id, source.id, source.port, target.id, target.port, {}});
                    graph.edge_id_set.insert(edge_id);
                }
            }
        }
        return true;
    }

    // replace a cell with module
    // include concat module's primary input & output
    bool replace(int replace_node_id, System &sub) {
        const auto rep_full_name = full_name_trie_.find_idx(node_ref(replace_node_id).full_name_);
        if (attribute_trie_.find_idx(node_ref(replace_node_id).type_) != sub.module_name_) {
            std::cerr << "Error: rep_cell_type and module_name is different!\n";
            return false;
        }
        const auto _sub_id = sub.attribute_trie_.find_str("-");
        std::map<int, int> port_map;
        // check concat port&sig
        for (auto &[port_id, sig_order] : np_signal_order_[replace_node_id]) {
            const auto cell_port = attribute_trie_.find_idx(port_id);
            const auto sub_fn_id = sub.full_name_trie_.find_str(cell_port);
            auto sub_node_id = [&]() -> int {
                for (auto sub_node_id : sub.now_graph_.node_id_set) {
                    if (sub.node_ref(sub_node_id).full_name_ == sub_fn_id) {
                        return sub_node_id;
                    }
                }
                return -1;
            }();
            if (sub_node_id == -1) {
                std::cerr << "Error: port name not match!\n";
                return false;
            }
            port_map[port_id] = sub_node_id;
            auto &sub_sig_order = sub.np_signal_order_[sub_node_id][_sub_id];
            if (sig_order.size() != 1 && sig_order.size() != sub_sig_order.size()) {
                std::cerr << "Error: port-signal number not match!\n";
                return false;
            }
        }
        // get aft_nodes & aft_edges
        std::vector<int> aft_nodes, aft_edges;
        auto &nodes = storage_.nodes_;
        auto &edges = storage_.edges_;
        std::map<int, int> node_map, sub_node_map, sub_attr_map;
        auto all_sub_fn_map = std::move(sub.full_name_trie_.all_mapping());
        auto all_sub_attr_map = std::move(sub.attribute_trie_.all_mapping());
        auto all_sub_sig_map = std::move(sub.signal_trie_.all_mapping());
        for (auto &item : all_sub_attr_map) {
            sub_attr_map[item.first] = attribute_trie_.insert(item.second).first;
        }
        // add inner nodes
        for (auto sub_node_id : sub.now_graph_.node_id_set) {
            const auto sub_node = sub.node_ref(sub_node_id);
            if (sub_node.type_ == _sub_id) continue; // virtual node
            int new_node_id = (int) nodes.size();
            std::string hie_fn = rep_full_name + "-" + all_sub_fn_map[sub_node.full_name_];
            int new_fn_id = full_name_trie_.insert(hie_fn).first;
            nodes.push_back({new_fn_id, sub_attr_map[sub_node.type_], {}});
            aft_nodes.push_back(new_node_id);
            sub_node_map[sub_node_id] = new_node_id;
        }
        // add inner edges, migrate np_signal_order_
        std::map<int, std::map<int, int>> sub_sig_order_map;
        for (auto sub_edge_id : sub.now_graph_.edge_id_set) {
            const auto sub_edge = sub.edge_ref(sub_edge_id);
            if (sub.node_ref(sub_edge.source_).type_ == _sub_id) {
                int sig_rank = sub.np_signal_order_[sub_edge.source_][_sub_id][sub_edge.signal_name_];
                sub_sig_order_map[sub_edge.source_][sig_rank] = sub_edge_id;
                continue;
            } else if (sub.node_ref(sub_edge.target_).type_ == _sub_id) {
                int sig_rank = sub.np_signal_order_[sub_edge.target_][_sub_id][sub_edge.signal_name_];
                sub_sig_order_map[sub_edge.target_][sig_rank] = sub_edge_id;
                continue;
            }
            int new_edge_id = edges.size();
            std::string hie_sig = rep_full_name + "-" + all_sub_sig_map[sub_edge.signal_name_];
            int new_sig_id = signal_trie_.insert(hie_sig).first;
            edges.push_back({new_sig_id, sub_node_map[sub_edge.source_], sub_attr_map[sub_edge.source_port_], 
                            sub_node_map[sub_edge.target_], sub_attr_map[sub_edge.target_port_], {}});
            for (auto item : sub_edge.attributes_) {
                edges.back().attributes_.push_back(sub_attr_map[item]);
            }
            aft_edges.push_back(new_edge_id);
            if (sub.np_signal_order_.count(sub_edge.source_)) {
                np_signal_order_[sub_node_map[sub_edge.source_]][sub_attr_map[sub_edge.source_port_]][new_sig_id] = 
                    sub.np_signal_order_[sub_edge.source_][sub_edge.source_port_][sub_edge.signal_name_];
            }
            if (sub.np_signal_order_.count(sub_edge.target_)) {
                np_signal_order_[sub_node_map[sub_edge.target_]][sub_attr_map[sub_edge.target_port_]][new_sig_id] = 
                    sub.np_signal_order_[sub_edge.target_][sub_edge.target_port_][sub_edge.signal_name_];
            }
        }
        // get bef_nodes & bef_edges
        std::vector<int> bef_nodes, bef_edges;
        bef_nodes.push_back(replace_node_id);
        for (auto edge_id : now_graph_.edge_id_set) {
            const auto edge = edge_ref(edge_id);
            if (edge.source_ == replace_node_id || edge.target_ == replace_node_id) {
                bef_edges.push_back(edge_id);
            }
        }
        // concat outer edge
        // hint: if (sig_order.size()==1&&sub_sig_order.size()!=1) than (expand)
        for (auto edge_id : bef_edges) {
            const auto edge = edge_ref(edge_id);
            if (edge.source_ == replace_node_id) {
                int sig_rank = np_signal_order_[replace_node_id][edge.source_port_][edge.signal_name_];
                int sub_node_id = port_map[edge.source_port_];
                auto add_inner_edge = [&](int rank) {
                    int sub_edge_id = sub_sig_order_map[sub_node_id][rank];
                    const auto sub_edge = sub.edge_ref(sub_edge_id);
                    int new_edge_id = (int) edges.size();
                    std::string hie_sig = rep_full_name + "-" + all_sub_sig_map[sub_edge.signal_name_];
                    int new_sig_id = signal_trie_.insert(hie_sig).first;
                    edges.push_back({new_sig_id, sub_node_map[sub_edge.source_], sub_attr_map[sub_edge.source_port_], 
                                    edge.target_, edge.target_port_, edge.attributes_});
                    aft_edges.push_back(new_edge_id);
                };
                if (np_signal_order_[replace_node_id][edge.source_port_].size() != sub_sig_order_map[sub_node_id].size()) {
                    for (auto i : range(sub_sig_order_map[sub_node_id].size())) {
                        add_inner_edge(i);
                    }
                } else  {
                    add_inner_edge(sig_rank);
                }
            } else if (edge.target_ == replace_node_id) {
                int sig_rank = np_signal_order_[replace_node_id][edge.target_port_][edge.signal_name_];
                int sub_node_id = port_map[edge.target_port_];
                auto add_inner_edge = [&](int rank) {
                    int sub_edge_id = sub_sig_order_map[sub_node_id][rank];
                    const auto sub_edge = sub.edge_ref(sub_edge_id);
                    std::string hie_sig = rep_full_name + "-" + all_sub_sig_map[sub_edge.signal_name_];
                    int new_sig_id = signal_trie_.insert(hie_sig).first;
                    int new_edge_id = (int) edges.size();
                    edges.push_back({new_sig_id, node_map[edge.source_], edge.source_port_,
                                    sub_node_map[sub_edge.target_], sub_attr_map[sub_edge.target_port_], edge.attributes_});
                    aft_edges.push_back(new_edge_id);
                };
                if (np_signal_order_[replace_node_id][edge.target_port_].size() != sub_sig_order_map[sub_node_id].size()) {
                    for (auto i : range(sub_sig_order_map[sub_node_id].size())) {
                        add_inner_edge(i);
                    }
                } else  {
                    add_inner_edge(sig_rank);
                }
            } else {
                abort();
            }
        }
        update_graph({bef_nodes, bef_edges}, {aft_nodes, aft_edges});
        logs_.data_.push_back({REPLACE, {bef_nodes, bef_edges}, {aft_nodes, aft_edges}});
        return true;
    }

    // shink sub to node
    // haven't build sub's primary input & output
    bool shink(std::set<int> sub, const std::string shink_cell_name, const std::string shink_cell_type) {
        if (full_name_trie_.find_str(shink_cell_name) != -1) {
            std::cerr << "Error: cell_name existed!\n";
            return false;
        }
        auto &nodes = storage_.nodes_;
        auto &edges = storage_.edges_;
        std::vector<int> bef_nodes, bef_edges;
        std::vector<int> aft_nodes, aft_edges;
        int shink_node_id = (int) nodes.size();
        int shink_fn_id = full_name_trie_.insert(shink_cell_name).first;
        int shink_ct_id = attribute_trie_.insert(shink_cell_type).first;
        nodes.push_back({shink_fn_id, shink_ct_id, {}});
        aft_nodes.push_back(shink_node_id);
        std::map<int, int> node_map;
        // get bef_nodes & bef_edges, concat outer edge
        for (auto node_id : sub) {
            bef_nodes.push_back(node_id);
        }
        for (auto edge_id : now_graph_.edge_id_set) {
            const auto edge = edge_ref(edge_id);
            bool source_in = sub.count(edge.source_), target_in  = sub.count(edge.target_);
            if (!source_in && !target_in) continue;
            bef_edges.push_back(edge_id);
            if (source_in && target_in) continue;
            if (source_in) {
                int new_edge_id = (int) edges.size();
                edges.push_back({edge.signal_name_, shink_node_id, edge.source_port_, 
                                edge.target_, edge.target_port_, edge.attributes_});
                aft_edges.push_back(new_edge_id);
                // TODO: port merge rule unknow
                int ptr = (int) np_signal_order_[shink_node_id][edge.source_port_].size();
                np_signal_order_[shink_node_id][edge.source_port_][edge.signal_name_] = ptr;
            } else if (target_in) {
                int new_edge_id = (int) edges.size();
                edges.push_back({edge.signal_name_, node_map[edge.source_], edge.source_port_, 
                                shink_node_id, edge.target_port_, edge.attributes_});
                aft_edges.push_back(new_edge_id);
                // TODO: port merge rule unknow
                int ptr = (int) np_signal_order_[shink_node_id][edge.target_port_].size();
                np_signal_order_[shink_node_id][edge.target_port_][edge.signal_name_] = ptr;
            }
        }
        update_graph({bef_nodes, bef_edges}, {aft_nodes, aft_edges});
        logs_.data_.push_back({SHINK, {bef_nodes, bef_edges}, {aft_nodes, aft_edges}});
        return true;
    }

    void update_graph(const std::pair<std::vector<int>, std::vector<int>> &bef, 
                    const std::pair<std::vector<int>, std::vector<int>> &aft) {
        // update now_graph_ & hid_graph_
        for (auto node_id : bef.first) {
            now_graph_.node_id_set.erase(node_id);
            hid_graph_.node_id_set.insert(node_id);
        }
        for (auto edge_id : bef.second) {
            now_graph_.edge_id_set.erase(edge_id);
            hid_graph_.edge_id_set.insert(edge_id);
        }
        for (auto node_id : aft.first) {
            now_graph_.node_id_set.insert(node_id);
        }
        for (auto edge_id : aft.second) {
            now_graph_.edge_id_set.insert(edge_id);
        }
    }

    bool roll_back(bool canceable = true, int sz = 1) {
        if (sz <= 0) {
            std::cerr << "Error: wrong size!\n";
            return false;
        }
        if (logs_.data_.size() < sz) {
            std::cerr << "Error: logs_size=" << logs_.data_.size() << " < roll_back_size!\n";
            return false;
        }
        auto &log = logs_.data_.back();
        update_graph(log.aft_, log.bef_);
        if (!canceable) {
            for (auto i : range(sz)) {
                auto &log = logs_.data_.back();
                update_graph(log.aft_, log.bef_);
                logs_.data_.pop_back();
            }
        } else {
            for (int i = logs_.size() - 1; sz-- > 0; --i) {
                auto &log = logs_.data_[i];
                update_graph(log.aft_, log.bef_);
                if (log.type_ == REPLACE) {
                    logs_.data_.push_back({SHINK, log.aft_, log.bef_});
                } else {
                    logs_.data_.push_back({REPLACE, log.aft_, log.bef_});
                }
            }
            
        }
        return true;
    }

    bool store(const std::string &folder_path) {
        bool res = true;
        res &= [&](const std::string &path) {
            std::ofstream outfile(path);
            if (outfile.fail()) {
                std::cerr << "Error: open " << path << " failed!\n";
                return false;
            }
            outfile << module_name_ << '\n';
            outfile << np_signal_order_;
            outfile.close();
            return true;
        }(folder_path + "/.module");
        res &= logs_.store(folder_path + "/log.data");
        res &= full_name_trie_.store(folder_path + "/full_name.map");
        res &= attribute_trie_.store(folder_path + "/attribute.map");
        res &= signal_trie_.store(folder_path + "/signal.map");
        res &= storage_.store(folder_path + "/storage.data");
        res &= now_graph_.store(folder_path + "/now.g");
        res &= hid_graph_.store(folder_path + "/hid.g");
        return res;
    }

    void clear() {
        module_name_.clear();
        np_signal_order_.clear();
        logs_.clear();
        storage_.clear();
        now_graph_.clear();
        hid_graph_.clear();
        full_name_trie_.clear();
        attribute_trie_.clear();
        signal_trie_.clear();
    }

    bool load(const std::string &folder_path) {
        clear();

        bool res = true;
        res &= [&](const std::string &path) {
            std::ifstream infile(path);
            if (infile.fail()) {
                std::cerr << "Error: open " << path << " failed!\n";
                return false;
            }
            infile >> module_name_;
            infile >> np_signal_order_;
            infile.close();
            return true;
        }(folder_path + "/.module");
        res &= logs_.load(folder_path + "/log.data");
        res &= full_name_trie_.load(folder_path + "/full_name.map");
        res &= attribute_trie_.load(folder_path + "/attribute.map");
        res &= signal_trie_.load(folder_path + "/signal.map");
        res &= storage_.load(folder_path + "/storage.data");
        res &= now_graph_.load(folder_path + "/now.g");
        res &= hid_graph_.load(folder_path + "/hid.g");
        return res;
    }

    int query_node(const std::string &fn_str) {
        int fn_id = full_name_trie_.find_str(fn_str);
        for (auto node_id : now_graph_.node_id_set) {
            if (node_ref(node_id).full_name_ == fn_id) {
                return node_id;
            }
        }
        return -1;
    }

    std::vector<int> query_edge(const std::string &sig_str) {
        std::vector<int> edges;
        int sig_id = signal_trie_.find_str(sig_str);
        for (auto edge_id : now_graph_.edge_id_set) {
            if (edge_ref(edge_id).signal_name_ == sig_id) {
                edges.push_back(edge_id);
            }
        }
        return edges;
    }

    std::vector<int> filter_node(const std::string &attr_str) {
        std::vector<int> nodes;
        int attr_id = attribute_trie_.find_str(attr_str);
        for (auto node_id : now_graph_.node_id_set) {
            auto datas = std::move(node_attributes(node_id));
            if (std::find(datas.begin(), datas.end(), node_id) != datas.end()) {
                nodes.push_back(node_id);
            }
        }
        return nodes;
    }

    std::vector<int> filter_edge(const std::string &attr_str) {
        std::vector<int> edges;
        int attr_id = attribute_trie_.find_str(attr_str);
        for (auto edge_id : now_graph_.edge_id_set) {
            auto datas = std::move(edge_attributes(edge_id));
            if (std::find(datas.begin(), datas.end(), edge_id) != datas.end()) {
                edges.push_back(edge_id);
            }
        }
        return edges;
    }


    bool write_networkX(const std::string &path) {
        /*
            Tips:
                1. The read interface implemented by Networkx inside 'networkx/readwrite/graphml.py'
                   does not support mixed graphs (directed and unidirected edge_id_set together), hyperedges,
                   **nested graphs**( Hierarchical graph structure ), or **ports**( Treated as parts of node_id_set ).
                       - This is the description written in the comments of the file
                2. Ways to use .graphml files in networkx
                    1.  s = """...
                        *data of .graphml file contains*
                        ...
                        """
                    2.  fh = io.BytesIO(s.encode("UTF-8"))
                    3.  G = nx.read_graphml(fh)
            TODO:
                1. Can be modified referring to the 'boost/property_map/dynamic_properties' later
                    - make the mapping from variables to values more flexible
                      i.e. aotumatically get the members of the data structs( Node, Edge ) instead of fixing
                2. Refer [Write/read vector of item in xml using boost::write_graphml](
                       https://stackoverflow.com/questions/49156829/write-read-vector-of-item-in-xml-using-boostwrite-graphml
                   )
            Reference:
                boost/graph/graphml.hpp
        */
        std::ofstream out(path, std::ios::out);
        if (out.fail()) {
            std::cerr << "Error: open " << path << " failed!\n";
            return false;
        }
        auto &now_node_id_set = now_graph_.node_id_set;
        auto &now_edge_id_set = now_graph_.edge_id_set;
        auto full_name_mapping = std::move(full_name_trie_.all_mapping());
        auto attribute_mapping = std::move(attribute_trie_.all_mapping());
        auto signal_mapping = std::move(signal_trie_.all_mapping());
        // outpur header
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\" "
               "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
               "xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns "
               "http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd\">\n";
        // output keys
        std::map<std::string, std::pair<std::string, bool> > key_name_type_isnode = {
                {"full_name", {"string", true}},
                {"node_attr", {"string", true}},
                {"signal",    {"string", false}},
                {"edge_attr", {"string", false}}
        };
        std::map<std::string, std::string> key_ids;
        int key_count = 0;
        for (auto &key: key_name_type_isnode) {
            std::string key_id = "key" + std::to_string(key_count++);
            key_ids[key.first] = key_id;
            out << "  <key id=\"" << key_id << "\" for=\""
                << (key.second.second ? "node" : "edge") << "\""
                << " attr.name=\"" << key.first << "\""
                << " attr.type=\"" << key.second.first << "\""
                << " />\n";
        }
        // output graph without
        out << "  <graph id=\"G\" edgedefault=\""
            << "directed\""
            << " parse.nodeids=\"free\""
            << " parse.edgeids=\"canonical\" parse.order=\"nodesfirst\">\n";
        // output node_id_set and edge_id_set of now graph
        for(int node_id : now_node_id_set) {
            auto nod = node_ref(node_id);
            out << "    <node id=\"n" << node_id << "\">\n";
            out << "      <data key=\"" << key_ids["full_name"] << "\">"
                << full_name_mapping[nod.full_name_]
                << "</data>\n";
            const auto &node_attrs = node_attributes(node_id);     // cell_type has been included
            if (!node_attrs.empty()) {
                std::string node_attr_temp = attribute_mapping[node_attrs[0]];
                for (auto j : range(node_attrs.size())) {
                    node_attr_temp += (" " + attribute_mapping[node_attrs[j]]);
                }
                out << "      <data key=\"" << key_ids["node_attr"] << "\">"
                    << node_attr_temp
                    << "</data>\n";
            }
            out << "    </node>\n";
        }
        for (auto edge_id : now_edge_id_set) {
            auto edg = edge_ref(edge_id);
            out << "    <edge id=\"e" << edge_id << "\" source=\"n"
                << edg.source_ << "\" target=\"n"
                << edg.target_ << "\">\n";
            const auto &edge_attrs = edge_attributes(edge_id);
            if (!edge_attrs.empty()) {
                std::string edge_attr_temp = attribute_mapping[edge_attrs[0]];
                for (auto j : range(edge_attrs.size())) {
                    edge_attr_temp += (" " + attribute_mapping[edge_attrs[j]]);
                }
                out << "      <data key=\"" << key_ids["edge_attr"] << "\">"
                    << edge_attr_temp
                    << "</data>\n";
            }
            out << "      <data key=\"" << key_ids["signal"] << "\">"
                << signal_mapping[edg.signal_name_]
                << "</data>\n";
            out << "    </edge>\n";
        }
        out << "  </graph>\n"
            << "</graphml>\n";
        return true;
    }

    std::string generate_label(std::vector<int> attributes) {
        std::string label = "";
        bool meet = false;
        for (int &attr : attributes) {
            if(!meet)
                meet = true;
            else
                label += "_";
            label += std::to_string(attr);
        }
        return label;
    }

    uint64_t size() {
        uint64_t res = 0;
        res += logs_.size();
        res += storage_.size();
        res += now_graph_.size();
        res += hid_graph_.size();
        res += full_name_trie_.size();
        res += signal_trie_.size();
        res += attribute_trie_.size();
        return res;
    }
//private:
    Node &node_ref(int node_id) {
        return storage_.nodes_[node_id];
    }

    Edge &edge_ref(int edge_id) {
        return storage_.edges_[edge_id];
    }

    std::vector<int> node_attributes(int node_id) {
        return storage_.node_attributes(node_id);
    }

    std::vector<int> edge_attributes(int edge_id) {
        return storage_.edge_attributes(edge_id);
    }

    std::string module_name_;
    std::map<int, std::map<int, std::map<int, int>>> np_signal_order_;
    Logs logs_;
    Storage storage_;
    Graph now_graph_, hid_graph_;
    Trie full_name_trie_, attribute_trie_, signal_trie_;
};